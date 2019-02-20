using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceModel;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.IdentaMasterServices
{
    [ServiceContract(CallbackContract = typeof(IClientCallback))]
    public interface IDialogClientService
    {
        [OperationContract]
        bool Login();

        [OperationContract]
        void Logout();

        [OperationContract]
        CollectorState Log(LogRecord record);

    }
}
