using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SG
{
    public class PluginSecuGen : IDevicePlugin
    {
        public string Description
        {
            get { return "Plugin is developed for SecuGen scanners"; }
        }

        public String DeploymentList
        {
            get { return "SG_Plugin.dll"; }
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
        public void Initialize(object MainContainer)
        {
        }

        public string Name
        {
            get { return "SeguGen"; }
        }

        public Version Version
        {
            get { return new Version(0, 0, 0, 1); }
        }
    }
}
