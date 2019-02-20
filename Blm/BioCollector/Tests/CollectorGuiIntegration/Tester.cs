using IdentaZone.Collector;
using IdentaZone.IZService;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CollectorGuiIntegration
{
    class Tester
    {

        public Tester()
        {
            Task.Factory.StartNew(() =>
           {
               var izService = new IZService();
           });
        }

        CollectorClient client = null;

        internal void ShowNewGUI()
        {
            try
            {
                client = new CollectorClient();
                var time = System.Environment.TickCount;
                client.Init();
                while (client.IsReady() != IdentaZone.ReturnTypes.rtOK)
                {
                    Thread.Sleep(100);
                }
                time = System.Environment.TickCount - time;
                Console.WriteLine("Init has taken {0} ticks", time);

                client.AddCryptoProvider("MS CRYPTO");
                client.AddCryptoProvider("SuperBlowFish");

                IdentaZone.CollectorDialogData data = new IdentaZone.CollectorDialogData()
                {
                    Filename = "test.izbiosecure",
                    Username = "Grand Biosec",
                    mode = IdentaZone.CollectorDialogMode.cdmDecryptNoOverwrite
                };

                client.ShowDialogEx(data);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

        internal void LoginUser()
        {
            client.UpdateState(IdentaZone.StateTypes.stGoodBiometric);
        }

        internal void IdentificationFail()
        {
            client.UpdateState(IdentaZone.StateTypes.stBadBiometric);
        }

        internal void WrongUser()
        {
            client.UpdateState(IdentaZone.StateTypes.stBadBiometricUser);
        }
    }
}
