using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using DPUruNet;
using System.Xml;
using System.Collections.Concurrent;
using System.Threading;
using IdentaZone.IMPlugin;

namespace IdentaZone.Plugins.DP
{
    public class DeviceDP : IFingerDevice
    {   

        #region IFingerDevice functions

        public String Name { get; set; }
        public String Id { get; set; }

        /// <summary>
        /// Returns a string that represents the current object.
        /// </summary>
        /// <returns>
        /// A string that represents the current object.
        /// </returns>
        public override String ToString()
        {
            return "Digital Persona";
        }

        public string Description
        {
            get
            {
                return this.ToString();
            }
        }


        public void Dispose()
        {
            cancelToken.Cancel(true);
        }

        public void Dispatch(COMMAND com)
        {
            queue.Add(com);
        }

        public FingerTemplate Extract(FingerImage image)
        {
            return _devControl.Extract(image);
        }

        #endregion
        public int BSPCode
        {
            get { return TemplateTypes.DPTemplate; }
        }

        #region Fields

        private enum STATE { IDLE, TEST, GETFINGER, GETIMAGE, GETSINGLE, GETSINGLEIMAGE };
        List<FingerTemplate> resultBios = new List<FingerTemplate>();

        private BlockingCollection<COMMAND> queue = new BlockingCollection<COMMAND>();
        private CancellationTokenSource cancelToken = new CancellationTokenSource();
        private const int STREAM_DELAY = 50;
        private DeviceConrolDP _devControl;


        #endregion

        #region Constructor

        public DeviceDP(String id, String name, DeviceConrolDP control)
        {
            Id = id;
            Name = name;
            _devControl = control;
            
            Task.Factory.StartNew(() =>
            {
                deviceThread();
            });
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


        private void OnSingleCaptured(FingerImageDP image)
        {
            Ambassador.AddMessage(new MBiometricsSingleCaptured(this, image));
        }

        #endregion


        #region Thread
        private void deviceThread()
        {
            List<Fmd> preenrollmentFmds = new List<Fmd>();
            Reader activeReader = null;
            CaptureResult captureResult;
            STATE state = STATE.IDLE;
            COMMAND com = COMMAND.NONE;
            int enrollCount = 0;

            try
            {
                ReaderCollection readers = ReaderCollection.GetReaders();

                foreach (var reader in readers)
                {
                    if (reader.Description.Name == Id)
                    {
                        activeReader = reader;
                        break;
                    }
                }
                if (activeReader != null)
                {
                    activeReader.Open(Constants.CapturePriority.DP_PRIORITY_EXCLUSIVE);
                }

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

                    StopOngoingStream(ref state, activeReader, com);
                    switch (com)
                    {
                        case COMMAND.LIVECAPTURE_START:
                            activeReader.StartStreaming();
                            state = STATE.TEST;
                            break;
                        case COMMAND.LIVECAPTURE_STOP:
                            state = STATE.IDLE;
                            break;
                        case COMMAND.SINGLECAPTURE_START:
                            state = STATE.GETSINGLE;
                            break;
                        case COMMAND.SINGLECAPTURE_STOP:
                            state = STATE.IDLE;
                            break;
                        case COMMAND.ENROLLMENT_START:
                            state = STATE.GETFINGER;
                            enrollCount = 0;
                            preenrollmentFmds.Clear();
                            resultBios.Clear();
                            break;
                        case COMMAND.ENROLLMENT_STOP:
                            state = STATE.IDLE;
                            break;
                    }
                    com = COMMAND.NONE;

                    switch (state)
                    {
                        case STATE.TEST:
                            SendBaseMessage("Please, touch the scanner");
                            captureResult = activeReader.GetStreamImage(Constants.Formats.Fid.ANSI, Constants.CaptureProcessing.DP_IMG_PROC_DEFAULT, activeReader.Capabilities.Resolutions[0]);
                            if (captureResult.ResultCode == Constants.ResultCode.DP_DEVICE_FAILURE || captureResult.ResultCode == Constants.ResultCode.DP_DEVICE_BUSY)
                            {
                                Console.WriteLine("DEVICE_FAILURE, perfoming RESET" + captureResult.ResultCode);
                                activeReader.Reset();
                                activeReader.StartStreaming();
                                break;
                            }
                            if (!CheckIfReaderIsOnline(captureResult, activeReader))
                            {
                                state = STATE.IDLE;
                                break;
                            }

                            if (captureResult.ResultCode == Constants.ResultCode.DP_SUCCESS)
                            {
                                OnBiometricsCaptured(new FingerImageDP(captureResult.Data));
                            }
                            else
                            {
                                System.Console.WriteLine(captureResult.ResultCode);
                            }
                            break;
                        case STATE.GETSINGLE:
                            state = STATE.GETSINGLEIMAGE;
                            SendBaseMessage("Please, touch the scanner");
                            break;
                        case STATE.GETSINGLEIMAGE:
                            captureResult = activeReader.Capture(Constants.Formats.Fid.ANSI, Constants.CaptureProcessing.DP_IMG_PROC_DEFAULT, 50, activeReader.Capabilities.Resolutions[0]);

                            if (!CheckIfReaderIsOnline(captureResult, activeReader))
                            {
                                state = STATE.IDLE;
                                break;
                            }

                            if (!CheckCaptureResult(captureResult)) break;

                            OnSingleCaptured(new FingerImageDP(captureResult.Data));

                            state = STATE.GETSINGLE;

                            break;
                        case STATE.GETFINGER:
                            SendBaseMessage("Put finger on the scanner and then lift it up, when Image is captured");
                            state = STATE.GETIMAGE;
                            break;
                        case STATE.GETIMAGE:
                            captureResult = activeReader.Capture(Constants.Formats.Fid.ANSI, Constants.CaptureProcessing.DP_IMG_PROC_DEFAULT, 50, activeReader.Capabilities.Resolutions[0]);

                            if (!CheckIfReaderIsOnline(captureResult, activeReader))
                            {
                                state = STATE.IDLE;
                                break;
                            }

                            if (!CheckCaptureResult(captureResult)) break;

                            FingerImageDP image = new FingerImageDP(captureResult.Data);
                            OnBiometricsCaptured(image);
                            enrollCount++;

                            OnSendProgressMessage(enrollCount * 20);
                            var template = _devControl.Extract(image);
                            if(template == null){
                                   OnBiometricsEnrolled(null);
                                break;                            
                            }

                            preenrollmentFmds.Add((template as TemplateDP).fmd);

                            if (enrollCount > 4)
                            {
                                state = STATE.IDLE;
                                DataResult<Fmd> resultEnrollment = DPUruNet.Enrollment.CreateEnrollmentFmd(Constants.Formats.Fmd.ANSI, preenrollmentFmds);

                                if (resultEnrollment.ResultCode == Constants.ResultCode.DP_SUCCESS)
                                {
                                    resultBios.Add(new TemplateDP(resultEnrollment.Data));
                                    SendPopUpMessage("Successfully registered");
                                    OnBiometricsEnrolled(resultBios.ToList());
                                    break;
                                }
                                SendPopUpMessage("Can't create template, please repeat");
                                OnBiometricsEnrolled(null);
                            }
                            else
                            {
                                state = STATE.GETFINGER;
                            }

                            break;
                        case STATE.IDLE:
                            break;
                    }
                }
            }
            catch
            {
            }

            finally
            {
                activeReader.Reset();
            }
        }

        private void StopOngoingStream(ref STATE state, Reader activeReader, COMMAND com)
        {
            if (state == STATE.TEST && com != COMMAND.LIVECAPTURE_START && com != COMMAND.NONE)
            {
                activeReader.StopStreaming();
            }
        }
        #endregion

        #region Auxillary Fuctions
        public bool CheckIfReaderIsOnline(CaptureResult captureResult, Reader activeReader)
        {

            if (captureResult.ResultCode == Constants.ResultCode.DP_FAILURE ||
                captureResult.ResultCode == Constants.ResultCode.DP_INVALID_DEVICE)
            {
                Constants.ResultCode test = activeReader.GetStatus();
                if (test == Constants.ResultCode.DP_INVALID_DEVICE)
                {
                    Id = Id + "FAILED";
                    Ambassador.AddMessage(new MSignal(this, MSignal.SIGNAL.DEVICE_FAILURE));
                    Console.WriteLine(captureResult.ResultCode);
                    return false;
                }
            }
            return true;
        }

        /// <summary>
        /// Interaction logic for CheckFingerDP.xaml
        /// </summary>
        public bool CheckCaptureResult(CaptureResult captureResult)
        {
            if (captureResult.ResultCode != Constants.ResultCode.DP_SUCCESS)
            {
                return false;
                //throw new Exception(captureResult.ResultCode.ToString());
            }

            switch (captureResult.Quality)
            {
                case Constants.CaptureQuality.DP_QUALITY_GOOD:
                    return true;
                case Constants.CaptureQuality.DP_QUALITY_READER_DIRTY:
                    SendPopUpMessage("Scanner is dirty");
                    break;
                case Constants.CaptureQuality.DP_QUALITY_READER_FAILED:
                    SendPopUpMessage("Reader failed");
                    break;
                case Constants.CaptureQuality.DP_QUALITY_FAKE_FINGER:
                    SendPopUpMessage("Bad finger Image");
                    break;
            }

            return false;
        }

        /// <summary>
        /// Initializes the device.
        /// </summary>
        private Reader InitDevice()
        {
            Reader activeReader = null;
            using (ReaderCollection readers = ReaderCollection.GetReaders())
            {
                foreach (var reader in readers)
                {
                    if (reader.Description.Name == Id)
                    {
                        activeReader = reader;
                        break;
                    }
                }
                if (activeReader != null)
                {
                    activeReader.Open(Constants.CapturePriority.DP_PRIORITY_COOPERATIVE);
                }
            }
            return activeReader;
        }

        #endregion

    }
}
