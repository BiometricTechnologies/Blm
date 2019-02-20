using IdentaZone.IdentaMasterServices;
using log4net;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.ServiceModel;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Timers;

namespace IdentaZone.IZService
{

    public class IZService
    {
        DialogClientService dialogClient;
        ServiceHost dialogClientHost;
        EventLogger EventLogger = new SimpleEventLogger();

        ~IZService()
        {

        }

        public IZService()
        {
            try
            {
                var binding = new NetNamedPipeBinding();
                binding.ReceiveTimeout = System.TimeSpan.FromHours(24);
                dialogClient = new DialogClientService(this);
                dialogClientHost = new ServiceHost(dialogClient, new Uri(("net.pipe://localhost/IdentaZone/IMLogger")));

                dialogClientHost.AddServiceEndpoint(typeof(IDialogClientService), binding, "DialogClient");
                dialogClientHost.Open();
                dialogClientHost.Faulted += host_Faulted;
            } catch (Exception ex)
            {
                //log.Info("Cannot start, maybe another instance working? ", Exception);
            }
        }


        public CollectorState Init()
        {
            return CollectorState.OK;
        }



        void host_Faulted(object sender, EventArgs e)
        {
            //log.Error(sender.ToString() + e);
        }



        internal bool Log(LogRecord record)
        {
            try
            {
                String op = "";
                if (record.operation == LogOperation.DECRYPT)
                {
                    op = "decrypted ";
                }
                else
                {
                    op = "encrypted ";
                }
                String message = String.Format("File {1} has been {0} by {3}", op, record.filename, record.provider, record.username);
                EventLogger.Log(message);
                return true;
            }
            catch (Exception ex)
            {
                //log.Error("Can't write to log", ex);
                return false;
            }
        }
		
    }
}
