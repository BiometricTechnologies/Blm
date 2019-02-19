using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IB
{
    public class PluginIntegratedBiometrics : IDevicePlugin
    {
        public String Description
        {
            get { return "Plugin is developed for IdentaBiometrics scanners"; }
        }

        public String DeploymentList
        {
            get { return "IB_Plugin.dll"; }
        }
        /// <summary>
        /// Releases unmanaged and - optionally - managed resources.
        /// </summary>
        public void Dispose()
        {

        }

        /// <summary>
        /// Initializes the specified main container.
        /// </summary>
        /// <param name="MainContainer">The main container.</param>
        /// <exception cref="System.NotImplementedException"></exception>
        public void Initialize(object MainContainer)
        {
        }

        public String Name
        {
            get { return "Integrated Biometrics"; }
        }

        public Version Version
        {
            get { return new Version(0, 0, 0, 1); }
        }
    }
}
