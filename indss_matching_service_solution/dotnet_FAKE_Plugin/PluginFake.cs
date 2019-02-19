using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.Plugins.FAKE
{
    public class PluginFake : IDevicePlugin
    {
        public string Name
        {
            get { return "Fake Plugin"; }
        }

        public Version Version
        {
            get { return new Version("1.0"); }
        }

        public string Description
        {
            get { return "Fake plugin for testing purposes"; }
        }

        public string DeploymentList
        {
            get { return ""; }
        }

        public void Initialize(object MainContainer)
        {
            
        }

        public void Dispose()
        {
            
        }
    }
}
