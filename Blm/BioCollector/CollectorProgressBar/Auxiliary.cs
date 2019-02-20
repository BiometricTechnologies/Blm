using log4net;
using log4net.Appender;
using log4net.Core;
using log4net.Layout;
using log4net.Repository.Hierarchy;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.CollectorProgressBar
{
    class Auxiliary
    {
        static public String PathToIM
        {
            get
            {
                try
                {
                    using (RegistryKey skey = Registry.LocalMachine.OpenSubKey("Software", true))
                    {
                        using (RegistryKey key = skey.OpenSubKey("IdentaMaster", RegistryKeyPermissionCheck.ReadWriteSubTree))
                        {
                            String PathToIM = (string)key.GetValue("InstallDir");
                            return PathToIM;
                        }
                    }
                }
                catch
                {
                    Directory.SetCurrentDirectory(@"C:\CD\");
                    return Directory.GetCurrentDirectory();
                }
            }
        }
        public static class Logger
        {
            static public String PathToIM { get; set; }
            public static void Init()
            {
                SetupLogger(@"C:\Logs\IdentaZone\CollectorProgressBar.txt");
            }
            static Logger()
            {
             


            }
            public static void SetupLogger(String path)
            {

                Hierarchy hierarchy = (Hierarchy)LogManager.GetRepository();
                if (hierarchy.Configured)
                    return;

                PatternLayout patternLayout = new PatternLayout();
                patternLayout.ConversionPattern = "%date [%thread] %-5level %logger - %message%newline";
                patternLayout.ActivateOptions();

                RollingFileAppender roller = new RollingFileAppender();
                roller.AppendToFile = true;
                roller.File = Path.Combine(path);
                roller.Layout = patternLayout;
                roller.MaxSizeRollBackups = 5;
                roller.MaximumFileSize = "10MB";
                roller.RollingStyle = RollingFileAppender.RollingMode.Size;
                roller.StaticLogFileName = true;
                roller.LockingModel = new FileAppender.MinimalLock();
                roller.ActivateOptions();
                hierarchy.Root.AddAppender(roller);

                MemoryAppender memory = new MemoryAppender();
                memory.ActivateOptions();
                hierarchy.Root.AddAppender(memory);

                hierarchy.Root.Level = Level.Info;
                hierarchy.Configured = true;
            }
        }
    }
}
