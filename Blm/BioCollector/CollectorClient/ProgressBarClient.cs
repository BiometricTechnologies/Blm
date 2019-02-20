using IdentaZone.CollectorServices;
using log4net;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.ServiceModel;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.Collector
{
    [ComVisible(true)]
    [Guid("06764527-A418-4C94-880C-2FCE29F3362D")]
    [ClassInterface(ClassInterfaceType.None)]
    [ProgId("IdentaZoneCollectorProgressBar")]
    public class ProgressBarClient : ICollectorProgressBar, IClientCallback
    {
        // Logger instance
        protected readonly ILog log = LogManager.GetLogger(typeof(ProgressBarClient));

        // Pipe to server
        IProgressBarClientService progressBarService = null;
        ChannelFactory<IProgressBarClientService> pipeFactory;

        bool IsConnected = false;

        public ProgressBarClient()
        {
            Auxiliary.Init();
            // Connect to IZService
            log.Info("Connecting to IZService!!!");
            try
            {

                log.Info("Trying to open pipe");
                var binding = new NetNamedPipeBinding();

                pipeFactory =
                    new DuplexChannelFactory<IProgressBarClientService>(new InstanceContext(this),
                        binding,
                        new EndpointAddress("net.pipe://localhost/IdentaZone/Collector/ProgressBarClient"));
                log.Info("Trying to create channel");
                progressBarService = pipeFactory.CreateChannel();
                log.Info("Trying to login");
                if (!progressBarService.Login())
                {
                    log.Error("Service is busy");
                    return;
                }
                else
                {
                    log.Info("Successfully logged in");
                    IsConnected = true;
                }
            }
            catch (Exception ex)
            {
                log.Error("Cannot connect to pipe", ex);
                return;
            }
        }


        internal bool CheckConnection()
        {
            if (!IsConnected)
            {
                log.Fatal("Not connected to IZService");
                return false;
            }
            else
            {
                return true;
            }
        }

        public ReturnTypes CloseDialog()
        {
            log.Info("call");
            if (!CheckConnection())
            {
                return ReturnTypes.rtErrorUnknown;
            }
            else
            {
                var comResult = progressBarService.HideDialog();
                log.InfoFormat("result {0}", comResult);
                return T(comResult);
            }
        }

        private ReturnTypes T(ProgressBarState comResult)
        {
            switch (comResult)
            {
                case ProgressBarState.OK:
                    return ReturnTypes.rtOK;
                case ProgressBarState.UNITIALIZED:
                    return ReturnTypes.rtErrorInitializeFirst;
                case ProgressBarState.WAIT_MORE:
                    return ReturnTypes.rtWaitMore;
                default:
                    log.Error("Unknown ProgressBarState " + comResult);
                    return ReturnTypes.rtErrorUnknown;
            }
        }

        public ReturnTypes SetProgress(int pBarNumber, int value)
        {
            if (!CheckConnection())
            {
                return ReturnTypes.rtErrorUnknown;
            }
            else
            {
                var comResult = progressBarService.SetProgress(pBarNumber, value);
                return T(comResult);
            }
        }

        public ReturnTypes ShowDialog()
        {
            log.Info("call");
            if (!CheckConnection())
            {
                return ReturnTypes.rtErrorUnknown;
            }
            else
            {
                var comResult = progressBarService.ShowDialog();
                log.InfoFormat("result {0}", comResult);
                return T(comResult);
            }
        }

        public void Dropped()
        {
            log.Info("Dropped from server");
            IsConnected = false;
            progressBarService.Logout();
        }

        public void Heartbeat()
        {
        }


        public ReturnTypes Dispose()
        {
            log.Info("call");

            if (!IsConnected)
            {
                log.Error("Not connected to IZService");
                return ReturnTypes.rtErrorUnknown;
            }

            var res = progressBarService.Dispose();
            log.InfoFormat("result {0}", res);

            log.Info("Logging out from server");
            progressBarService.Logout();

            IsConnected = false;
            return T(res);
        }

        public ReturnTypes Init()
        {
            log.Info("call");
            if (!IsConnected)
            {
                log.Fatal("Not connected to IZService");
                return ReturnTypes.rtErrorUnknown;
            }

            var res = progressBarService.Init();
            log.InfoFormat("result {0}", res);
            return T(res);
        }

        public ReturnTypes IsReady()
        {
            log.Info("IsReady called");
            var res = progressBarService.IsReady();
            log.InfoFormat("result {0}", res);
            return T(res);
        }


        public ReturnTypes SetBarsCount(int count)
        {
            log.InfoFormat("SetBarsCount to {0} called", count);
            var res = progressBarService.SetBarsCount(count);
            log.InfoFormat("result {0}", res);
            return T(res);
        }

        public ReturnTypes SetCaption(String caption)
        {
            log.InfoFormat("Set Caption to {1} called", caption);
            var res = progressBarService.SetCaption(caption);
            log.InfoFormat("result {0}", res);
            return T(res);
        }

        public ReturnTypes SetText(int pBarNumber, string text)
        {
            var res = progressBarService.SetText(pBarNumber, text);
            return T(res);
        }


        public ReturnTypes CloseDialogAfter(int timeout, String message)
        {
            log.InfoFormat("Close dialog after {0} ms with {1} message", timeout, message);
            var res = progressBarService.HideDialogAfter(timeout, message);
            return T(res);
        }
    }
}
