using CommandLine;
using CommandLine.Text;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows;

namespace IdentaZone.BioSecure
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        [DllImport("kernel32", SetLastError = true)]
        private static extern bool AttachConsole(int dwProcessId);

        [DllImport("kernel32", SetLastError = true)]
        private static extern bool FreeConsole();

        private void Application_Startup(object sender, StartupEventArgs e)
        {
            Auxiliary.Logger.Init();

            var options = new Options();

            foreach (var arg in e.Args)
            {
                //_log.Info("Input param " + arg);
            }

            AttachConsole(-1);
            var parser = new CommandLine.Parser(with =>
            {
                with.MutuallyExclusive = true;
                with.CaseSensitive = true;
                with.HelpWriter = Console.Error;
            });

            

            if (parser.ParseArguments(e.Args, options))
            {
                options.AfterParse();
                try{
                    var cd = new CollectorDialog(options);
                    cd.Show();
                }
                catch (Exception ex)
                {
                    Auxiliary.Logger._log.Error("Error: " + ex.Message);
                }
            }

            FreeConsole();
        }
    }
}
