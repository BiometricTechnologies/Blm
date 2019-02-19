using BioNetACSLib;
using IdentaZone.IMPlugin;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
namespace IB
{
    public class DeviceIB : IFingerDevice
    {
        #region Fields
        private const int NEEDED_QUALITY = 59;

        private enum STATE { IDLE, TEST, GETFINGER_1, GETIMAGE_1, GETEMPTY_1, GETEMPTY_2, GETFINGER_2, GETIMAGE_2, OFFLINE, GETSINGLE, LIFTSINGLE };

        List<FingerTemplate> resultBios = new List<FingerTemplate>();
        private const int MAX_FEATURE = 6;
        private const int ENROLL_IMAGES = 3;

        private BlockingCollection<COMMAND> queue = new BlockingCollection<COMMAND>();
        private CancellationTokenSource cancelToken = new CancellationTokenSource();

        private readonly List<String> _supportedBSP = new List<String>() { TemplateTypes.IBTemplate.ToString() };
        #endregion

        #region Constructor
        public DeviceIB(string deviceName)
        {
            this.name = deviceName;
            Task.Factory.StartNew(() =>
            {
                deviceThread();
            });
        }
        #endregion



        #region IDevicePlugin

        public String name { get; set; }
        public String id { get; set; }

        public void Dispose()
        {
            cancelToken.Cancel(true);
        }

        /// <summary>
        /// Returns a string that represents the current object.
        /// </summary>
        /// <returns>
        /// A string that represents the current object.
        /// </returns>
        public override String ToString()
        {
            return "Integrated Biometrics";
        }
        public int BSPCode
        {
            get { return TemplateTypes.IBTemplate; }
        }

        public string Description
        {
            get{
                return this.ToString();
            }
        }

        public void Dispatch(COMMAND com)
        {
            queue.Add(com);
        }

        public FingerTemplate Extract(FingerImage image)
        {
            if (image is FingerImageIB)
            {
                byte[] pFp = new byte[BioNetACSDLL._GetFeatSize()];
                int extResult = BioNetACSDLL._ExtractFt((image as FingerImageIB).rawData, pFp);
                return new TemplateIB(pFp);
            }
            return null;
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
            Ambassador.AddMessage(MUpdateProgress.Set(this, progress));
        }

        private void OnBiometricsEnrolled(List<FingerTemplate> templates)
        {
            Ambassador.AddMessage(new MBiometricsEnrolled(this, templates));
        }

        private void OnSingleCaptured(FingerImage image)
        {
            Ambassador.AddMessage(new MBiometricsSingleCaptured(this, image));
        }
        #endregion

        #region Thread
        private void deviceThread()
        {
            STATE state = STATE.IDLE;
            COMMAND com = COMMAND.NONE;
            byte[] image = null;
            byte[][] resultFeat = null;
            byte[][] imageStack = null;
            byte[][] FingerFeat = null;
            int firstEnrollCount = 0, secondEnrollCount = 0;
            int status = 0;
            
            BioNetACSDLL._AlgoInit();

            Thread.Sleep(1000);
            var error = BioNetACSDLL._OpenNetAccessDeviceByUSN(name);
            if (error != 1)
            {
                throw new Exception("Can't open device");
            }


            int uniqueUsbKey = BioNetACSDLL._GetUSBKey();
            int imageSize = BioNetACSDLL._GetImgSize();
            int imageWidth = BioNetACSDLL._GetImgWidth();
            int imageHeight = BioNetACSDLL._GetImgSize() / BioNetACSDLL._GetImgWidth();

            FingerFeat = new byte[MAX_FEATURE][];
            for (int i = 0; i < MAX_FEATURE; i++)
            {
                FingerFeat[i] = new byte[BioNetACSDLL._GetFeatSize()];
            }

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
                        case COMMAND.SINGLECAPTURE_START:
                            state = STATE.GETSINGLE;
                            break;
                        case COMMAND.ENROLLMENT_START:
                            state = STATE.GETFINGER_1;
                            resultFeat = new byte[2][];
                            for (int i = 0; i < 2; i++)
                            {
                                resultFeat[i] = new byte[BioNetACSDLL._GetFeatSize()];
                            }

                            imageStack = new byte[ENROLL_IMAGES * 2][];
                            for (int i = 0; i < ENROLL_IMAGES * 2; i++)
                            {
                                imageStack[i] = new byte[imageSize];
                            }

                            firstEnrollCount = 0;
                            secondEnrollCount = 0;
                            resultBios.Clear();
                            break;
                        case COMMAND.LIVECAPTURE_STOP:
                        case COMMAND.SINGLECAPTURE_STOP:
                        case COMMAND.ENROLLMENT_STOP:
                            state = STATE.IDLE;
                            break;
                    }
                    com = COMMAND.NONE;

                    if (uniqueUsbKey != BioNetACSDLL._GetUSBKey())
                    {
                        if (state != STATE.OFFLINE)
                        {
                            state = STATE.OFFLINE;
                            Ambassador.AddMessage(new MSignal(this, MSignal.SIGNAL.DEVICE_FAILURE));
                        }
                    }

                    switch (state)
                    {
                        case STATE.TEST:
                            image = new byte[imageSize];
                            status = BioNetACSDLL._GetNetAccessImage(image);
                            switch (status)
                            {
                                // not connected
                                case -1:
                                    state = STATE.OFFLINE;
                                    SendBaseMessage("Scanner is not connected");
                                    break;
                                case 0:
                                    SendBaseMessage("Please, touch the scanner");
                                    break;
                                default:
                                    break;
                            }
                            OnBiometricsCaptured(new FingerImageIB(image, imageWidth, imageHeight));
                            break;
                        case STATE.GETSINGLE:
                            SendBaseMessage("Please, touch the scanner");
                            image = new byte[imageSize];
                            status = BioNetACSDLL._GetNetAccessImage(image);

                            switch (status)
                            {
                                // not connected
                                case -1:
                                    SendPopUpMessage("Scanner is not connected");
                                    break;
                                case 0:
                                    break;
                                case 1:
                                    int qual = BioNetACSDLL._GetIBQualityScore(image);
                                    if (qual > NEEDED_QUALITY)
                                    {
                                        goto default;
                                    }
                                    SendPopUpMessage("Image quality " + qual + " not enough, keep finger on scanner");
                                    OnBiometricsCaptured(new FingerImageIB(image, imageWidth, imageHeight));
                                    break;
                                default:
                                    OnSingleCaptured(new FingerImageIB(image, imageWidth, imageHeight));
                                    state = STATE.LIFTSINGLE;
                                    break;
                            }
                            break;
                        case STATE.LIFTSINGLE:
                            image = new byte[imageSize];
                            status = BioNetACSDLL._GetNetAccessImage(image);
                            BioNetACSDLL._ControlTOUCH(false);
                            switch (status)
                            {
                                // not connected
                                case -1:
                                    SendPopUpMessage("Scanner is not connected");
                                    break;
                                case 0:
                                    state = STATE.GETSINGLE;
                                    break;
                                default:
                                    //SendPopUpMessage("Please, remove finger from the scanner");
                                    break;
                            }
                            break;
                        case STATE.GETFINGER_1:
                            image = new byte[imageSize];
                            state = STATE.GETIMAGE_1;
                            BioNetACSDLL._ChangeGain(0);
                            break;
                        case STATE.GETIMAGE_1:
                            status = BioNetACSDLL._GetNetAccessImageByManual(image);
                            switch (status)
                            {
                                // not connected
                                case -1:
                                    SendPopUpMessage("Scanner is not connected");
                                    break;
                                case 0:
                                    SendBaseMessage("Please, touch the scanner");
                                    break;
                                case 1:
                                    int qual = BioNetACSDLL._GetIBQualityScore(image);
                                    if (qual > NEEDED_QUALITY)
                                    {
                                        goto default;
                                    }
                                    SendPopUpMessage("Image quality " + qual + " not enough, keep finger on scanner");
                                    OnBiometricsCaptured(new FingerImageIB(image, imageWidth, imageHeight));
                                    break;
                                default:
                                    Array.Copy(image, imageStack[firstEnrollCount], imageSize);

                                    OnBiometricsCaptured(new FingerImageIB(image, imageWidth, imageHeight));

                                    firstEnrollCount++;
                                    OnSendProgressMessage((firstEnrollCount) * 22);

                                    state = STATE.GETEMPTY_1;

                                    break;
                            }
                            break;
                        case STATE.GETEMPTY_1:
                            image = new byte[imageSize];
                            status = BioNetACSDLL._GetNetAccessImage(image);
                            switch (status)
                            {
                                // not connected
                                case -1:
                                    SendPopUpMessage("Scanner is not connected");
                                    break;
                                case 0:
                                    if (firstEnrollCount < 3)
                                    {
                                        state = STATE.GETFINGER_1;
                                    }
                                    else
                                    {
                                        int singeEnrolResult = BioNetACSDLL._Enroll_SingleTemplate(
                                            (byte[])imageStack[0], (byte[])imageStack[1], (byte[])imageStack[2],
                                            (byte[])FingerFeat[0], (byte[])FingerFeat[1], (byte[])FingerFeat[2],
                                            resultFeat[0]);
                                        if (singeEnrolResult < 1)
                                        {
                                            state = STATE.GETEMPTY_2;
                                        }
                                        else
                                        {
                                            state = STATE.IDLE;
                                            resultBios.Add(new TemplateIB(resultFeat[0]));
                                            SendPopUpMessage("Successfully registered");
                                            OnBiometricsEnrolled(resultBios.ToList());
                                        }
                                    }
                                    break;
                                default:
                                    SendPopUpMessage("Please, remove finger from the scanner");
                                    break;
                            }
                            break;
                        case STATE.GETFINGER_2:
                            image = new byte[imageSize];
                            state = STATE.GETIMAGE_2;
                            break;
                        case STATE.GETIMAGE_2:
                            status = BioNetACSDLL._GetNetAccessImageByManual(image);
                            switch (status)
                            {
                                // not connected
                                case -1:
                                    SendPopUpMessage("Scanner is not connected");
                                    break;
                                case 0:
                                    SendBaseMessage("Please, touch the scanner");
                                    break;
                                case 1:
                                    int qual = BioNetACSDLL._GetIBQualityScore(image);
                                    if (qual > NEEDED_QUALITY)
                                    {
                                        goto default;
                                    }
                                    SendPopUpMessage("Image quality " + qual + " not enough, keep finger on scanner");
                                    OnBiometricsCaptured(new FingerImageIB(image, imageWidth, imageHeight));
                                    break;
                                default:
                                    Array.Copy(image, imageStack[secondEnrollCount], imageSize);

                                    OnBiometricsCaptured(new FingerImageIB(image, imageWidth, imageHeight));
                                    secondEnrollCount++;
                                    OnSendProgressMessage(66 + (secondEnrollCount) * 11);
                                    state = STATE.GETEMPTY_2;
                                    break;
                            }
                            break;
                        case STATE.GETEMPTY_2:
                            image = new byte[imageSize];
                            status = BioNetACSDLL._GetNetAccessImage(image);
                            switch (status)
                            {
                                // not connected
                                case -1:
                                    SendPopUpMessage("Scanner is not connected");
                                    break;
                                case 0:
                                    if (secondEnrollCount < 3)
                                    {
                                        state = STATE.GETFINGER_2;
                                    }
                                    else
                                    {
                                        int multiEnrollResult = BioNetACSDLL._Enroll_MultiTemplate(
                                            (byte[])FingerFeat[0], (byte[])FingerFeat[1], (byte[])FingerFeat[2],
                                            (byte[])imageStack[3], (byte[])imageStack[4], (byte[])imageStack[5],
                                            resultFeat[0], resultFeat[1]);
                                        if (multiEnrollResult > 0)
                                        {
                                            state = STATE.IDLE;
                                            resultBios.Add(new TemplateIB(resultFeat[0]));
                                            resultBios.Add(new TemplateIB(resultFeat[1]));
                                            SendPopUpMessage("Successfully registered");
                                            OnBiometricsEnrolled(resultBios.ToList());
                                        }
                                        else
                                        {
                                            state = STATE.IDLE;
                                            SendPopUpMessage("Can't create template, please repeat");
                                            OnBiometricsEnrolled(null);
                                        }
                                    }
                                    break;
                                default:
                                    SendPopUpMessage("Please, remove finger from the scanner");
                                    break;
                            }
                            break;
                        case STATE.OFFLINE:
                            break;
                    }
                }
            }
            catch
            {
                BioNetACSDLL._CloseNetAccessDevice();
            }
        }
        #endregion
    }
}
