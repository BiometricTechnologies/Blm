using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hitachi
{
    public class PluginHitachi : IDevicePlugin
    {
        public string Description
        {
            get { return "Plugin is developed for Hitachi finger vein scanners"; }
        }

        public String DeploymentList
        {
            get { return "Hi_Plugin.dll"; }
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
            log4net.Config.BasicConfigurator.Configure(new log4net.Appender.FileAppender(
                new log4net.Layout.PatternLayout("%d [%t]%-5p %c [%x] ;%X{auth}; - %m%n"), "c:\\logs\\Hiplugin.log"));
        }

        public string Name
        {
            get { return "Hitachi Finger Vein Scanner"; }
        }

        public Version Version
        {
            get { return new Version(0, 0, 0, 1); }
        }
    }
}
