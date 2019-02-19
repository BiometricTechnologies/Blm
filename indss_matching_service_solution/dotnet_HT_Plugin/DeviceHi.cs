using Hitachi;
using Hitachi.Wrapper;
using IdentaZone.IMPlugin;
using log4net;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
namespace Hitachi
{
    public class DeviceHi : IFingerDevice
    {
        private readonly ILog _log = log4net.LogManager.GetLogger(typeof(DeviceHi));

        private enum STATE { STARTTEST, TEST, IDLE, GETFINGER, GETIMAGE, GETEMPTY, FINISHENROLL, GETEMPTYIMAGE, GETSINGLE, SINGLELIFT, GETSINGLEIMAGE, OFFLINE };
        private BlockingCollection<COMMAND> queue = new BlockingCollection<COMMAND>();
        private CancellationTokenSource cancelToken = new CancellationTokenSource();
        bioapi_unit_schema unitinfo;
        uint handle;
        public bool online = false;
        private bool isThreadFinalized = true;
        private Task deviceTask = null;
        public int BSPCode
        {
            get { return TemplateTypes.HiTemplate; }
        }

        public string Description
        {
            get { return "Hitachi Finger Vein Biometric Scanner"; }
        }
        public String Uid { get; set; }

        public DeviceHi(bioapi_unit_schema info)
        {
            unitinfo = info;
            Uid = unitinfo.UnitId.ToString();
            deviceTask = Task.Factory.StartNew(() =>
            {
                deviceThread();
            });
        }

        public bool IsOnline
        {
            get
            {
                bool status = false;
                uint res = 0;
                if (online)
                {
                    Attach();
                    res = BioAPI.Cancel(handle);
                    if (res == 0)
                    {
                        int bir = 0;
                        res = BioAPI.Capture(handle, BioAPI.PURPOSE_ENROLL, ref bir, 10);
                        if (((res & (uint)BioAPI.Error.TIMEOUT_EXPIRED) == (uint)BioAPI.Error.TIMEOUT_EXPIRED)
                            || ((res & (uint)BioAPI.Error.USER_CANCELLED) == (uint)BioAPI.Error.USER_CANCELLED)
                            || (res == 0))
                        {
                            status = true;
                        }
                    }
                    Detach();
                    online = status;
                }
                return status;
            }
        }
        public void Dispatch(COMMAND com)
        {
            queue.Add(com);
            //BioAPI.Cancel(handle);
        }

        public void Dispose()
        {

            cancelToken.Cancel(true);
            BioAPI.Cancel(handle);
            deviceTask.Wait();

        }
        public override String ToString()
        {
            return "Hitachi";
        }
        public static uint GuiStateCallback(System.IntPtr GuiStateCallbackCtx, uint GuiState, System.IntPtr Response, uint Message, byte Progress, IntPtr SampleBuffer)
        {
            Marshal.WriteByte(Response, BioAPI.CONTINUE);
            if ((GuiState & BioAPI.MESSAGE_PROVIDED) == 0)
            {
                return 0;
            }

            switch (Message)
            {
                case 0:	// PROCESS
                    break;
                case 1:	// SUCCESS
                    break;
                case 2:	// FAIL
                    break;
                case 3:	// CANCEL
                    Marshal.WriteByte(Response, BioAPI.CANCEL);
                    break;
                case 4:	// TIMEOUT
                    break;
                case 5:	// INSERT
                    break;
                default:	// UNKOWN
                    break;
            }

            return 0;

        }
        public static uint GuiStreamingCallback(System.IntPtr GuiStreamingCallbackCtx, IntPtr bitmap)
        {
            return 0;
        }
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

        BioAPI.GUI_STATE_CALLBACK stateCallBack;
        BioAPI.GUI_STREAMING_CALLBACK streamingCallBack;
        static int CaptureTimeout = 2000;
        private bool _isAttached = false;

        private void deviceThread()
        {
            isThreadFinalized = false;
            ///
            int enrollCount = 0;
            stateCallBack = new BioAPI.GUI_STATE_CALLBACK(DeviceHi.GuiStateCallback);
            streamingCallBack = new BioAPI.GUI_STREAMING_CALLBACK(DeviceHi.GuiStreamingCallback);
            List<FingerTemplate> templates = new List<FingerTemplate>();
            STATE state = STATE.IDLE;
            COMMAND com = COMMAND.NONE;
            int BirHandle = 0;
            int intsize = Marshal.SizeOf(typeof(IntPtr));
            uint res;
            bioapi_data unitcontroldata;
            bioapi_bir bir = new bioapi_bir();
            int[] enrollmentBirHandle = new int[3];
            int verifyEnrollBirHandle = 0;
            byte enrollState = BioAPI.NO_PURPOSE_AVAILABLE;
            unitcontroldata.Data = Marshal.AllocHGlobal(4 * intsize);


            try
            {               
                online = true;
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
                            state = STATE.STARTTEST;
                            break;
                        case COMMAND.ENROLLMENT_START:
                            state = STATE.GETFINGER;
                            enrollCount = 0;
                            enrollState = BioAPI.PURPOSE_ENROLL;
                            templates.Clear();
                            break;
                        case COMMAND.SINGLECAPTURE_START:
                            state = STATE.GETSINGLE;
                            break;
                        case COMMAND.SINGLECAPTURE_STOP:
                        case COMMAND.LIVECAPTURE_STOP:
                        case COMMAND.ENROLLMENT_STOP:
                            BioAPI.Cancel(handle);
                            state = STATE.IDLE;
                            break;
                    }
                    com = COMMAND.NONE;
                    switch (state)
                    {
                        case STATE.IDLE:
                            Detach();
                            break;
                        case STATE.STARTTEST:
                            SendBaseMessage("Please, place finger on scanner");
                            state = STATE.TEST;
                            break;
                        case STATE.TEST:
                            Attach();
                            res = BioAPI.Capture(handle, BioAPI.PURPOSE_VERIFY, ref BirHandle, CaptureTimeout);
                            if (res == 0)
                            {
                                bioapi_input_bir captured_bir_data = new bioapi_input_bir();
                                int processed_bir = 0;
                                GCHandle tmpgch = GCHandle.Alloc(BirHandle, GCHandleType.Pinned);
                                captured_bir_data.Form = BioAPI.BIR_HANDLE_INPUT;
                                captured_bir_data.InputBIR.BIRinBSP = tmpgch.AddrOfPinnedObject();
                                BioAPI.Process(handle, ref captured_bir_data, ref processed_bir);
                                BioAPI.FreeBIRHandle(handle, BirHandle);
                                tmpgch.Free();
                                BioAPI.GetBIRFromHandle(handle, processed_bir, ref bir);
                                OnBiometricsCaptured(new FingerImageHi(bir));
                                Thread.Sleep(300);
                                BioAPI.Free(bir.BiometricData.Data);
                                BioAPI.Free(bir.SecurityBlock.Data);
                            }
                            else
                            {
                                if (((res & (uint)BioAPI.Error.TIMEOUT_EXPIRED) == (uint)BioAPI.Error.TIMEOUT_EXPIRED)
                                    || ((res & (uint)BioAPI.Error.USER_CANCELLED) == (uint)BioAPI.Error.USER_CANCELLED))
                                {
                  //                  _log.Info(res.ToString() + " test capture error");
                                    OnBiometricsCaptured(null);
                                }
                                else
                                {
                                    online = false;
                                    state = STATE.OFFLINE;
                                    Ambassador.AddMessage(new MSignal(this, MSignal.SIGNAL.DEVICE_FAILURE));
                                }
                            }
                            break;
                        case STATE.GETSINGLE:
                            //SendBaseMessage("Please, select finger to enroll or place a finger on the FV scanner");
                            state = STATE.GETSINGLEIMAGE;
                            break;
                        case STATE.GETSINGLEIMAGE:
                            Attach();
                            res = BioAPI.Capture(handle, BioAPI.PURPOSE_VERIFY, ref BirHandle, CaptureTimeout);
                            if (res == 0)
                            {
                                bioapi_input_bir captured_bir_data = new bioapi_input_bir();
                                int processed_bir = 0;
                                GCHandle tmpgch = GCHandle.Alloc(BirHandle, GCHandleType.Pinned);
                                captured_bir_data.Form = BioAPI.BIR_HANDLE_INPUT;
                                captured_bir_data.InputBIR.BIRinBSP = tmpgch.AddrOfPinnedObject();
                                BioAPI.Process(handle, ref captured_bir_data, ref processed_bir);
                                BioAPI.FreeBIRHandle(handle, BirHandle);
                                tmpgch.Free();
                                BioAPI.GetBIRFromHandle(handle, processed_bir, ref bir);
                                Detach();
                                FingerImageHi catchedImage = new FingerImageHi(bir);
                                OnSingleCaptured(catchedImage);
                                Thread.Sleep(300);
                                BioAPI.Free(bir.BiometricData.Data);
                                BioAPI.Free(bir.SecurityBlock.Data);
                                
                            }
                            else
                            {
                                if (((res & (uint)BioAPI.Error.TIMEOUT_EXPIRED) == (uint)BioAPI.Error.TIMEOUT_EXPIRED)
                                    || ((res & (uint)BioAPI.Error.USER_CANCELLED) == (uint)BioAPI.Error.USER_CANCELLED))
                                {
                //                    _log.Info(res.ToString() + " getsingleimage capture error");
                                }
                                else
                                {
                                    online = false;
                                    state = STATE.OFFLINE;
                                    Ambassador.AddMessage(new MSignal(this, MSignal.SIGNAL.DEVICE_FAILURE));
                                }

                            }
                            break;
                        case STATE.GETFINGER:
                            SendBaseMessage("Please, place a finger on the FV scanner");
                            state = STATE.GETIMAGE;
                            break;
                        case STATE.GETIMAGE:
                            Attach();
                            res = BioAPI.Capture(handle, enrollState, ref BirHandle, CaptureTimeout);
                            if (res == 0)
                            {
                                bioapi_input_bir captured_bir_data = new bioapi_input_bir();
                                GCHandle tmpgch = GCHandle.Alloc(BirHandle, GCHandleType.Pinned);
                                captured_bir_data.Form = BioAPI.BIR_HANDLE_INPUT;
                                captured_bir_data.InputBIR.BIRinBSP = tmpgch.AddrOfPinnedObject();
                                if (enrollState == BioAPI.PURPOSE_ENROLL)
                                {
                                    res = BioAPI.CreateTemplate(handle, ref captured_bir_data, ref enrollmentBirHandle[enrollCount]);
                                    enrollCount++;
                                    if (enrollCount == 2)
                                    {
                                        enrollState = BioAPI.PURPOSE_VERIFY;
                                    }
                                }
                                else if (enrollState == BioAPI.PURPOSE_VERIFY)
                                {
                                    res = BioAPI.Process(handle, ref captured_bir_data, ref verifyEnrollBirHandle);
                                    enrollCount++;
                                }
                                tmpgch.Free();

                                BioAPI.GetBIRFromHandle(handle, BirHandle, ref bir);
                                OnBiometricsCaptured(new FingerImageHi(bir));
                                BioAPI.Free(bir.BiometricData.Data);
                                BioAPI.Free(bir.SecurityBlock.Data);

                                OnSendProgressMessage(enrollCount * 33);
                                if (enrollCount < 3)
                                {
                                    state = STATE.GETEMPTY;
                                }
                                else
                                {
                                    int FMRAchieved = 0;
                                    byte matchResult = 0;
                                    GCHandle gcverify, gcenroll;
                                    bioapi_input_bir verify_input_bir = new bioapi_input_bir();
                                    verify_input_bir.Form = BioAPI.BIR_HANDLE_INPUT;
                                    gcverify = GCHandle.Alloc(verifyEnrollBirHandle, GCHandleType.Pinned);
                                    verify_input_bir.InputBIR.BIRinBSP = gcverify.AddrOfPinnedObject();

                                    bioapi_input_bir enroll_input_bir = new bioapi_input_bir();
                                    enroll_input_bir.Form = BioAPI.BIR_HANDLE_INPUT;
                                    gcenroll = GCHandle.Alloc(enrollmentBirHandle[0], GCHandleType.Pinned);
                                    enroll_input_bir.InputBIR.BIRinBSP = gcenroll.AddrOfPinnedObject();

                                    res = BioAPI.VerifyMatch(handle, 2072, ref verify_input_bir, ref enroll_input_bir, ref matchResult, ref FMRAchieved);
                                    gcenroll.Free();
                                    if ((res == 0) && (matchResult != 0))
                                    {
                                        enroll_input_bir.Form = BioAPI.BIR_HANDLE_INPUT;
                                        gcenroll = GCHandle.Alloc(enrollmentBirHandle[1], GCHandleType.Pinned);
                                        enroll_input_bir.InputBIR.BIRinBSP = gcenroll.AddrOfPinnedObject();

                                        res = BioAPI.VerifyMatch(handle, 2072, ref verify_input_bir, ref enroll_input_bir, ref matchResult, ref FMRAchieved);
                                        gcenroll.Free();

                                    }
                                    gcverify.Free();
                                    if ((res == 0) && (matchResult != 0))
                                    {
                                        res = BioAPI.GetBIRFromHandle(handle, enrollmentBirHandle[1], ref bir);
                                        TemplateHi template = new TemplateHi(bir);
                                        BioAPI.Free(bir.BiometricData.Data);
                                        BioAPI.Free(bir.SecurityBlock.Data);
                                        templates.Add(template);
                                        state = STATE.FINISHENROLL;
                                    }
                                    else
                                    {
                                        state = STATE.FINISHENROLL;
                                        BioAPI.FreeBIRHandle(handle, enrollmentBirHandle[1]);
                                    }
                                    BioAPI.FreeBIRHandle(handle, enrollmentBirHandle[0]);
                                    BioAPI.FreeBIRHandle(handle, verifyEnrollBirHandle);
                                }
                            }
                            else if (((res & (uint)BioAPI.Error.TIMEOUT_EXPIRED) == (uint)BioAPI.Error.TIMEOUT_EXPIRED)
                                || ((res & (uint)BioAPI.Error.USER_CANCELLED) == (uint)BioAPI.Error.USER_CANCELLED))
                            {
//                                _log.Info(res.ToString() + " getimage capture error");
                            }
                            else
                            {
                                online = false;
                                state = STATE.OFFLINE;
                                for (int i = 0; i < enrollCount; i++)
                                {
                                    BioAPI.FreeBIRHandle(handle, enrollmentBirHandle[i]);
                                }
                                Ambassador.AddMessage(new MSignal(this, MSignal.SIGNAL.DEVICE_FAILURE));
                            }
                            break;
                        case STATE.GETEMPTY:
                            SendPopUpMessage("Please remove finger from scanner");
                            Attach();
                            res = BioAPI.Capture(handle, BioAPI.PURPOSE_ENROLL, ref BirHandle, CaptureTimeout);
                            if (res != 0)
                            {
                                state = STATE.GETFINGER;
                            }
                            else
                            {
                                BioAPI.FreeBIRHandle(handle, BirHandle);
                            }
                            break;
                        case STATE.GETEMPTYIMAGE:
                            break;
                        case STATE.FINISHENROLL:
                            state = STATE.IDLE;
                            if (templates.Count() > 0)
                            {
                                SendPopUpMessage("Successfully registered");
                                OnBiometricsEnrolled(templates.ToList());
                            }
                            else
                            {
                                SendPopUpMessage("Registration failed");
                                OnBiometricsEnrolled(null);
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            catch (Exception e)
            {
                _log.Error(e.Message);
            }
            finally
            {
                Detach();
                isThreadFinalized = true;
            }
        }


        private bool Attach()
        {
            if (!_isAttached)
            {

                if (HitachiBio.Attach(unitinfo, ref handle, streamingCallBack, stateCallBack))
                {
                    _isAttached = true;
                    return true;
                }
                else
                {
                    return false;
                }
            }
            return false;
        }

        private void Detach()
        {
            if (_isAttached)
            {
                HitachiBio.Detach(handle);
                _isAttached = false;
            }
        }

        public FingerTemplate Extract(FingerImage image)
        {
            var hiImage = image as FingerImageHi;
            if (hiImage != null)
            {
                return new TemplateHi(hiImage.bir_);
            }
            return null;

        }

        internal int Match(FingerTemplate template, IEnumerable<FingerTemplate> candidates, out List<FingerTemplate> matches)
        {
            Attach();
            int res = HitachiBio.Match(handle, template, candidates, out matches);
            //Detach();
            return res;
        }
    }


}
