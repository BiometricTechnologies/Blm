using IdentaZone.IMPlugin;
using SecuGen.FDxSDKPro.Windows;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using log4net;
namespace SG
{   
    public class DeviceSG : IFingerDevice
    {
        public readonly ILog log = LogManager.GetLogger(typeof(DeviceSG));

        #region Fields

        private static int TIMEOUT_CAPTURE = 50;
        private static int TIMEOUT_AFTER_CAPTURE = 200;

        private static int ENROLL_QUALITY = 80;
        private enum STATE { TEST, IDLE, GETFINGER, GETIMAGE, GETEMPTY, FINISHENROLL, GETEMPTYIMAGE, GETSINGLE, SINGLELIFT, GETSINGLEIMAGE, OFFLINE };

        List<FingerTemplate> resultBios = new List<FingerTemplate>();

        private SGFingerPrintManager m_FPM = new SGFingerPrintManager();
        private SGFPMFingerInfo FingerInfoTemplate = new SGFPMFingerInfo(); 
        private int m_ImageWidth;
        private int m_ImageHeight;
        private int maxTemplateSize;

        private BlockingCollection<COMMAND> queue = new BlockingCollection<COMMAND>();
        private CancellationTokenSource cancelToken = new CancellationTokenSource();
        #endregion

        #region Properties

        public SGFPMDeviceName devName { get; set; }
        public int devId { get; set; }
        private int _BSPCode
        {
            get {
                switch (devName)
                {
                    case SGFPMDeviceName.DEV_FDU03:
                        return 22;
                    case SGFPMDeviceName.DEV_FDU05:
                        return 23;
                    default:
                        return 18;
                }
            }
        }
        public int BSPCode
        {
            get { return _BSPCode; }
        }

        #endregion


        #region IFingerDevice

        public String name { get; set; }
        public String id { get; set; }
        public int count = 0;

        public DeviceSG(int id, SGFPMDeviceName sGFPMDeviceName, String name)
        {
            // TODO: Complete member initialization
            this.devId = id;
            this.devName = sGFPMDeviceName;
            this.name = name;
            Task.Factory.StartNew(() =>
            {
                deviceThread();
            });
        }

        public void Dispose()
        {
            cancelToken.Cancel(true);
        }       

        public override String ToString()
        {
            String ret;
            switch(devName){
                case SGFPMDeviceName.DEV_FDU03:
                    ret = "SecuGen SDU03P";
                    break;
                case SGFPMDeviceName.DEV_FDU04:
                    ret = "SecuGen SDU04P";
                    break;
                case SGFPMDeviceName.DEV_FDU05:
                    ret = "SecuGen U20";
                    break;
                default:
                    ret = "SecuGen";
                    break;
            }
            if (count > 0)
            {
                ret += " (device " + count.ToString() + ")";
            }
            return ret;
        }
        public string Description
        {
            get
            {
                switch (devName)
                {
                    case SGFPMDeviceName.DEV_FDU03:
                        return "SecuGen® Hamster™ Plus, SecuGen® OptiMouse™ Plus, SecuGen® iD-USB SC™";
                    case SGFPMDeviceName.DEV_FDU04:
                        return "SecuGen® Hamster™ IV, SecuGen® iD-USB SC/PIV™";
                    case SGFPMDeviceName.DEV_FDU05:
                        return "SecuGen® Hamster™ Pro 20, SecuGen® Hamster™ Pro Duo/CL";
                    default:
                        return "SecuGen";
                }
            }
        }


        public void Dispatch(COMMAND com)
        {
            queue.Add(com);
        }

        public FingerTemplate Extract(FingerImage image)
        {
            var imageSG = image as FingerImageSG;
            if (imageSG == null)
            {
                return null;
            }

            int quality = 0;
            TemplateSG tmpl = new TemplateSG(_BSPCode);
            tmpl.Bytes = new byte[maxTemplateSize];
            tmpl.info = FingerInfoTemplate;
            m_FPM.GetImageQuality(imageSG.Width, imageSG.Height, imageSG.RawData, ref quality);
            tmpl.info.ImageQuality = (short)quality;
            m_FPM.SetTemplateFormat(SGFPMTemplateFormat.ANSI378);
            m_FPM.CreateTemplate(tmpl.info, imageSG.RawData, tmpl.Bytes);
            m_FPM.GetTemplateSize(tmpl.Bytes, ref tmpl.size);

            return tmpl;
        }

        #endregion

        #region Callbacks
        private void OnBiometricsCaptured(FingerImage image)
        {
            Ambassador.AddMessage(new MBiometricsLiveCaptured(this, image));
        }

        private void SendBaseMessage(String msg)
        {
            Ambassador.AddMessage(new MShowText(this, msg));
        }

        private void SendPopUpMessage(String msg)
        {
            Ambassador.AddMessage(new MShowText(this, msg, MShowText.TYPE.POPUP));
        }

        private void OnSendProgressMessage(int progress)
        {
            Ambassador.AddMessage(MUpdateProgress.Set(this,progress));
        }

        private void OnBiometricsEnrolled(List<FingerTemplate> templates)
        {
            Ambassador.AddMessage(new MBiometricsEnrolled(this, templates));
        }

        private void OnSingleCaptured(FingerImageSG image)
        {
            Ambassador.AddMessage(new MBiometricsSingleCaptured(this, image));
        }
        #endregion

        #region Thread
        private void deviceThread()
        {
            STATE state = STATE.IDLE;
            COMMAND com = COMMAND.NONE;
            List<FingerTemplate> templates = new List<FingerTemplate>();
            byte[] image = null;
            Int32 iError = 1;
            int enrollCount = 0;
            int quality = 50;

            iError = m_FPM.Init(devName);
            iError = m_FPM.OpenDevice(devId);

            GetInfo();

            try
            {
                while (true)
                {
                    cancelToken.Token.ThrowIfCancellationRequested();

                    if (state == STATE.IDLE)
                    {
                        com = queue.Take(cancelToken.Token);
                    }
                    else
                    {
                        // non blocking take
                        if (queue.Count > 0)
                        {
                            com = queue.Take();
                        }
                    }

                    switch (com)
                    {
                        case COMMAND.LIVECAPTURE_START:
                            state = STATE.TEST;
                            break;
                        case COMMAND.ENROLLMENT_START:
                            state = STATE.GETFINGER;
                            enrollCount = 0;
                            templates.Clear();
                            break;
                        case COMMAND.SINGLECAPTURE_START:
                            state = STATE.GETSINGLE;
                            break;
                        case COMMAND.SINGLECAPTURE_STOP:
                        case COMMAND.LIVECAPTURE_STOP:
                        case COMMAND.ENROLLMENT_STOP:
                            state = STATE.IDLE;
                            break;
                    }
                    com = COMMAND.NONE;

                    switch (state)
                    {
                        case STATE.TEST:
                            SendBaseMessage("Please, touch the scanner");
//                            iError = m_FPM.SetLedOn(false);

                            image = new Byte[m_ImageWidth * m_ImageHeight];
                            try
                            {
                                iError = m_FPM.GetImageEx(image, TIMEOUT_CAPTURE, 0, ENROLL_QUALITY);
                            }
                            catch(Exception ex)
                            {

                            }
                            if (iError == (Int32)SGFPMError.ERROR_NONE)
                            {
                                OnBiometricsCaptured(new FingerImageSG(_BSPCode, image, m_ImageWidth, m_ImageHeight));
                            }
                            else if (iError == (Int32)SGFPMError.ERROR_TIME_OUT)
                            {
                                OnBiometricsCaptured(null);
                                Thread.Sleep(TIMEOUT_AFTER_CAPTURE);
                            }
                            else if ((iError != (Int32)SGFPMError.ERROR_INVALID_PARAM) && (iError != (Int32)SGFPMError.ERROR_LINE_DROPPED))
                            {
                                state = STATE.OFFLINE;
                                Ambassador.AddMessage(new MSignal(this, MSignal.SIGNAL.DEVICE_FAILURE));
                            }

                            break;
                        case STATE.GETSINGLE:
                            SendBaseMessage("Please, touch the scanner");
                            image = new Byte[m_ImageWidth * m_ImageHeight];
                            state = STATE.GETSINGLEIMAGE;
                            break;
                        case STATE.GETSINGLEIMAGE:
   //                         iError = m_FPM.SetLedOn(false);
                            image = new Byte[m_ImageWidth * m_ImageHeight];
                            iError = m_FPM.GetImageEx(image, TIMEOUT_CAPTURE, 0, ENROLL_QUALITY);
                            if (iError == 0)
                            {
                                OnSingleCaptured(new FingerImageSG(_BSPCode, image, m_ImageWidth, m_ImageHeight));
                                image = new Byte[m_ImageWidth * m_ImageHeight];
                                state = STATE.SINGLELIFT;
                            }
                            else if (iError == (Int32)SGFPMError.ERROR_TIME_OUT)
                            {
                                Thread.Sleep(TIMEOUT_AFTER_CAPTURE);
                            }
                            else if ((iError != (Int32)SGFPMError.ERROR_INVALID_PARAM) && (iError != (Int32)SGFPMError.ERROR_LINE_DROPPED))
                            {
                                state = STATE.OFFLINE;
                                Ambassador.AddMessage(new MSignal(this, MSignal.SIGNAL.DEVICE_FAILURE));
                            }
                       break;
                        case STATE.SINGLELIFT:
 //                           iError = m_FPM.SetLedOn(false);
                            iError = m_FPM.GetImageEx(image, 10, 0, ENROLL_QUALITY);
                            if (iError != 0)
                            {
                                state = STATE.GETSINGLE;
                            }
                            else
                            {
                                Thread.Sleep(TIMEOUT_AFTER_CAPTURE);
                            }

                            break;
                        case STATE.GETFINGER:
                            SendBaseMessage("Please, touch the scanner");
                            image = new Byte[m_ImageWidth * m_ImageHeight];
                            state = STATE.GETIMAGE;
                            break;
                        case STATE.GETIMAGE:
//                            iError = m_FPM.SetLedOn(false);
                            iError = m_FPM.GetImageEx(image, 10, 0, ENROLL_QUALITY);
                            if (iError == 0)
                            {
                                enrollCount++;
                                FingerImageSG fingerImage = new FingerImageSG(_BSPCode, image, m_ImageWidth, m_ImageHeight);
                                TemplateSG tmpl = Extract(fingerImage) as TemplateSG;
                                templates.Add(tmpl);

                                OnBiometricsCaptured(fingerImage);
                                OnSendProgressMessage((enrollCount) * 33);

                                if (enrollCount < 3)
                                {
                                    state = STATE.GETEMPTY;
                                }
                                else
                                {
                                    state = STATE.FINISHENROLL;
                                }
                            }
                            else if (iError == (Int32)SGFPMError.ERROR_TIME_OUT)
                            {
                                Thread.Sleep(TIMEOUT_AFTER_CAPTURE);
                            }
                            else if ((iError != (Int32)SGFPMError.ERROR_INVALID_PARAM) && (iError != (Int32)SGFPMError.ERROR_LINE_DROPPED))
                            {
                                state = STATE.OFFLINE;
                                Ambassador.AddMessage(new MSignal(this, MSignal.SIGNAL.DEVICE_FAILURE));
                            }
                          break;
                        case STATE.GETEMPTY:
                            SendBaseMessage("Please, lift your finger up");
                            image = new Byte[m_ImageWidth * m_ImageHeight];
                            state = STATE.GETEMPTYIMAGE;
                            break;
                        case STATE.GETEMPTYIMAGE:
//                            iError = m_FPM.SetLedOn(false);
                            iError = m_FPM.GetImageEx(image, 10, 0, ENROLL_QUALITY);
                            if (iError != 0)
                            {
                                state = STATE.GETFINGER;
                            }
                            else
                            {
                                Thread.Sleep(TIMEOUT_AFTER_CAPTURE);
                            }
                            break;
                        case STATE.FINISHENROLL:
                            state = STATE.IDLE;
                            foreach (var template in templates)
                            {
                                bool matched = false;
                                m_FPM.MatchTemplate((template as TemplateSG).Bytes, (templates[templates.Count - 1] as TemplateSG).Bytes, SGFPMSecurityLevel.NORMAL, ref matched);
                                if (!matched)
                                {
                                    SendPopUpMessage("Can't create template, please repeat");
                                    OnBiometricsEnrolled(null);
                                    OnSendProgressMessage(0);
                                }
                                OnSendProgressMessage(100);
                            }
                            SendPopUpMessage("Successfully registered");
                            OnBiometricsEnrolled(templates.ToList()); // return new copy of list
                            break;
                        default:
                            break;
                    }
                }
            }
            catch (Exception ex)
            {
                log.Error(ex.Message);
                m_FPM.CloseDevice();
            }

        }
        #endregion

        #region Auxilliary functions
        private void GetInfo()
        {
            SGFPMDeviceInfoParam pInfo = new SGFPMDeviceInfoParam();
            Int32 iError = m_FPM.GetDeviceInfo(pInfo);
            if (iError == (Int32)SGFPMError.ERROR_NONE)
            {
                m_ImageWidth = pInfo.ImageWidth;
                m_ImageHeight = pInfo.ImageHeight;
            }
            //m_FPM.GetMaxTemplateSize(ref maxTemplateSize);
            maxTemplateSize = 0x200;
        }
        #endregion
    }
}
