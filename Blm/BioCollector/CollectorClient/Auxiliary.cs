using log4net;
using log4net.Appender;
using log4net.Core;
using log4net.Layout;
using log4net.Repository.Hierarchy;
using Microsoft.Win32;
using System;
using System.IO;

namespace IdentaZone.Collector
{
    internal static class Auxiliary
    {
        static public String PathToIM { get; set; }
        static Auxiliary()
        {
        }


        public static void Init()
        {
#if DEBUG
            SetupLogger(@"Logs\IZClient.txt");
#else
            SetupLogger(@"C:\Logs\IdentaZone\IZClient.txt");
#endif
        }
        public static void SetupLogger(String path)
        {

            Hierarchy hierarchy = (Hierarchy)LogManager.GetRepository();
            if (hierarchy.Configured)
                return;

            PatternLayout patternLayout = new PatternLayout();
            patternLayout.ConversionPattern = "%date [%thread] %-5level %logger %method - %message%newline";
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
