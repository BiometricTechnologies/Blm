using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace IdentaZone.IZService
{
    class App : ServiceBase
    {
        static IZService service;
        static void Main(String[] args)
        {

            if ((args.Length > 0) && (args[0] == "/console"))
            {
                // Run the console version here
                service = new IZService();
                Console.WriteLine("Service is available. " +
             "Press <ENTER> to exit.");
                Console.ReadLine();
            }
            else
            {
                ServiceBase.Run(new App());
              //  ServicesToRun = new ServiceBase[] { new IZService() };
              //  ServiceBase.Run(ServicesToRun);
            }
        }


        public App()
        {
            this.ServiceName = "IdentaZone Collector Service";
        }

        protected override void OnStart(string[] args)
        {
            base.OnStart(args);
            service = new IZService();   
        }

        protected override void OnStop()
        {
            base.OnStop();
        }
    }
}
