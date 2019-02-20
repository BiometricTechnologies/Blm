using IdentaZone;
using IdentaZone.BioConrols.Auxiliary;
using IdentaZone.Collector.Dialog.Help;
using IdentaZone.CollectorServices;
using IdentaZone.IMPlugin;
using IdentaZone.IMPlugin.PluginManager;
using log4net;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Security.Permissions;
using System.ServiceModel;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Timers;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Threading;

namespace IdentaZone.Collector.Dialog
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class CollectorDialog : Window, IDialogGUICallback
    {
        // Logger instance
        protected readonly ILog log = LogManager.GetLogger(typeof(CollectorDialog));

        public static readonly SolidColorBrush COLOR_OK = new SolidColorBrush(Colors.DarkGreen);
        public static readonly SolidColorBrush COLOR_BAD = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#FF3000"));

        HelpLoader _helpLoader;
        FileChooser _fileChooser;

        System.Timers.Timer Wdt = new System.Timers.Timer();

        // Pipe to server
        IDialogGUIService _collectorServer = null;
        ChannelFactory<IDialogGUIService> pipeFactory;

        // Plugin manager
        PluginManager pm = new PluginManager();
        IFingerDevice currDevice = null;

        // how frequent is timer checked
        const int RESETER_TIMER_PERIOD_MS = 100;
        // How long will Image last on page
        const int RESETER_IMAGE_LIFETIME_MS = 2000;

        const int RESETER_MESSAGE_LIFETIME_MS = 3000;

        DispatcherTimer imageReseter = null;
        DispatcherTimer _messageReseter = null;


        // Timer for fingerImageBoxClearing
        int nextReset = 0;
        int textReset = 0;

        private Licenser Licenser;

        public String baseMessage = "";

        private String Username = "";
        public String Filename { get; set; }

        private readonly BackgroundWorker imageFetcher = new BackgroundWorker();
        private readonly BackgroundWorker refresher = new BackgroundWorker();
        private readonly BackgroundWorker enumerator = new BackgroundWorker();

        private String PathToProgram = "";

        DispatcherTimer destroyer;
        private CollectorDialogState _currState = CollectorDialogState.NOT_SET;

        public enum CollectorMode
        {
            ENCRYPT,
            DECRYPT
        }

        public CollectorMode Mode { get; set; }

        public CollectorDialog()
        {
            Auxiliary.Logger.Init();
            log.Info("Const");
            InitializeComponent();
            this.Hide();

            // Connect to IZService
            log.Info("Connecting to IZService");
            try
            {
                var binding = new NetNamedPipeBinding();
                binding.MaxBufferSize = BioData.MaxSize;
                binding.MaxReceivedMessageSize = binding.MaxBufferSize;

                pipeFactory =
                   new DuplexChannelFactory<IDialogGUIService>(new InstanceContext(this),
                       binding,
                       new EndpointAddress("net.pipe://localhost/IdentaZone/Collector/DialogGui"));

                _collectorServer = pipeFactory.CreateChannel();

                if (!_collectorServer.Login())
                {
                    log.Error("Service is busy");
                    this.Close();
                    return;
                }
                else
                {
                    log.Info("Connected to service");
                }

                log.Info("Trying to get App directory");
                PathToProgram = _collectorServer.GetApplicationDirectory();

            }
            catch (Exception ex)
            {
                log.Info(ex);
                this.Close();
                return;
            }


            Dispatcher.BeginInvoke(new Action(() => SignalizeReady()));

            Dispatcher.BeginInvoke(new Action(() => InitPM()), DispatcherPriority.ContextIdle);
            log.Info(".ctr finished");
        }


        private void InitPM()
        {
            _helpLoader = new HelpLoader();

            // Licenser
            log.Info("Starting licenser");
            Licenser = new Licenser();
            Licenser.Init(new Action(() => LicenseIsActivated()));

            log.Info("Starting enumerator worker");
            RefreshBtn.IsEnabled = false;
            enumerator.DoWork += EnumeratorWork;
            enumerator.RunWorkerAsync(pm);

            log.Info("Starting image reset timer");
            // Image Reseter
            imageReseter = new System.Windows.Threading.DispatcherTimer();
            imageReseter.Tick += new EventHandler(imageClearTimerHandler);
            imageReseter.Interval = new TimeSpan(0, 0, 0, 0, RESETER_TIMER_PERIOD_MS);
            imageReseter.Start();
            // Message Reseter
            _messageReseter = new System.Windows.Threading.DispatcherTimer();
            _messageReseter.Tick += new EventHandler(messageClearTimerHandler);
            _messageReseter.Interval = new TimeSpan(0, 0, 0, 0, RESETER_MESSAGE_LIFETIME_MS);

            log.Info("Setup refresher");
            // setup background worker for refresh
            refresher.DoWork += refreshOneDevice;
            imageFetcher.DoWork += fetchImage;

        }

        private void messageClearTimerHandler(object sender, EventArgs e)
        {
            ShowState();
            _messageReseter.Stop();
        }

        private void AssociationCheck()
        {
            try
            {
                using (RegistryKey regkey = Registry.CurrentUser.OpenSubKey(@"Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\", true))
                {
                    var biosecKey = regkey.OpenSubKey(".izbiosecure", true);
                    if (biosecKey != null)
                    {
                        log.Info("RegKey is " + biosecKey.Name);
                        foreach (String key in biosecKey.GetSubKeyNames())
                            biosecKey.DeleteSubKey(key);
                        biosecKey.Close();
                        regkey.DeleteSubKeyTree(@".izbiosecure");
                    }
                }
            }
            catch (Exception ex)
            {
                log.Error("File association check failed ", ex);
            }
        }


        private void EnumeratorWork(object sender, DoWorkEventArgs e)
        {
            log.Info("Loading PM");
            pm = (PluginManager)e.Argument;
            try
            {
                pm.LoadPlugins(Path.Combine(PathToProgram, @"CollectorDialog\"));
            }
            catch (Exception ex)
            {
                log.Error(ex);
            }

            log.InfoFormat("Found {0} deviceControls", pm.DeviceControls.Count);
            // populate device ctrl tab
            foreach (var deviceCtrl in pm.DeviceControls)
            {
                DoEvents();
                log.InfoFormat("Adding devices from device control {0} to GUI", deviceCtrl);
                try
                {
                    deviceCtrl.EnumerateDevices();
                    Dispatcher.BeginInvoke(new Action(() => AddDeviceControl(deviceCtrl)));

                }
                catch (Exception ex)
                {
                    log.Error(ex);
                }
            }
            Dispatcher.BeginInvoke(new Action(() => RefreshBtn.IsEnabled = true));
        }

        private void AddDeviceControl(IDeviceControl deviceCtrl)
        {
            RadioButton radioButton = new RadioButton();

            radioButton.Checked += DeviceSelected;
            radioButton.Unchecked += DeviceDeselected;
            if (deviceCtrl.ActiveDevices.Count == 0)
            {
                radioButton.Content = deviceCtrl.ToString();
                radioButton.IsChecked = false;
                radioButton.IsEnabled = true;

                radioButton.Style = this.FindResource("IZRadioButton") as Style;

                TestDevicePanel.Children.Add(radioButton);
            }
            else
            {
                radioButton.Content = deviceCtrl.ActiveDevices[0];
                radioButton.IsChecked = false;
                radioButton.Style = this.FindResource("IZRadioButton") as Style;
                TestDevicePanel.Children.Add(radioButton);
            }
        }




        private void LicenseIsActivated()
        {
            log.Info("License check is finished");
#if DEBUG
            log.Info("License is OK");
            Dispatcher.BeginInvoke(new Action(() => LicenseActivationFinished()));
#else
            if (Licenser.State != Licenser.STATE.ACTIVATED)
            {
                log.Error("No license found");
                Dispatcher.BeginInvoke(new Action(() => LicenseActivationFinished()));
            }
            else
            {
                log.Info("License is OK");
                Dispatcher.BeginInvoke(new Action(() => LicenseActivationFinished()));
            }
#endif
        }

        private void LicenseActivationFinished()
        {
            if (_currState != CollectorDialogState.NOT_SET)
            {
                UpdateState(_currState);
            }
        }


        private void SignalizeReady()
        {
            log.Info("Sending ready signal");
            _collectorServer.Ready();
            _collectorServer.DeleteAfterChanged(DeleteAfterCB.IsChecked.Value);

            if (Constants.IS_WDT_ENABLED)
            {
                log.Info("Starting WDT");
                Wdt.Elapsed += new ElapsedEventHandler(OnWdt);
                Wdt.Interval = Constants.KEEP_ALIVE * 6;
                Wdt.Enabled = true;
            }
        }

        private void OnWdt(object sender, ElapsedEventArgs e)
        {
            log.Error("WDT timer alarm!, lost connection to service, closing");
            Destroy();
        }

        /// <summary>
        /// Refreshes the one device.
        /// </summary>
        /// <param name="Sender">The Sender.</param>
        /// <param name="e">The <see cref="DoWorkEventArgs"/> instance containing the event data.</param>
        private void refreshOneDevice(object sender, DoWorkEventArgs e)
        {
            (e.Argument as IDeviceControl).EnumerateDevices();
        }

        private void DeviceDeselected(object sender, RoutedEventArgs e)
        {
            RadioButton radioButton = (sender as RadioButton);
            try
            {
                // If Provider has an active device
                if (radioButton.Content is IFingerDevice)
                {
                    IFingerDevice device = (sender as RadioButton).Content as IFingerDevice;
                    Ambassador.ClearCallback();

                    device.Dispatch(COMMAND.LIVECAPTURE_STOP);
                    (sender as RadioButton).IsEnabled = true;
                }
            }
            catch (Exception ex)
            {
                log.Error(ex);
            }
        }

        private void DeviceSelected(object sender, RoutedEventArgs e)
        {
            RadioButton rb = (sender as RadioButton);
            try
            {
                checkFinger.SetText("");
                // If Provider has an active device
                if (rb.Content is IFingerDevice)
                {
                    Ambassador.SetCallback(DispatchMessage);
                    currDevice = rb.Content as IFingerDevice;
                    currDevice.Dispatch(COMMAND.SINGLECAPTURE_START);
                    checkFinger.IsDeviceConnected = true;
                }
                else
                {
                    checkFinger.IsDeviceConnected = false;
                }
            }
            catch (Exception ex)
            {
                log.Error(ex);
            }
        }

        private void DispatchMessage(Message msg)
        {
            if (msg is MBiometricsSingleCaptured)
            {
                BiometricsCaptured(msg.Sender, (msg as MBiometricsSingleCaptured).Image);
            }
            else if (msg is MShowText)
            {
                MessageSend(msg.Sender, (msg as MShowText).Message);
            }
            else if (msg is MSignal)
            {
                SignalCaptured(msg.Sender, (msg as MSignal).Signal);
            }
        }

        public void DeviceFailure(Object sender)
        {
            // only for our device
            foreach (var radioButton in TestDevicePanel.Children.OfType<RadioButton>().Where(rb => rb.Content == sender))
            {
                radioButton.Content = sender.ToString();

                Ambassador.ClearCallback();
                //MessageSend(sender, "Connect device and press refresh");
                checkFinger.IsDeviceConnected = false;
                radioButton.IsEnabled = true;
            }
        }

        private void SignalCaptured(object sender, MSignal.SIGNAL sig)
        {
            try
            {
                switch (sig)
                {
                    case MSignal.SIGNAL.DEVICE_FAILURE:
                        this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Background,
                            new Action(() => DeviceFailure(sender)));
                        break;
                }
            }
            catch (Exception ex)
            {
                log.Error(ex);
            }
        }

        private void MessageSend(Object sender, String text)
        {
            try
            {
                this.Dispatcher.Invoke(new Action(() => checkFinger.SetText(text)));
                this.Dispatcher.Invoke(new Action(() => nextReset = RESETER_IMAGE_LIFETIME_MS));
            }
            catch (Exception ex)
            {
                log.Error(ex);
            }
        }

        private void BiometricsCaptured(Object sender, FingerImage fingerImage)
        {
            try
            {
                this.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal,
                    new Action(() => ProcessImage(sender, fingerImage)));
            }
            catch (Exception ex)
            {
                log.Error(ex);
            }
        }


        private void fetchImage(object sender, DoWorkEventArgs e)
        {
            var image = e.Argument as FingerImage;
            String type, data;

            Dispatcher.Invoke(new Action(() => currDevice.Dispatch(COMMAND.SINGLECAPTURE_STOP)));
            Thread.Sleep(250); // it's magic!

            image.Serialize(out type, out data);
            var bioData = new BioData(type, data);
            Dispatcher.Invoke(new Action(() => SendBio(bioData)));
            FingerPicture pic = image.MakePicture();
            Dispatcher.Invoke(new Action(() => showImage(pic)));
        }


        void SendBio(BioData data)
        {
            _collectorServer.SendBiometrics(data, CryptoProvidersCombo.SelectedIndex);
        }

        void ProcessImage(Object sender, FingerImage image)
        {
            try
            {
                // Do not process signals from another devices
                if (sender == currDevice)
                {
                    if (image != null)
                    {
                        if (Licenser.State == Licenser.STATE.ACTIVATED)
                        {
                            if (!imageFetcher.IsBusy)
                            {
                                imageFetcher.RunWorkerAsync(image);
                            }
                        }
                        else
                        {
                            ShowMessage(true);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                log.Error(ex);
            }
        }



        private void showImage(FingerPicture pic)
        {
            checkFinger.ShowImage(pic.Image, pic.Width, pic.Height);
            nextReset = RESETER_IMAGE_LIFETIME_MS;
        }

        /// <summary>
        /// Every IMAGE_RESETER_TIMER_PERIOD_MS check if we need to Clear some Image 
        /// </summary>
        /// <param name="Sender"></param>
        /// <param name="e"></param>
        private void imageClearTimerHandler(object sender, EventArgs e)
        {

            if (nextReset > 0)
            {
                nextReset -= RESETER_TIMER_PERIOD_MS;
                if (nextReset <= 0)
                {
                    try
                    {
                        checkFinger.ClearImage();
                        checkFinger.SetText("");
                    }
                    catch (Exception ex)
                    {
                        log.Error("Error while stopping the timer" + ex);
                    }
                }
            }
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            log.Info("Window closed");
            try
            {
                log.Info("Trying to logout");
                _collectorServer.Logout();
                log.Info("Closing factory");
                pipeFactory.Close();
            }
            catch (Exception ex)
            {
                log.Error(ex);
            }
        }

        void updateSelection()
        {
        }

        void IDialogGUICallback.AddProvider(String provider)
        {
            AddProvider(provider);
        }

        void AddProvider(String provider)
        {
            try
            {
                CryptoProvidersCombo.Items.Add(provider);
                if (CryptoProvidersCombo.SelectedIndex == -1)
                {
                    CryptoProvidersCombo.SelectedIndex = 0;
                }
            }
            catch (Exception ex)
            {
                log.Error(ex);
            }
        }

        public String Truncate(String value, int maxLength)
        {
            if (String.IsNullOrEmpty(value)) return value;
            return value.Length <= maxLength ? value : value.Substring(0, maxLength);
        }




        public void ShowWindowEx(CollectorDialogInitData initData)
        {
            Filename = initData.Filename;
            Username = initData.Username;

            switch (initData.Mode)
            {
                case CollectorDialogDisplayType.ENCRYPT:
                    Mode = CollectorMode.ENCRYPT;
                    break;
                case CollectorDialogDisplayType.DECRYPT_OVERWRITE_NO:
                    Mode = CollectorMode.DECRYPT;
                    DeleteAfterCB.Visibility = System.Windows.Visibility.Collapsed;
                    break;
                case CollectorDialogDisplayType.DECRYPT_OVERWRITE_YES:
                    Mode = CollectorMode.DECRYPT;
                    break;
                default:
                    log.Error("Unacceptable work mode");
                    break;
            }

            ShowState();

            try
            {
                log.Info("Show Window called");
                this.Topmost = true;
                this.Show();

                // Check if association is ok?
                AssociationCheck();
            }
            catch (Exception ex)
            {
                log.Info(ex);
            }
        }

        private void ShowState()
        {
            if (Mode == CollectorMode.ENCRYPT)
            {
                Prompt.Text = "Please, proceed with biometric authentication to start encryption process";
                Prompt.Background = COLOR_OK;
                this.Title = "IdentaMaster - Secure";
                DeleteAfterCB.Content = "Delete original file / folder after encryption";
            }
            else
            {
                Prompt.Text = "Please, proceed with biometric authentication to start decryption process";
                Prompt.Background = COLOR_OK;
                this.Title = "IdentaMaster - Unsecure";
                DeleteAfterCB.Content = "Overwrite original file / folder";

            }
        }

        public void ShowFileView()
        {
            _currState = CollectorDialogState.SHOW_FILE_LIST;
            ShowMessage(false);
        }

        private void ShowMessage(bool ShowUnregistredMessage)
        {

            _messageReseter.Stop();
            _messageReseter.Start();
            if (!ShowUnregistredMessage)
            {
                switch (_currState)
                {
                    case CollectorDialogState.GOOD_BIOMETRICS:
                        Prompt.Text = "Identified as " + Truncate(Username, 20);
                        Prompt.Background = COLOR_OK;
                        break;
                    case CollectorDialogState.BAD_BIOMETRICS:
                        Prompt.Text = "Identification failure";
                        Prompt.Background = COLOR_BAD;
                        Dispatcher.Invoke(new Action(() => currDevice.Dispatch(COMMAND.SINGLECAPTURE_START)));
                        break;
                    case CollectorDialogState.BAD_BIOMETRICS_USER:
                        Prompt.Text = "Not identified as data owner";
                        Prompt.Background = COLOR_BAD;
                        Dispatcher.Invoke(new Action(() => currDevice.Dispatch(COMMAND.SINGLECAPTURE_START)));
                        break;
                    case CollectorDialogState.BAD_USER_LOGGED_IN:
                        Prompt.Text = "Wrong user logged in";
                        Prompt.Background = COLOR_OK;
                        Dispatcher.Invoke(new Action(() => currDevice.Dispatch(COMMAND.SINGLECAPTURE_START)));
                        break;
                    case CollectorDialogState.SHOW_FILE_LIST:
                        _fileChooser = new FileChooser(this, Filename);
                        _fileChooser.Show();
                        break;
                    case CollectorDialogState.NOT_IDENTIFIED_AS_CURRENT_USER:
                        Prompt.Text = String.Format("Not identified as current user ({0})", Truncate(Username, 20));
                        Prompt.Background = COLOR_BAD;
                        Dispatcher.Invoke(new Action(() => currDevice.Dispatch(COMMAND.SINGLECAPTURE_START)));
                        break;
                }
            }
            else
            {
                if (Licenser.State == BioConrols.Auxiliary.Licenser.STATE.LOADING)
                {
                    Prompt.Text = "Please, wait while license is checked";
                    Prompt.Background = COLOR_OK;
                }
                else if (Licenser.State == BioConrols.Auxiliary.Licenser.STATE.NOT_REGISTERED)
                {
                    Prompt.Text = "Please, activate IdentaMaster to enable files and folders encryption capabilities";
                    Prompt.Foreground = COLOR_BAD;
                }
            }
        }

        public void UpdateState(CollectorDialogState newState)
        {
            _currState = newState;
            log.InfoFormat("Update State {0}", _currState);
            ShowMessage(false);
        }


        /// <summary>
        /// Send current GUI function to "sleep", allowing GUI Dispatcher to refresh GUI
        /// </summary>
        [SecurityPermissionAttribute(SecurityAction.Demand, Flags = SecurityPermissionFlag.UnmanagedCode)]
        public void DoEvents()
        {
            try
            {
                DispatcherFrame frame = new DispatcherFrame();
                this.Dispatcher.Invoke(DispatcherPriority.Background,
                    new DispatcherOperationCallback(ExitFrame), frame);
                Dispatcher.PushFrame(frame);
            }
            catch (Exception ex)
            {
                log.Error(ex);
            }
        }

        public object ExitFrame(object f)
        {
            ((DispatcherFrame)f).Continue = false;
            return null;
        }

        /// <summary>
        /// Refreshes the test page.
        /// </summary>
        private void RefreshTestPage()
        {
            foreach (var deviceCtrl in pm.DeviceControls)
            {
                var currentRadioButtons = TestDevicePanel.Children.OfType<RadioButton>().
                    Where(rb => rb.Content.ToString() == deviceCtrl.ToString());

                // Block all RB
                foreach (var radioButton in currentRadioButtons)
                {
                    radioButton.IsEnabled = false;
                }

                // Run Refresh in separate worker thread
                refresher.RunWorkerAsync(deviceCtrl);
                while (refresher.IsBusy)
                {
                    DoEvents();
                }

                // If no device is online
                if (deviceCtrl.ActiveDevices.Count == 0)
                {
                    foreach (var radioButton in currentRadioButtons)
                    {
                        radioButton.Content = deviceCtrl.ToString();
                    }
                }
                // If some devices are online
                else
                {
                    foreach (var radioButton in currentRadioButtons)
                    {
                        radioButton.Content = deviceCtrl.ActiveDevices[0];
                        if (radioButton.IsChecked == true)
                        {
                            DeviceSelected(radioButton, null);
                        }
                    }
                }

                // Unblock RB
                foreach (var radioButton in currentRadioButtons)
                {
                    radioButton.IsEnabled = true;
                }
            }
            RefreshBtn.IsEnabled = true;
        }

        private void RefreshBtn_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                RefreshBtn.IsEnabled = false;
                this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal,
                    (new Action(() => RefreshTestPage())));
            }
            catch (Exception ex)
            {
                RefreshBtn.IsEnabled = true;
                log.Error(ex);
            }
        }

        private void ShowHelpBtn_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                var helpWindow = _helpLoader.GetWindow();
                helpWindow.Owner = this;
                helpWindow.ShowDialog();
            }
            catch (Exception ex)
            {
                log.Error("ShowHelpButton click failed with " + ex);
            }

        }

        private void DeleteAfterCB_Checked(object sender, RoutedEventArgs e)
        {
            _collectorServer.DeleteAfterChanged(true);
        }

        private void DeleteAfterCB_Unchecked(object sender, RoutedEventArgs e)
        {
            _collectorServer.DeleteAfterChanged(false);
        }

        public void HideWindow()
        {
            this.Hide();
        }


        public void Destroy()
        {
            if (destroyer == null)
            {
                if (Constants.IS_WDT_ENABLED)
                {
                    Wdt.Dispose();
                }
                this.Dispatcher.BeginInvoke(new Action(() => this.Close()));
            }
        }
        public void HideWindowAfter(int timeout)
        {
            destroyer = new System.Windows.Threading.DispatcherTimer();
            destroyer.Tick += new EventHandler(DestroyTimerEvent);
            destroyer.Interval = new TimeSpan(0, 0, 0, 0, timeout);
            destroyer.Start();
        }

        private void DestroyTimerEvent(object sender, EventArgs e)
        {
            destroyer.Stop();
            if (Constants.IS_WDT_ENABLED)
            {
                Wdt.Dispose();
            }
            this.Close();
        }


        public void Heartbeat()
        {
            Wdt.Stop();
            Wdt.Start();
        }

        public void AddFile(BioFileInfo bioFileInfo)
        {
            try
            {
                //log.InfoFormat("Adding file {0} with num {1}", bioFileInfo.Filename, bioFileInfo.FileNumber);
                _fileChooser.AddFile(bioFileInfo);
            }
            catch (Exception ex)
            {
                log.Error("can't add file ", ex);
            }
        }

        internal void NotifyDecryptAll()
        {
            _collectorServer.NotifyDecryptAll();
        }

        internal void NotifyDecryptList(List<int> list)
        {
            _collectorServer.NotifyDecryptList(list);
        }


    }
}
