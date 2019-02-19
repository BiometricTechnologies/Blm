using IdentaZone.IMPlugin;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace IdentaZone.Plugins.FAKE
{
    public class DeviceFake : IFingerDevice
    {
        /// <summary>
        /// State machine for device.
        /// TODO: if your device requires some logic for single capture
        /// e.g. you should check image quality and wait while user lifts finger up
        /// You should add additional state. E.g. SINGLECAPTURE_START, SINGLECAPTURE_END
        /// And change device thread.
        /// Same for Enrollment. E.g. you should capture 3 fingerprints and wait between capture
        /// Then you can extend STATE -> ENROLL_START, ENROLL_END and add some var into thread
        /// to track state of ENROLLMENT
        /// </summary>
        private enum STATE { IDLE, LIVECAPTURE, SINGLECAPTURE, ENROLL };

        /// <summary>
        /// Queue is used to pass commands into DeviceThread
        /// </summary>
        private BlockingCollection<COMMAND> queue = new BlockingCollection<COMMAND>();
        /// <summary>
        /// Token is used to correct Thread breaking during Dispose() call
        /// </summary>
        private CancellationTokenSource cancelToken = new CancellationTokenSource();

        /// <summary>
        /// Device constructor
        /// You can pass here some device ID, needed to communicate with device or some object which represents device
        /// If some information from
        /// </summary>
        public DeviceFake()
        {
            //TODO: save const parameters to local field and start thread. Device initialization should be done in thread
            Task.Factory.StartNew(() => deviceThread());
        }

        #region DeviceThread
        private void deviceThread()
        {

            STATE state = STATE.IDLE;
            COMMAND com = COMMAND.NONE;
            try
            {
                //TODO: Add your device initialization here
                InitDevice();

                while (true)
                {
                    // This block is common for all plugins
                    // Check the thread cancelation, then try to get command from queue
                    // If nothing should be done (IDLE state) - block on queueTaking
                    // If not - check for new command and go to main logic
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
                            state = STATE.LIVECAPTURE;
                            break;
                        case COMMAND.ENROLLMENT_START:
                            state = STATE.ENROLL;
                            break;
                        case COMMAND.SINGLECAPTURE_START:
                            state = STATE.SINGLECAPTURE;
                            break;
                        case COMMAND.LIVECAPTURE_STOP:
                        case COMMAND.ENROLLMENT_STOP:
                        case COMMAND.SINGLECAPTURE_STOP:
                            state = STATE.IDLE;
                            break;
                        default:
                            break;
                    }

                    com = COMMAND.NONE;

                    switch (state)
                    {
                        case STATE.LIVECAPTURE:
                            Thread.Sleep(100);
                            Ambassador.AddMessage(new MBiometricsLiveCaptured(this, new FingerImageFake()));
                            break;
                        case STATE.SINGLECAPTURE:
                            Thread.Sleep(500);
                            Ambassador.AddMessage(new MBiometricsSingleCaptured(this, new FingerImageFake()));
                            state = STATE.IDLE;
                            break;
                        case STATE.ENROLL:
                            Ambassador.AddMessage(new MBiometricsSingleCaptured(this, new FingerImageFake()));
                            Ambassador.AddMessage(new MBiometricsEnrolled(this, new List<FingerTemplate>() { new TemplateFake(new byte[0]) }));
                            state = STATE.IDLE;
                            break;
                        case STATE.IDLE:
                        default:
                            break;
                    }
                }
            }
            catch
            {
            }

            finally
            {
                CleanUp();
            }
        }

        #endregion

        #region DeviceAuxFunctions
        
        private void InitDevice()
        {
            //TODO: init device here
        }

        private void CleanUp()
        {
            //TODO: clean-up device resources
        }

        #endregion

        #region IFingerDevice

        /// <summary>
        /// Each device should be capabale of extracting Template from Image
        /// </summary>
        /// <param name="image"></param>
        /// <returns></returns>
        public FingerTemplate Extract(FingerImage image)
        {
            // Plugin should check if it CAN extract this kind of image
            // If not - return null
            if (image is FingerImageFake)
            {
                // TODO: change implementation to device extraction function.
                return new TemplateFake(new byte[0]);
            }
            return null;
        }

        public int BSPCode {
            get { return 0; }
        }

        /// <summary>
        /// Here we just pass command to internal thread, using queue
        /// </summary>
        /// <param name="com"></param>
        public void Dispatch(COMMAND com)
        {
            queue.Add(com);
        }

        /// <summary>
        /// If dispose command received - we just close Thread, clean-up would be done through thread
        /// </summary>
        public void Dispose()
        {
            cancelToken.Cancel(true);
        }

        public string Description
        {
            get
            {
                return this.ToString();
            }
        }

        /// <summary>
        /// Here we return human-readable ID of device
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            //TODO: change it to your device name.
            return "Fake Device";
        }
        #endregion
    }
}
