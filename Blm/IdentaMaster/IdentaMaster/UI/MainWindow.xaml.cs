using IdentaZone.IMPlugin;
using IdentaZone.IMPlugin.PluginManager;
using log4net;
using log4net.Config;
using Microsoft.Win32;
using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Security.Permissions;
using System.Security.Principal;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Collections.Generic;

[assembly: log4net.Config.XmlConfigurator(Watch = true)]

namespace IdentaZone.IdentaMaster
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>



    public static class IgnoreCtrlTabBehaviour
    {
        //Setter for use in XAML: this "enables" this behaviour
        public static void SetEnabled(DependencyObject depObj, bool value)
        {
            depObj.SetValue(EnabledProperty, value);
        }

        public static readonly DependencyProperty EnabledProperty =
            DependencyProperty.RegisterAttached("Enabled", typeof(bool),
            typeof(IgnoreCtrlTabBehaviour),
            new FrameworkPropertyMetadata(false, OnEnabledSet));

        static void OnEnabledSet(DependencyObject depObj, DependencyPropertyChangedEventArgs args)
        {
            var uiElement = depObj as UIElement;
            uiElement.PreviewKeyDown +=
              (object _, System.Windows.Input.KeyEventArgs e) =>
              {
                  if (e.Key == Key.Tab)
                      //&& (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
                  {
                      e.Handled = true;
                  }
              };
        }
    }
  
    public partial class MainWindow : Window
    {
        [assembly: log4net.Config.XmlConfigurator(Watch = true)]
        [DllImport("User32.dll", CharSet = CharSet.Auto)]
        public static extern IntPtr SetActiveWindow(

            IntPtr hWnd);

        private readonly ILog Log = LogManager.GetLogger(typeof(MainWindow));

        const string greenColor = "#9FB924";
        int activeTabIndex = 0;
        IUserDB _db = new XmlDB();
        private static Mutex m_Mutex;
        public static readonly string helpFolder = "help";
        public static readonly string helpIndexFileName = "manual_page.htm";
        string helpString = "";
        PluginManager pluginManager = new PluginManager();
        private bool NoDevices = false;
        private List<string> appSet = new List<string>();

        void SystemEvents_SessionSwitch(object sender, SessionSwitchEventArgs e)
        {
            switch (e.Reason)
            {
                case SessionSwitchReason.SessionLock:
                    Log.Warn("Closing. Reason: Session has been locked");
                    this.Close();
                    break;
                case SessionSwitchReason.ConsoleDisconnect:
                    Log.Warn("Closing. Reason: Session has been disconnected from console (Switch User?)");
                    this.Close();
                    break;
            }
        }
        public MainWindow()
        {
            
            XmlConfigurator.Configure();
            Version appVersion = new Version();//
            //Log.Info("Application is starting");
            if (!IsUserAdministrator())
            {
                SetActiveWindow(IntPtr.Zero);
                var result = MessageBox.Show("This program require higher privileges.");
                this.Close();
                return;
            }
            
            Task.Factory.StartNew(() =>
            {
                loadHtmlThread();
            });
            try
            {
                InitializeComponent();
                appVersion = Assembly.GetExecutingAssembly().GetName().Version;
                ApplicationVersion.Content = "V"+appVersion.Major.ToString()+"."+appVersion.Minor.ToString();
                Title += " (v"+appVersion.ToString()+")";
                bool createdNew;
                m_Mutex = new Mutex(true, "IdentaZoneAdminMutex", out createdNew);
                if (!createdNew)
                {
                    SetActiveWindow(IntPtr.Zero);
                    MessageBox.Show("The application is already running.", "Warning", MessageBoxButton.OK);
                    this.Close();
                }
                this.userGrid.ItemsSource = userRows;
                aboutIdentaZoneBtn.IsChecked = true;


                LicenseTabInit();
                // plugins init
                try {
                    pluginManager.LoadPlugins();
                }
                catch (Exception ex)
                {
                    Log.Error(ex);
                }
                if (pluginManager.Plugins.Count == 0)
                {
                    NoDevices = true;
                }
                // Load DB
                _db.Init("usrdb");

                var users = _db.GetAllUsers();
                foreach (var user in users)
                {
                    foreach (var credential in user.credentials_List)
                    {
                        if (credential is FingerCredential)
                        {
                            var fingerCredential = credential as FingerCredential;
                            foreach (var finger in fingerCredential.fingers)
                            {
                                finger.Template = pluginManager.GetTemplate(finger.Type, Convert.FromBase64String(finger.Bytes));
                            }
                        }
                    }
                }
                InitTestTab();
                SystemEvents.SessionSwitch += new SessionSwitchEventHandler(this.SystemEvents_SessionSwitch);
            }
            catch (Exception E)
            {
                Log.Error("Startup failed " + E);
            }
        }


        private void NextPage(object sender, RoutedEventArgs e)
        {
            selectTab(++tabControl.SelectedIndex);
        }

        private bool IsUserAdministrator()
        {
            //bool value to hold our return value
            bool isAdmin;
           try
            {
                //get the currently logged in user
                WindowsIdentity user = WindowsIdentity.GetCurrent();
                WindowsPrincipal principal = new WindowsPrincipal(user);
                isAdmin = principal.IsInRole(WindowsBuiltInRole.Administrator);
            }
            catch (UnauthorizedAccessException ex)
            {
                isAdmin = false;
            }
            catch (Exception ex)
            {
                isAdmin = false;
            }
            // check Registry Write rights
            if (isAdmin)
            {
                try
                {
                    using (RegistryKey skey = Registry.LocalMachine.OpenSubKey("Software", true))
                    {
                        using (RegistryKey key = skey.OpenSubKey("IdentaMaster", RegistryKeyPermissionCheck.ReadWriteSubTree))
                        {
                        }
                    }
                }
                catch (Exception e)
                {
                    Log.Error("Can't open registry for write, Application is run without privileges", e);
                    return false;
                }

                try
                {
                    RegistryPermission perm1 = new RegistryPermission(RegistryPermissionAccess.Write, "HKEY_LOCAL_MACHINE\\SOFTWARE\\");
                    perm1.Demand();
                }
                catch (System.Security.SecurityException ex)
                {
                    return false;
                }
            }
            return isAdmin;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            try
            {
                // update user list
                UpdateUsers();
                // update logs
                updateEventList();
            }
            catch (Exception ex)
            {
                Log.Error("Init failed " + ex);
            }
        }


        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            imageReseter.Stop();
            if (currentTestDevice != null)
            {
                var activeDevice = (currentTestDevice.Content as IFingerDevice);
                if (activeDevice != null)
                {
                    activeDevice.Dispatch(COMMAND.LIVECAPTURE_STOP);
                    currentTestDevice = null;
                }
            }
            if (Licenser.State == IdentaZone.BioControls.Auxiliary.Licenser.STATE.ACTIVATED)
            {
                _db.Deploy(pluginManager.GetDeploymentList());
            }
            
            pluginManager.UnloadPlugins();
            Thread.Sleep(1000);

        }



        void selectTab(int index)
        {
            if (testScannerBtn.IsChecked == true)
            {
                TestScannerDisable();
            }
            if (licensingBtn.IsChecked == true)
            {
                LicesingTabSelected();
            }
            switch (index)
            {
                //About tab
                case 0:
                    aboutIdentaZoneTab.IsSelected = true;
                    aboutIdentaZoneBtn.IsChecked = true;
                    testScannerBtn.IsChecked = false;
                    licensingBtn.IsChecked = false;
                    historyBtn.IsChecked = false;
                    userEnrollBtn.IsChecked = false;
                    break;
                // Test tab
                case 1:
                    testScannerTab.IsSelected = true;
                    aboutIdentaZoneBtn.IsChecked = false;
                    testScannerBtn.IsChecked = true;
                    licensingBtn.IsChecked = false;
                    historyBtn.IsChecked = false;
                    userEnrollBtn.IsChecked = false;
                    if (NoDevices == true)
                    {
                        System.Windows.MessageBox.Show(
                            "Please, install any available biometric plug-in software for IdentaMaster® application to enable particular biometric device."
                            );
                    }
                    break;
                // Licensing tab
                case 2:
                    licensingTab.IsSelected = true;
                    aboutIdentaZoneBtn.IsChecked = false;
                    testScannerBtn.IsChecked = false;
                    licensingBtn.IsChecked = true;
                    historyBtn.IsChecked = false;
                    userEnrollBtn.IsChecked = false;
                    break;
                case 3:
                    historyTab.IsSelected = true;
                    aboutIdentaZoneBtn.IsChecked = false;
                    testScannerBtn.IsChecked = false;
                    licensingBtn.IsChecked = false;
                    historyBtn.IsChecked = true;
                    userEnrollBtn.IsChecked = false;
                    break;
                case 4:
                    userEnrollTab.IsSelected = true;
                    aboutIdentaZoneBtn.IsChecked = false;
                    testScannerBtn.IsChecked = false;
                    licensingBtn.IsChecked = false;
                    historyBtn.IsChecked = false;
                    userEnrollBtn.IsChecked = true;
                    break;
                default:
                    // TODO(an.skornyakov@gmail.com) throw exception or smth
                    break;
            }
        }

        // TODO: modify following monstrous functions (an.skornyakov@gmail.com
        private void ToggleButton_Checked_1(object sender, RoutedEventArgs e)
        {
            activeTabIndex = 0;
            selectTab(activeTabIndex);
        }

        private void TestScannerTabSelected(object sender, RoutedEventArgs e)
        {
            activeTabIndex = 1;
            selectTab(activeTabIndex);
            UpdateTestTab();
        }

        private void ToggleButton_Checked_3(object sender, RoutedEventArgs e)
        {
            activeTabIndex = 2;
            selectTab(activeTabIndex);
        }

        private void ToggleButton_Checked_4(object sender, RoutedEventArgs e)
        {
            activeTabIndex = 3;
            selectTab(activeTabIndex);
        }

        private void ToggleButton_Checked_5(object sender, RoutedEventArgs e)
        {
            activeTabIndex = 4;
            selectTab(activeTabIndex);
        }

        private void ShowHelp(object sender, RoutedEventArgs e)
        {
            Help helpWindow = new Help(helpString);
            helpWindow.Owner = this;
            helpWindow.ShowDialog();
        }


        private void loadHtmlThread()
        {
            try
            {

                helpString = File.ReadAllText(helpFolder + "/" + helpIndexFileName);
                String executableName = System.Windows.Forms.Application.ExecutablePath;
                System.IO.FileInfo executableFileInfo = new System.IO.FileInfo(executableName);
                String executableDirectoryName = executableFileInfo.DirectoryName;
                helpString = modifyHelpHtml(helpString, executableDirectoryName);

            }
            catch (Exception ex)
            {
                Log.Error("Reading help html file failed " + ex);
            }
        }


        static private string modifyHelpHtml(string input_html, string url)
        {
            // modify path to linux style ("path/file") 
            string pat = "\\\\";
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
                string res = ItemRegex.Replace(input_html, match.Value + url + "/" + helpFolder + "/", System.Int32.MaxValue);
                return res;

            }

            // not found
            return input_html;
        }

        private void userGrid_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                userGridClickReaction();
                e.Handled = true;
            }
            if (e.Key == Key.Tab)
            {
                e.Handled = true;
            }

        }

   
    }
}
