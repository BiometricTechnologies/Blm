using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Configuration.Install;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.IZService
{
    [RunInstaller(true)]
    public class IZServiceInstaller : Installer
    {
        public IZServiceInstaller()
        {
            var processInstaller = new ServiceProcessInstaller();
            var serviceInstaller = new ServiceInstaller();

            // set the privileges
            processInstaller.Account = ServiceAccount.LocalSystem;

            serviceInstaller.DisplayName = "IdentaZone Collector Service";
            serviceInstaller.Description = "IdentaZone Biosecure Service - data collection and user interaction";
            serviceInstaller.StartType = ServiceStartMode.Automatic;

            serviceInstaller.ServiceName = "IdentaZone Collector Service";
            

            this.Installers.Add(processInstaller);
            this.Installers.Add(serviceInstaller);
        }

    }
}
