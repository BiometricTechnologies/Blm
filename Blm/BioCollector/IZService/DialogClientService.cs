using IdentaZone.IdentaMasterServices;
using log4net;
using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceModel;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.IZService
{
    [ServiceBehavior(InstanceContextMode = InstanceContextMode.Single)]
    class DialogClientService : IDialogClientService
    {

        public IClientCallback callback;

        IZService Service;


        public DialogClientService(IZService service)
        {
            Service = service;
        }

        public bool Login()
        {
           return true;
        }

       

        public CollectorState Log(LogRecord record)
        {
            if (Service.Log(record))
            {
                return CollectorState.OK;
            }
            else
            {
                return CollectorState.NO_DATA;
            }
        }

        public void Logout()
        {
            callback = null;
        }

        
    }
}
