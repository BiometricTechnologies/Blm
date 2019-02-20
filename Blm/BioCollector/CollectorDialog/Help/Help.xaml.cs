using log4net;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace IdentaZone.Collector.Dialog.Help
{
    /// <summary>
    /// Interaction logic for Help.xaml
    /// </summary>
    public partial class Help : Window
    {
        private readonly ILog _log = LogManager.GetLogger(typeof(Help));
        public Help(String helpString)
        {
            InitializeComponent();
            try
            {
                myWebBrowser.NavigateToString(helpString);
            }
            catch (Exception ex)
            {
                _log.Error("const  failed with " + ex);
            }
        }
    }

    public class HelpLoader
    {
        private readonly ILog _log = LogManager.GetLogger(typeof(HelpLoader));

        private readonly String helpFolder = "help";
        private readonly String helpIndexFileName = "manual_page.htm";
        public String helpString = "";


        public Help GetWindow()
        {
            return new Help(helpString);
        }

        public HelpLoader()
        {
            try
            {
                String executableFolder = AppDomain.CurrentDomain.BaseDirectory;
                helpString = File.ReadAllText(System.IO.Path.Combine(executableFolder, helpFolder) + System.IO.Path.DirectorySeparatorChar + helpIndexFileName);

                helpString = modifyHelpHtml(helpString, System.IO.Path.GetDirectoryName(executableFolder));
            }
            catch (Exception ex)
            {
                _log.Error("Reading help html file failed " + ex);
            }
        }


        String modifyHelpHtml(String input_html, String url)
        {
            // modify path to linux style ("path/file") 
            String pat = "\\\\";
            System.Text.RegularExpressions.Regex pathRegex = new System.Text.RegularExpressions.Regex(pat, System.Text.RegularExpressions.RegexOptions.Compiled | System.Text.RegularExpressions.RegexOptions.IgnoreCase);
            System.Text.RegularExpressions.Match urlMatch = pathRegex.Match(url);
            if (urlMatch.Success)
            {
                // replace all matches
                url = pathRegex.Replace(url, "/", System.Int32.MaxValue);

            }

            // URL format
            url = "file:///" + url;
            url = url.Replace(" ", "%20");

            // modify images relative path to absolute path
            pat = "Image src=";
            System.Text.RegularExpressions.Regex ItemRegex = new System.Text.RegularExpressions.Regex(pat, System.Text.RegularExpressions.RegexOptions.Compiled | System.Text.RegularExpressions.RegexOptions.IgnoreCase);
            System.Text.RegularExpressions.Match match = ItemRegex.Match(input_html);
            if (match.Success)
            {
                // replace all matches
                String res = ItemRegex.Replace(input_html, match.Value + url + "/" + helpFolder + "/", System.Int32.MaxValue);
                return res;

            }

            // not found
            return input_html;
        }
    }
}
