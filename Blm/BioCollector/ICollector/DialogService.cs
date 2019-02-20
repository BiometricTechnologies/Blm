using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceModel;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone
{
    public interface IDialogCallbackService
    {
        [OperationContract(IsOneWay = true)]
        void NotifyClient();
    }

    [ServiceContract(CallbackContract = typeof(IDialogCallbackService))]
    public interface IDialogService
    {
        [OperationContract]
        bool IsAlive();
    }
}
