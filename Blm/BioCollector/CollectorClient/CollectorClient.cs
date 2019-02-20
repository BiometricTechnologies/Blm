using CollectorInterfaceLib;
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
    [Guid("DB27C81C-20FA-42CD-85B9-B5722D1E818D")]
    [ClassInterface(ClassInterfaceType.None)]
    [ProgId("IdentaZoneCollectorDialog")]
    public class CollectorClient : ICollectorDialog, IClientCallback
    {
        // Logger instance
        protected readonly ILog log = LogManager.GetLogger(typeof(CollectorClient));

        // Pipe to server
        IDialogClientService collectorService = null;
        ChannelFactory<IDialogClientService> pipeFactory;

        bool IsConnected = false;

        public CollectorClient()
        {
            Auxiliary.Init();
            // Connect to IZService
            log.Info("Connecting to IZService");
            try
            {
                var binding = new NetNamedPipeBinding();
                binding.MaxBufferSize = BioData.MaxSize;
                binding.MaxReceivedMessageSize = binding.MaxBufferSize;

                pipeFactory =
                   new DuplexChannelFactory<IDialogClientService>(new InstanceContext(this),
                       binding,
                       new EndpointAddress("net.pipe://localhost/IdentaZone/Collector/DialogClient"));

                collectorService = pipeFactory.CreateChannel();

                if (!collectorService.Login())
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
                log.Info(ex);
                return;
            }
        }

        public static ReturnTypes T(CollectorState state)
        {
            switch (state)
            {
                case CollectorState.OK:
                    return ReturnTypes.rtOK;
                case CollectorState.UNITIALIZED:
                    return ReturnTypes.rtErrorInitializeFirst;
                case CollectorState.WAIT_MORE:
                    return ReturnTypes.rtWaitMore;
                case CollectorState.NO_DATA:
                    return ReturnTypes.rtNoData;
                case CollectorState.BUSY:
                    return ReturnTypes.rtErrorBusy;
                case CollectorState.DECRYPT_ALL:
                    return ReturnTypes.rtDecryptAll;
                case CollectorState.DECRYPT_LIST:
                    return ReturnTypes.rtDecryptList;
                default:
                    return ReturnTypes.rtErrorUnknown;
            }
        }

        public ReturnTypes AddCryptoProvider(String providerName)
        {
            log.Info("call");

            if (!IsConnected)
            {
                log.Fatal("Not connected to IZService");
                return ReturnTypes.rtErrorUnknown;
            }

            var res = collectorService.AddCryptoProvider(providerName);
            log.InfoFormat("result {0}", res);
            return T(res);
        }

        public ReturnTypes CloseDialog()
        {
            log.Info("call");

            if (!IsConnected)
            {
                log.Fatal("Not connected to IZService");
                return ReturnTypes.rtErrorUnknown;
            }

            var res = collectorService.HideDialog();
            log.InfoFormat("result {0}", res);
            return T(res);
        }

        public ReturnTypes Dispose()
        {
            log.Info("call");

            if (!IsConnected)
            {
                log.Error("Not connected to IZService");
                return ReturnTypes.rtErrorUnknown;
            }

            var res = collectorService.Dispose();
            log.InfoFormat("result {0}", res);

            log.Info("Logging out from server");
            collectorService.Logout();

            IsConnected = false;
            Marshal.FreeCoTaskMem(_allocatedMem);

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

            var res = collectorService.Init();
            log.InfoFormat("result {0}", res);
            return T(res);
        }

        public ReturnTypes PullBioData(out string type, out string data, out int providerNumber, out int deleteAfter)
        {
            log.Info("call");
            type = "";
            data = "";
            providerNumber = -1;
            deleteAfter = 0;

            if (!IsConnected)
            {
                log.Fatal("Not connected to IZService");
                return ReturnTypes.rtErrorUnknown;
            }

            var res = collectorService.PullBioData(out type, out data, out providerNumber, out deleteAfter);
            log.InfoFormat("result {0}", res);
            return T(res);
        }

        public CollectorDialogState T(StateTypes newState)
        {
            switch (newState)
            {
                case StateTypes.stIdle:
                    return CollectorDialogState.IDLE;
                case StateTypes.stGoodBiometric:
                    return CollectorDialogState.GOOD_BIOMETRICS;
                case StateTypes.stBadBiometric:
                    return CollectorDialogState.BAD_BIOMETRICS;
                case StateTypes.stBadBiometricUser:
                    return CollectorDialogState.BAD_BIOMETRICS_USER;
                case StateTypes.stBadUserLoggedIn:
                    return CollectorDialogState.BAD_USER_LOGGED_IN;
                case StateTypes.stShowFileList:
                    return CollectorDialogState.SHOW_FILE_LIST;
                case StateTypes.stNotIdentifiedAsCurrentUser:
                    return CollectorDialogState.NOT_IDENTIFIED_AS_CURRENT_USER;
                default:
                    log.Info("Unknown StateTypes state " + newState);
                    return CollectorDialogState.IDLE;
            }
        }

        public ReturnTypes UpdateState(StateTypes newState)
        {
            log.Info("call");
            if (!IsConnected)
            {
                log.Fatal("Not connected to IZService");
                return ReturnTypes.rtErrorUnknown;
            }

            var res = collectorService.UpdateState(T(newState));
            log.InfoFormat("result {0}", res);
            return T(res);
        }

        public ReturnTypes CloseDialogAfter(int timeout)
        {
            log.Info("Close after " + timeout);
            var res = collectorService.HideDialogAfter(timeout);
            log.InfoFormat("result {0}", res);
            return T(res);
        }

        public void Dropped()
        {
            log.Info("Dropped from server");
            collectorService.Logout();
            IsConnected = false;
        }


        public ReturnTypes IsReady()
        {
            log.Info("IsReady called");
            CollectorState res = collectorService.IsReady();
            log.InfoFormat("result {0}", res);
            return T(res);
        }

        public void Heartbeat()
        {
        }

        public ReturnTypes AddFile(FileInfoStruct fileInfo)
        {
            log.Info("Add file called");
            BioFileInfo bioFileInfo = new BioFileInfo()
            {
                Filename = fileInfo.Filename,
                FileNumber = fileInfo.FileNumber,
                FileSize = fileInfo.FileSize,
                Timestamp = fileInfo.ModificationDate,
                PathType = fileInfo.type == FileEntityTypes.FOLDER ? BioFileInfo.EntityAtPathType.Folder : BioFileInfo.EntityAtPathType.Regular
            };
            CollectorState res = collectorService.AddFile(bioFileInfo);
            return T(res);
        }



        public ReturnTypes PullFileAction()
        {
            log.Info("Pull file called");
            var res = collectorService.PullFileData();
            return T(res);
        }

        IntPtr _allocatedMem = IntPtr.Zero;
        List<int> selectedItems = null;

        public ReturnTypes GetSelectedFiles(ref int elemCount, IntPtr fileNumArray)
        {
            int arraySize = elemCount;
            log.Info("Get Selected called");
            ReturnTypes res = 0;

            if (selectedItems == null)
            {
                collectorService.GetSelectedFiles(out selectedItems);
            }

            if (selectedItems.Count == 0)
            {
                selectedItems = null;
                return T(CollectorState.NO_DATA);
            }

            if (arraySize < selectedItems.Count)
            {
                elemCount = arraySize;
            }
            else
            {
                elemCount = selectedItems.Count;
            }

            Marshal.Copy(selectedItems.ToArray(), 0, fileNumArray, elemCount);
            selectedItems.RemoveRange(0, elemCount);

            if (selectedItems.Count == 0)
            {
                selectedItems = null;
                return ReturnTypes.rtOK;
            }
            else
            {
                return ReturnTypes.rtHasMoreData;
            }
        }
        public ReturnTypes Log(LogRecordStruct logRecord)
        {
            log.InfoFormat("Log record called {0} {1} {2}", logRecord.Filename, logRecord.Provider, logRecord.Username);
            LogRecord record = new LogRecord()
            {
                filename = logRecord.Filename,
                provider = logRecord.Provider,
                username = logRecord.Username
            };

            if (logRecord.Operation == LogOperationTypes.lotDecrypt)
            {
                record.operation = LogOperation.DECRYPT;
            }
            else
            {
                record.operation = LogOperation.ENCRYPT;
            }

            CollectorState res = collectorService.Log(record);
            log.Info("result" + res);
            return T(res);
        }


        public ReturnTypes ShowDialogEx(CollectorDialogData data)
        {
            log.Info("Called ShowDialogEx");

            var initData = new CollectorDialogInitData()
            {
                CryptoProviders = new List<String>(),
                Filename = data.Filename,
                Username = data.Username,
                Mode = TranslateMode(data.mode)
            };

            CollectorState res = collectorService.ShowDialogEx(initData);
            log.Info("result" + res);
            return T(res);
        }

        private CollectorDialogDisplayType TranslateMode(CollectorDialogMode collectorDialogMode)
        {
            switch (collectorDialogMode)
            {
                case CollectorDialogMode.cdmEncrypt:
                    return CollectorDialogDisplayType.ENCRYPT;
                case CollectorDialogMode.cdmDecryptOverwrite:
                    return CollectorDialogDisplayType.DECRYPT_OVERWRITE_YES;
                case CollectorDialogMode.cdmDecryptNoOverwrite:
                    return CollectorDialogDisplayType.DECRYPT_OVERWRITE_NO;
            }
            return CollectorDialogDisplayType.UNKNOWN;
        }

        public ReturnTypes ShowFileView()
        {
            log.Info("Called ShowFileView");

            CollectorState res = collectorService.ShowFileView();
            log.Info("result" + res);
            return T(res);
        }
    }
}
