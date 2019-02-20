using CommandLine;
using CommandLine.Text;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IdentaZone.BioSecure
{
    public class Options
    {
        [OptionArray('i', "input", MutuallyExclusiveSet = "source",
            HelpText = "Input file(s) to read. You can't use it with inputfile option")]
        public String[] InputFiles { get; set; }

        [Option('f', "inputfile", MutuallyExclusiveSet = "source",
            HelpText = "Path to file which contains list of targets. You can't use it with target option")]
        public String InputFilePath { get; set; }

        [Option('s', "secure", MutuallyExclusiveSet = "mode",
            HelpText = "Show secure dialog. You can't use it with unsecure option")]
        public Boolean IsSecure { get; set; }

        [Option('u', "unsecure", MutuallyExclusiveSet = "mode",
            HelpText = "Show unsecure dialog. You can't use it with secure option")]
        public Boolean IsUnsecure { get; set; }

        [HelpOption(HelpText = "Display this help screen.")]
        public string GetUsage()
        {
            return HelpText.AutoBuild(this,
                (HelpText current) => HelpText.DefaultParsingErrorsHandler(this, current));
        }

        public void AfterParse()
        {
            if (!IsSecure && !IsUnsecure)
            {
                IsSecure = true;
            }

            if (!String.IsNullOrEmpty(InputFilePath))
            {
                try
                {
                    InputFiles = System.IO.File.ReadAllLines(InputFilePath);
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex);
                }
            }
        }
    }
}
