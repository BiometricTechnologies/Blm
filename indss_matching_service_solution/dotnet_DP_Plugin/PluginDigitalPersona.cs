using DPUruNet;
using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.Plugins.DP
{
    public class PluginDigitalPersona : IDevicePlugin
    {
        public string Description
        {
            get { return "Plugin is developed for Digital Persona scanners"; }
        }

        public String DeploymentList
        {
            get {
                try
                {
                    ReaderCollection readers = ReaderCollection.GetReaders();
                }
                catch (Exception ex)
                {
                    throw ex;
                }
                return "DP_Plugin.dll";
            }
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
            get { return "Digital Persona"; }
        }

        public Version Version
        {
            get { return new Version(0, 0, 0, 1); }
        }
    }
}
