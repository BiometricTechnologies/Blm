using log4net;
using log4net.Appender;
using log4net.Core;
using log4net.Layout;
using log4net.Repository.Hierarchy;
using Microsoft.Win32;
using System;
using System.IO;

namespace IdentaZone.IZService
{
    static class Auxiliary
    {
        static public String PathToIM { get; set; }
        static Auxiliary()
        {
        }


        public static void Init()
        {
#if DEBUG
            SetupLogger(@"Logs\IZService.txt");
#else
            SetupLogger(@"C:\Logs\IdentaZone\IZService.txt");
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


            PatternLayout consoleLayout = new PatternLayout();
            consoleLayout.ConversionPattern = "%timestamp [%thread] %-5level %-15.15logger.%method - %message%newline";
            consoleLayout.ActivateOptions();

            ConsoleAppender consoleAppender = new ConsoleAppender();
            consoleAppender.Layout = consoleLayout;
            hierarchy.Root.AddAppender(consoleAppender);

            hierarchy.Root.Level = Level.Info;
            hierarchy.Configured = true;
        }
    }
}
