using IdentaZone.BioControls.Auxiliary;
using IdentaZone.IMPlugin;
using IdentaZone.IMPlugin.PluginManager;
using IdentaZone.IdentaMasterServices;
using log4net;
using Microsoft.Win32;
using System;
using System.ServiceModel;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Security.Permissions;
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
using System.Runtime.InteropServices;

namespace IdentaZone.BioSecure
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class CollectorDialog : Window
    {
        #region types

        private enum CollectorDialogState
        {
            IDLE = 0,
            BAD_BIOMETRICS,
            GOOD_BIOMETRICS,
            BAD_BIOMETRICS_USER,
            NOT_SET,
            BAD_USER_LOGGED_IN,
            NOT_IDENTIFIED_AS_CURRENT_USER,
            BAD_ENCRYTPION_KEY
        }

        private enum ResultState
        {
            SHOW_FILECHOOSER,
            SHOW_PB_ENC,
            EXIT,
            SHOW_PB_DEC_ALL,
            SHOW_PB_DEC_SOME,
        }

        #endregion

        #region constants

        // how frequent is timer checked
        const int RESETER_TIMER_PERIOD_MS = 100;
        // How long will Image last on page
        const int RESETER_IMAGE_LIFETIME_MS = 2000;

        const int RESETER_MESSAGE_LIFETIME_MS = 3000;

        const UInt64 maxEntriesToShowFileChooserInstantly = 10;
        private static readonly SolidColorBrush COLOR_OK = new SolidColorBrush(Colors.DarkGreen);
        private static readonly SolidColorBrush COLOR_BAD = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#FF3000"));

        #endregion

        #region Fields

        private static Mutex m_Mutex;

        private ResultState _result;



        //private EventLogger _eventLogger;
        private ProgressBar _progressBar;
        private HelpLoader _helpLoader;
        private FileChooser _fileChooser;
        private ErrorWindow _errorWindow;
        private bool _errorWindowShown;

        // Plugin manager
        private PluginManager _pm = new PluginManager();
        private IFingerDevice _currDevice = null;

        private DispatcherTimer imageReseter = null;
        private DispatcherTimer _messageReseter = null;
        private DispatcherTimer _destroyer = null;

        private readonly BackgroundWorker _liveImageFetcher = new BackgroundWorker();
        private readonly BackgroundWorker _imageFetcher = new BackgroundWorker();
        private readonly BackgroundWorker _refresher = new BackgroundWorker();
        private readonly BackgroundWorker _enumerator = new BackgroundWorker();

        private Thread _fileListFiller;

        // Timer for fingerImageBoxClearing
        private int _nextReset = 0;

        private Licenser _licenser;

        private String baseMessage = "";

        private String _username = "";
        private String _filename;

        private CollectorDialogState _currState = CollectorDialogState.IDLE;
        private Options _options;

        private BiosecLibClient _libClient = new BiosecLibClient();
        private BiosecLibClient.BiometricProcessedDelegate _biometricProcessionFinished;
        private BiosecLibClient.UpdateDelegate _updateProgress;
        private BiosecLibClient.FinishDelegate _fileOperationFinished;
        private BiosecLibClient.ErrorDelegate _fileOperationError;
        private BiosecLibClient.CryptoOperationRes _fileOperationRes = BiosecLibClient.CryptoOperationRes.BIOSEC_NO_ERROR;

        private bool _IsDisabled;
        private volatile bool _stopFillingList;

        // Pipe to server
        IDialogClientService loggingService = null;
        ChannelFactory<IDialogClientService> pipeFactory;
        bool IsConnected = false;
        private bool arrowClicked = false;
        #endregion


        #region Constructor

        void SystemEvents_SessionSwitch(object sender, SessionSwitchEventArgs e)
        {
            switch (e.Reason)
            {
                case SessionSwitchReason.SessionLock:
                    Auxiliary.Logger._log.Warn("Closing. Reason: Session has been locked");
                    this.Close();
                    break;
                case SessionSwitchReason.ConsoleDisconnect:
                    Auxiliary.Logger._log.Warn("Closing. Reason: Session has been disconnected from console (Switch User?)");
                    this.Close();
                    break;
            }
        }
        [DllImport("User32.dll", CharSet = CharSet.Auto)]
        public static extern IntPtr SetActiveWindow(

            IntPtr hWnd);

        public CollectorDialog(Options options)
        {
            bool createdNew;
            m_Mutex = new Mutex(true, "IdentaZoneAdminMutex", out createdNew);
            if (!createdNew)
            {
                SetActiveWindow(IntPtr.Zero);
                MessageBox.Show("Some biometric is already running.", "Warning", MessageBoxButton.OK);
                this.Close();
                throw new Exception("Biometric is already in use");
            }

            try
            {
                this._options = options;
                // Auxiliary.Logger.Init();
                //log.Info("Const");
                InitializeComponent();
                arrowClicked = false;
                ShowState();
                PreInit();
                _stopFillingList = false;
                _errorWindowShown = false;
                // Dispatcher.BeginInvoke(new Action(() => Init()), DispatcherPriority.ContextIdle);
                //log.Info(".ctr finished");
                _biometricProcessionFinished = new BiosecLibClient.BiometricProcessedDelegate(ImageProcessedCbc);
                _updateProgress = new BiosecLibClient.UpdateDelegate(UpdateCallbackCbc);
                _fileOperationFinished = new BiosecLibClient.FinishDelegate(FileOperationFinished);
                _fileOperationError = new BiosecLibClient.ErrorDelegate(FileOperationError);

                //log.Info("Connecting to service");
                var binding = new NetNamedPipeBinding();
                binding.ReceiveTimeout = System.TimeSpan.FromHours(24);

                pipeFactory =
                   new ChannelFactory<IDialogClientService>(
                       binding,
                       new EndpointAddress("net.pipe://localhost/IdentaZone/IMLogger/DialogClient"));

                loggingService = pipeFactory.CreateChannel();
                if (!loggingService.Login())
                {
                    Auxiliary.Logger._log.Error("Service is busy");
                    return;
                }
                else
                {
                    //log.Info("Successfully logged in");
                    IsConnected = true;
                }

                Dispatcher.BeginInvoke(new Action(() => Init()), DispatcherPriority.ContextIdle);
                
                SystemEvents.SessionSwitch += new SessionSwitchEventHandler(this.SystemEvents_SessionSwitch);
            }
            catch (Exception ex)
            {
                m_Mutex.ReleaseMutex();
                throw ex;
            }

        }

        private void PreInit()
        {

            if (_options.IsSecure)
            {
                _libClient.initialize(BiosecLibClient.Mode.Encrypt, UserNotRegisteredCbc);
            }
            else
            {
                _libClient.initialize(BiosecLibClient.Mode.Decrypt, UserNotRegisteredCbc);
            }

            if (!_IsDisabled)
            {
                addSourcePaths();
            }
        }


        private void Init()
        {

            try
            {

                _helpLoader = new HelpLoader();

                if (!_IsDisabled)
                {
                    // Licenser
                    //log.Info("Starting licenser");
                    _licenser = new Licenser();
                    _licenser.Init(new Action(() => LicenseIsActivated()));

                    //log.Info("Starting enumerator worker");
                    RefreshBtn.IsEnabled = false;
                    _enumerator.DoWork += EnumeratorWork;
                    _enumerator.RunWorkerAsync(_pm);

                    //log.Info("Starting image reset timer");
                    // Image Reseter
                    imageReseter = new System.Windows.Threading.DispatcherTimer();
                    imageReseter.Tick += new EventHandler(imageClearTimerHandler);
                    imageReseter.Interval = new TimeSpan(0, 0, 0, 0, RESETER_TIMER_PERIOD_MS);
                    imageReseter.Start();
                    // Message Reseter
                    _messageReseter = new System.Windows.Threading.DispatcherTimer();
                    _messageReseter.Tick += new EventHandler(messageClearTimerHandler);
                    _messageReseter.Interval = new TimeSpan(0, 0, 0, 0, RESETER_MESSAGE_LIFETIME_MS);

                    //log.Info("Setup refresher");
                    // setup background worker for refresh
                    _refresher.DoWork += refreshOneDevice;
                    _imageFetcher.DoWork += fetchImage;
                    _liveImageFetcher.DoWork += fetchLiveImage;

                    var providerList = _libClient.getCryptoProviders();
                    foreach (var provider in providerList)
                    {
                        AddProvider(provider.ToString());
                    }




                    _progressBar = new ProgressBar();
                    _progressBar.SetBarsCount(2);

                    _errorWindow = new ErrorWindow();

                    if (_options.IsUnsecure)
                    {
                        _filename = Path.GetFileName(_options.InputFiles[0]);
                        _fileChooser = new FileChooser(this, _filename) { DecryptSelected_Notify = DecryptSelected, DecryptAll_Notify = DecryptAll };
                        //log.Info("File chooser creation in Init");
                    }

                }

            }
            catch (Exception ex)
            {
                Auxiliary.Logger._log.Error("error: " + ex.Message);
            }
            // AssociationCheck();

        }

        private class FillFileArgs
        {
            public FileChooser FileChooser;
            public BiosecLibClient LibClient;
        }

        private void fillFileList(FillFileArgs args)
        {
            UInt32 index = 0;

            var archiveSize = _libClient.getArchiveEntriesCount();
            //log.Info("archive size = " + archiveSize);

            try
            {
                Nullable<BiosecLibClient.ArchivePathInfo> newPath = args.LibClient.getArchivePathsAt(index);
                while (newPath != null && !_stopFillingList)
                {
                    var biofile = new BioFileInfo()
                    {
                        Filename = newPath.Value.pathName,
                        FileSize = newPath.Value.size,
                        FileNumber = newPath.Value.index,
                        Timestamp = newPath.Value.mTime
                    };
    
                    if (newPath.Value.type == BiosecLibClient.EntityAtPathType.Folder)
                    {
                        biofile.Type = IdentaZone.BioSecure.BioFileInfo.PathType.Folder;
                    }
                    else
                    {
                            biofile.Type = IdentaZone.BioSecure.BioFileInfo.PathType.File;
                    }
                    args.FileChooser.AddFile(biofile);
                    ++index;
                
                    newPath = args.LibClient.getArchivePathsAt(index);
                    
                    //Thread.Sleep(1);
                }
            }
            catch (Exception ex)
            {
                Auxiliary.Logger._log.Error("Error retrieving archive paths: " + ex.Message);
            }

            if (!_stopFillingList)
            {
                args.FileChooser.IsDecryptAllEnabled = true;
            }
        }

        private void UserNotRegisteredCbc()
        {
            Prompt.Text = "User is not registered on this PC";
            Prompt.Background = COLOR_BAD;
            this.Title = "IdentaMaster - error";
            DeleteAfterCB.Content = "Delete original file / folder after encryption";
            TestDevicePanel.IsEnabled = false;
            RefreshBtn.IsEnabled = false;
            _IsDisabled = true;
        }


        private void addSourcePaths()
        {
            if (_options.InputFiles != null)
            {
                BiosecLibClient.SourcePathInfo[] sourcepaths = new BiosecLibClient.SourcePathInfo[_options.InputFiles.Length];
                int pos = 0;
                var currentDirectory = Directory.GetCurrentDirectory();
                foreach (var path in _options.InputFiles)
                {
                    String pathToAppend;
                    if (!System.IO.Path.IsPathRooted(path))
                    {
                        pathToAppend = currentDirectory + "\\" + path;
                    }
                    else
                    {
                        pathToAppend = path;
                    }
                    sourcepaths[pos].pathName = Path.GetFullPath(pathToAppend);
                    pos++;
                }

                try
                {
                    var ret = _libClient.setSourcePathList(sourcepaths);
                    switch (ret)
                    {
                        case BiosecLibClient.Ret.NoError:
                            // do nothing
                            break;
                        case BiosecLibClient.Ret.UserNotAnArchiveOwner:
                            SetActiveWindow(IntPtr.Zero);
                            MessageBox.Show("Access to encrypted archive is denied.  File belongs to another user.", "Wrong user", MessageBoxButton.OK, MessageBoxImage.Warning);
                            _libClient.dispose();
                            this.Close();
                            //throw new Exception("Current user logged is not identified as archive owner");
                            break;
                        default:
                            Auxiliary.Logger._log.Error("Error while setting source paths: unexpected return code");
                            break;
                    }
                }
                catch (Exception ex)
                {
                    Auxiliary.Logger._log.Error("Error while setting source paths" + ex);
                    throw ex;                    
                }
            }
        }
        #endregion

        #region TimerEventHandlers

        private void messageClearTimerHandler(object sender, EventArgs e)
        {
            ShowState();
            _messageReseter.Stop();
        }

        /// <summary>
        /// Every IMAGE_RESETER_TIMER_PERIOD_MS check if we need to Clear some Image 
        /// </summary>
        /// <param name="Sender"></param>
        /// <param name="e"></param>
        private void imageClearTimerHandler(object sender, EventArgs e)
        {

            if (_nextReset > 0)
            {
                _nextReset -= RESETER_TIMER_PERIOD_MS;
                if (_nextReset <= 0)
                {
                    try
                    {
                        checkFinger.ClearImage();
                        checkFinger.SetText("");
                    }
                    catch (Exception ex)
                    {
                        Auxiliary.Logger._log.Error("Error while stopping the timer" + ex);
                    }
                }
            }
        }

        #endregion

        #region PM Enumeration functions

        private void EnumeratorWork(object sender, DoWorkEventArgs e)
        {
            _pm = (PluginManager)e.Argument;
            try
            {
                _pm.LoadPlugins();//Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location));
            }
            catch (Exception ex)
            {
                Auxiliary.Logger._log.Error(ex);
            }

            //log.InfoFormat("Found {0} deviceControls", _pm.DeviceControls.Count);
            // populate device ctrl tab
            foreach (var deviceCtrl in _pm.DeviceControls)
            {
                DoEvents();
                //log.InfoFormat("Adding devices from device control {0} to GUI", deviceCtrl);
                try
                {
                    deviceCtrl.EnumerateDevices();
                    Dispatcher.BeginInvoke(new Action(() => AddDeviceControl(deviceCtrl)));

                }
                catch (Exception ex)
                {
                    Auxiliary.Logger._log.Error(ex);
                }
            }
            Dispatcher.BeginInvoke(new Action(() => RefreshBtn.IsEnabled = true));
        }

        private void AddDeviceControl(IDeviceControl deviceCtrl)
        {
            Action EmptyDelegate = delegate(){ };
            if (deviceCtrl.ActiveDevices.Count == 0)
            {
                RadioButton radioButton = new RadioButton();
                radioButton.Checked += DeviceSelected;
                radioButton.Unchecked += DeviceDeselected;

                radioButton.Content = deviceCtrl.ToString();
                radioButton.IsChecked = false;
                radioButton.IsEnabled = true;

                radioButton.Style = this.FindResource("IZRadioButton") as Style;
                radioButton.ToolTip = deviceCtrl.ToString();

                TestDevicePanel.Children.Add(radioButton);
                radioButton.Dispatcher.Invoke(DispatcherPriority.Render, EmptyDelegate);
            }
            else
            {
                for (var dev = 0; dev < deviceCtrl.ActiveDevices.Count; dev++)
                {
                    RadioButton radioButton = new RadioButton();
                    radioButton.Checked += DeviceSelected;
                    radioButton.Unchecked += DeviceDeselected;

                    radioButton.Content = deviceCtrl.ActiveDevices[dev];
                    radioButton.IsChecked = false;
                    radioButton.Style = this.FindResource("IZRadioButton") as Style;
                    radioButton.ToolTip = deviceCtrl.ActiveDevices[dev].Description.Replace(", ", "\n");
                    TestDevicePanel.Children.Add(radioButton);
                    radioButton.Dispatcher.Invoke(DispatcherPriority.Render, EmptyDelegate);
                }
            }
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


        #endregion

        #region Licenser functions

        private void LicenseIsActivated()
        {
            //log.Info("License check is finished");
#if DEBUG
            log.Info("License is OK");
            Dispatcher.BeginInvoke(new Action(() => LicenseActivationFinished()));
#else
            if (_licenser.State != Licenser.STATE.ACTIVATED)
            {
                Auxiliary.Logger._log.Error("No license found");
                Dispatcher.BeginInvoke(new Action(() => LicenseActivationFinished()));
            }
            else
            {
                //log.Info("License is OK");
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

        #endregion

        #region DeviceBar state handlers

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
                Auxiliary.Logger._log.Error(ex);
            }
        }

        private void DeviceSelected(object sender, RoutedEventArgs e)
        {
            RadioButton rb = (sender as RadioButton);
            try
            {
                checkFinger.SetText("");
                arrowClicked = true;
                // If Provider has an active device
                if (rb.Content is IFingerDevice)
                {
                    Ambassador.SetCallback(DispatchMessage);
                    _currDevice = rb.Content as IFingerDevice;
                    _currDevice.Dispatch(COMMAND.SINGLECAPTURE_START);
                    checkFinger.IsDeviceConnected = true;
                }
                else
                {
                    checkFinger.IsDeviceConnected = false;
                }
                ShowState();
            }
            catch (Exception ex)
            {
                Auxiliary.Logger._log.Error(ex);
            }
        }

        #endregion

        #region PM functions

        private void DispatchMessage(Message msg)
        {
            if (msg is MBiometricsSingleCaptured)
            {
                BiometricsCaptured(msg.Sender, (msg as MBiometricsSingleCaptured).Image);
            }
            else if (msg is MBiometricsLiveCaptured)
            {
                BiometricsLiveCaptured(msg.Sender, (msg as MBiometricsLiveCaptured).Image);
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
                MessageSend(sender, "Connect device and press refresh");
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
                Auxiliary.Logger._log.Error(ex);
            }
        }

        #endregion

        #region Messages and Biometric processing

        private void MessageSend(Object sender, String text)
        {
            try
            {
                this.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal,
                    new Action(() => ProcessText(sender, text)));
                this.Dispatcher.Invoke(new Action(() => _nextReset = RESETER_IMAGE_LIFETIME_MS));
            }
            catch (Exception ex)
            {
                Auxiliary.Logger._log.Error(ex);
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
                Auxiliary.Logger._log.Error(ex);
            }
        }

        private void BiometricsLiveCaptured(Object sender, FingerImage fingerImage)
        {
            try
            {
                this.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal,
                    new Action(() => ProcessLiveImage(sender, fingerImage)));
            }
            catch (Exception ex)
            {
                Auxiliary.Logger._log.Error(ex);
            }
        }
        private void fetchImage(object sender, DoWorkEventArgs e)
        {
            var image = e.Argument as FingerImage;
            String type;
            byte[] data;
            image.Serialize(out type, out data);
            Dispatcher.InvokeAsync(new Action(() => SendBio(type, Convert.ToBase64String(data))));
            FingerPicture pic = image.MakePicture();
            Dispatcher.InvokeAsync(new Action(() => showImage(pic)));
        }

        void SendBio(String type, String data)
        {
            byte[] bytes = new byte[data.Length * sizeof(char)];
            System.Buffer.BlockCopy(data.ToCharArray(), 0, bytes, 0, bytes.Length);

            _libClient.putBiometric(type, bytes, _biometricProcessionFinished);
        }

        void ProcessText(Object sender, String text)
        {
            if (sender == _currDevice)
            {
                checkFinger.SetText(text);
            }
        }
        void ProcessImage(Object sender, FingerImage image)
        {
            try
            {

                // Do not process signals from another devices
                if (sender == _currDevice)
                {
                    if (image != null)
                    {
#if !DEBUG
                        if (_licenser.State == Licenser.STATE.ACTIVATED)
                        {
#endif

                            if (!_imageFetcher.IsBusy && !_liveImageFetcher.IsBusy)
                            {
                                (sender as IFingerDevice).Dispatch(COMMAND.SINGLECAPTURE_STOP);
                                Thread.Sleep(250); // delay to stop device in plugin
                                _imageFetcher.RunWorkerAsync(image);
                            }
#if !DEBUG
                        }
                        else
                        {
                            ShowMessage(true);
                        }
#endif
                    }
                }
            }
            catch (Exception ex)
            {
                Auxiliary.Logger._log.Error(ex);
            }
        }


        private void fetchLiveImage(object sender, DoWorkEventArgs e)
        {
            var image = e.Argument as FingerImage;
            FingerPicture pic = image.MakePicture();
            Dispatcher.InvokeAsync(new Action(() => showImage(pic)));
        }

        void ProcessLiveImage(Object sender, FingerImage image)
        {
            try
            {

                // Do not process signals from another devices
                if (sender == _currDevice)
                {
                    if (image != null)
                    {
#if !DEBUG
                        if (_licenser.State == Licenser.STATE.ACTIVATED)
                        {
#endif

                            if (!_imageFetcher.IsBusy && !_liveImageFetcher.IsBusy)
                            {
                                _liveImageFetcher.RunWorkerAsync(image);
                            }
#if !DEBUG
                        }
                        else
                        {
                            ShowMessage(true);
                        }
#endif
                    }
                }
            }
            catch (Exception ex)
            {
                Auxiliary.Logger._log.Error(ex);
            }
        }

        private void showImage(FingerPicture pic)
        {
            checkFinger.ShowImage(pic.Image, pic.Width, pic.Height);
            _nextReset = RESETER_IMAGE_LIFETIME_MS;
        }

        #endregion

        #region Biosecure library interraction (Image check)

        private void ImageProcessedCbc(BiosecLibClient.BiometricProcessingRes result)
        {
            Dispatcher.BeginInvoke(new Action(() => ImageProcessed(result)));
        }

        private void ImageProcessed(BiosecLibClient.BiometricProcessingRes res)
        {

            switch (res)
            {
                case BiosecLibClient.BiometricProcessingRes.BiometricAccepted:
                    _username = _libClient.getUserName().ToString();
                    // Show biometrics accepted
                    _currState = CollectorDialogState.GOOD_BIOMETRICS;
                    ShowMessage(false);
                    if (_options.IsUnsecure)
                    {
                        _result = ResultState.SHOW_FILECHOOSER;
                    }
                    if (_options.IsSecure)
                    {
                        _result = ResultState.SHOW_PB_ENC;
                    }
                    HideWindowAfter(2000);
                    return;
                case BiosecLibClient.BiometricProcessingRes.BiometricMatchNotFound:
                    _currState = CollectorDialogState.BAD_BIOMETRICS;
                    ShowMessage(false);
                    break;
                case BiosecLibClient.BiometricProcessingRes.BiometricNotBelongsToUser:
                    _username = _libClient.getUserName().ToString();
                    _currState = CollectorDialogState.NOT_IDENTIFIED_AS_CURRENT_USER;
                    ShowMessage(false);
                    break;
                case BiosecLibClient.BiometricProcessingRes.BiometricInvalidEncryptionKey:
                    _currState = CollectorDialogState.BAD_ENCRYTPION_KEY;
                    ShowMessage(false);
                    break;
                case BiosecLibClient.BiometricProcessingRes.BiometricInternalError:
                case BiosecLibClient.BiometricProcessingRes.BiometricUnknowEnrror:
                default:
                    Auxiliary.Logger._log.Error("Biosecure library thrown " + res.ToString());
                    break;
            }
            if (_currDevice != null)
            {
                _currDevice.Dispatch(COMMAND.SINGLECAPTURE_START);
            }
        }


        private void DecryptAll()
        {
            _result = ResultState.SHOW_PB_DEC_ALL;
            HideWindowAfter(0);
        }

        private void DecryptSelected()
        {
            _result = ResultState.SHOW_PB_DEC_SOME;
            HideWindowAfter(0);
        }

        #endregion

        private void ShowState()
        {
            Prompt.Foreground = new SolidColorBrush(Colors.White);
            if (_options.IsSecure)
            {
                if (arrowClicked == false)
                {
                    Prompt.Text = "Please, click on arrow to start";
                }
                else{
                    Prompt.Text = "Please, proceed with biometric authentication to start encryption process";
                }
                Prompt.Background = COLOR_OK;
                this.Title = "IdentaMaster - Secure";
                DeleteAfterCB.Content = "Delete original file / folder after encryption";
            }
            else if (_options.IsUnsecure)
            {
                if (arrowClicked == false)
                {
                    Prompt.Text = "Please, click on arrow to start";
                }
                else {
                    Prompt.Text = "Please, proceed with biometric authentication to start decryption process";
                }
                Prompt.Background = COLOR_OK;
                this.Title = "IdentaMaster - Unsecure";
                DeleteAfterCB.Content = "Overwrite original file / folder";
            }
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
                        Prompt.Text = "Identified as " + Truncate(_username, 20);
                        Prompt.Background = COLOR_OK;
                        break;
                    case CollectorDialogState.BAD_BIOMETRICS:
                        Prompt.Text = "Identification failure";
                        Prompt.Background = COLOR_BAD;
                        break;
                    case CollectorDialogState.BAD_BIOMETRICS_USER:
                        Prompt.Text = "Not identified as data owner";
                        Prompt.Background = COLOR_BAD;
                        break;
                    case CollectorDialogState.BAD_USER_LOGGED_IN:
                        Prompt.Text = "Wrong user logged in";
                        Prompt.Background = COLOR_OK;
                        break;
                    case CollectorDialogState.BAD_ENCRYTPION_KEY:
                        Prompt.Text = "Bad encryption key";
                        Prompt.Background = COLOR_BAD;
                        break;
                    case CollectorDialogState.NOT_IDENTIFIED_AS_CURRENT_USER:
                        Prompt.Text = String.Format("Not identified as current user ({0})", Truncate(_username, 20));
                        Prompt.Background = COLOR_BAD;
                        break;
                }
            }
            else
            {
                if (_licenser.State == BioControls.Auxiliary.Licenser.STATE.LOADING)
                {
                    Prompt.Text = "Please, wait while license is checked";
                    Prompt.Background = COLOR_OK;
                }
                else if (_licenser.State == BioControls.Auxiliary.Licenser.STATE.NOT_REGISTERED)
                {
                    Prompt.Text = "Activate IdentaMaster to enable files and folders encryption capabilities";
                    Prompt.Foreground = COLOR_BAD;
                }
            }
        }

        private void UpdateState(CollectorDialogState newState)
        {
            _currState = newState;
            //log.InfoFormat("Update State {0}", _currState);
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
                Auxiliary.Logger._log.Error(ex);
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
            while (TestDevicePanel.Children.OfType<RadioButton>().Count() > 0)
            {
                TestDevicePanel.Children.Remove(TestDevicePanel.Children.OfType<RadioButton>().ElementAt(0));
            }

            foreach (var deviceCtrl in _pm.DeviceControls)
            {
                
                // Run Refresh in separate worker thread
                _refresher.RunWorkerAsync(deviceCtrl);
                while (_refresher.IsBusy)
                {
                    DoEvents();
                }

//                Dispatcher.BeginInvoke(new Action(() => AddDeviceControl(deviceCtrl)));
                AddDeviceControl(deviceCtrl);
                //// If no device is online
                //if (deviceCtrl.ActiveDevices.Count == 0)
                //{
                //    foreach (var radioButton in currentRadioButtons)
                //    {
                //        radioButton.Content = deviceCtrl.ToString();
                //    }
                //}
                //// If some devices are online
                //else
                //{
                //    foreach (var radioButton in currentRadioButtons)
                //    {
                //        radioButton.Content = deviceCtrl.ActiveDevices[0];
                //        if (radioButton.IsChecked == true)
                //        {
                //            DeviceSelected(radioButton, null);
                //        }
                //    }
                //}

                // Unblock RB
                //foreach (var radioButton in currentRadioButtons)
                //{
                //    radioButton.IsEnabled = true;
                //}
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
                Auxiliary.Logger._log.Error(ex);
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
                Auxiliary.Logger._log.Error("ShowHelpButton click failed with " + ex);
            }

        }


        private void DeleteAfterCB_Checked(object sender, RoutedEventArgs e)
        {
            _libClient.setOverwriteFlag(true);
        }

        private void DeleteAfterCB_Unchecked(object sender, RoutedEventArgs e)
        {
            _libClient.setOverwriteFlag(false);
        }


        public void HideWindow()
        {
            this.Hide();
        }


        public void Destroy()
        {

            this.Close();
        }


        public void HideWindowAfter(int timeout)
        {
            _destroyer = new System.Windows.Threading.DispatcherTimer();
            _destroyer.Tick += new EventHandler(DestroyTimerEvent);
            _destroyer.Interval = new TimeSpan(0, 0, 0, 0, timeout);
            _destroyer.Start();
        }

        private void DestroyTimerEvent(object sender, EventArgs e)
        {
            _destroyer.Stop();
            switch (_result)
            {
                case ResultState.SHOW_PB_ENC:
                    this.Hide();
                    _progressBar.Show();
                    _libClient.startEncryptionProcess(
                        _updateProgress, _fileOperationFinished, _fileOperationError);

                    break;
                case ResultState.SHOW_FILECHOOSER:
                    _fileChooser.IdentifiedAsString = "Identified as " + Truncate(_username, 20);
                    var archiveEntriesCount = _libClient.getArchiveEntriesCount();
                    if (archiveEntriesCount > maxEntriesToShowFileChooserInstantly)
                    {
                        var fileChooserProgressBar = new ProgressBar();
                        fileChooserProgressBar.SetBarsCount(1);
                        fileChooserProgressBar.SetText(0, "Please wait while BioSecure read protected file");
                        this.Hide();
                        fileChooserProgressBar.Show();

                        uint index = 0;

                        List<BioFileInfo> filesList = new List<BioFileInfo>();
                        Nullable<BiosecLibClient.ArchivePathInfo> newPath = _libClient.getArchivePathsAt(index);
                        while (newPath != null)
                        {
                            
                            var biofile = new BioFileInfo()
                            {
                                Filename = newPath.Value.pathName,
                                FileSize = newPath.Value.size,
                                FileNumber = newPath.Value.index,
                                Timestamp = newPath.Value.mTime
                            };

                            if (newPath.Value.type == BiosecLibClient.EntityAtPathType.Folder)
                            {
                                biofile.Type = IdentaZone.BioSecure.BioFileInfo.PathType.Folder;
                            }
                            else
                            {
                                biofile.Type = IdentaZone.BioSecure.BioFileInfo.PathType.File;
                            }
                            ++index;

                            filesList.Add(biofile);
                            newPath = _libClient.getArchivePathsAt(index);
                            this.Dispatcher.Invoke(new Action(() =>
                            {
                                var doneInPercent = (int)(((index + 1) * 100) / archiveEntriesCount);
                                fileChooserProgressBar.SetProgress(0, doneInPercent);
                            }));
                            //Thread.Sleep(1);
                        }

                        
                        fileChooserProgressBar.Destroy();
                        _fileChooser.ShowFromList(filesList);
                    }
                    else
                    {
                        _fileChooser.Show();
                        _fileListFiller = new Thread(() => fillFileList(new FillFileArgs() { FileChooser = _fileChooser, LibClient = _libClient }));
                        _fileListFiller.Priority = ThreadPriority.BelowNormal;
                        _fileListFiller.Start();
                    }
                    
                    
                    break;

                case ResultState.SHOW_PB_DEC_SOME:
                    _libClient.setSelectionPaths(_fileChooser.GetSelectedNodes().ToArray());
                    _fileChooser.Close();
                    this.Hide();
                    _progressBar.Show();
                    _libClient.startDecryptionProcess(_updateProgress, _fileOperationFinished, _fileOperationError);
                    break;
                case ResultState.SHOW_PB_DEC_ALL:
                    _libClient.setSelectionPaths(_fileChooser.GetAllNodes().ToArray());
                    _fileChooser.Close();
                    this.Hide();
                    _progressBar.Show();
                    _libClient.startDecryptionProcess(_updateProgress, _fileOperationFinished, _fileOperationError);
                    break;

            }
        }

        private void FillFileList(object obj)
        {
            throw new NotImplementedException();
        }

        private byte FileOperationFinished(BiosecLibClient.CryptoOperationRes result, string fileName)
        {
            LogRecord record = new LogRecord();
            _fileOperationRes = result;
            if (_options.IsSecure)
            {
                record.operation = LogOperation.ENCRYPT;
            }
            else
            {
                record.operation = LogOperation.DECRYPT;
            }

            record.filename = fileName;
            record.username = _username;
            //log.Info("Logging operation filename: " + fileName + " user: " + _username);
            if (result == BiosecLibClient.CryptoOperationRes.BIOSEC_NO_ERROR)
            {
                try
                {
                    loggingService.Log(record);
                }
                catch (Exception ex)
                {
                    Auxiliary.Logger._log.Error(ex.Message);
                }
            }

            this.Dispatcher.Invoke(new Action(() => this.Destroy()));
            return 0;
        }


        private void FileOperationError(BiosecLibClient.ErrorType err, string file, string errorMessage)
        {
            
            if(!_errorWindowShown){
                this.Dispatcher.BeginInvoke(new Action(() =>
                {
                    _errorWindow.Show();
                    
                }));
                _errorWindowShown = true;
            }

            
            this.Dispatcher.BeginInvoke(new Action(() =>
            {
                try
                {
                    String errorType = "";
                    switch (err)
                    {
                        case BiosecLibClient.ErrorType.Critical:
                            errorType = "CRITICAL";
                            break;
                        case BiosecLibClient.ErrorType.Error:
                            errorType = "ERROR";
                            break;
                        default:
                            break;
                    }
                    _errorWindow.addErrorToLog(errorType, file, errorMessage);
                }
                catch (Exception ex)
                {
                    Auxiliary.Logger._log.Error("error adding to log: " + ex.Message);
                }
            }));
            
        }


        private void UpdateCallbackCbc(string total, string current, Int32 doneTotal, Int32 fromTotal, Int32 doneCurrent, Int32 fromCurrent)
        {
            if (0 == fromTotal)
            {
                //log.Error("UpdateCallbackCbc division by zero (fromTotal)");
                return;
            }

            if (0 == fromCurrent)
            {
                //log.Error("UpdateCallbackCbc division by zero (fromCurrent)");
                return;
            }
            /*
            if (0 == fromTotal)
            {
                fromTotal = doneTotal;
            }            

            if (0 == fromTotal)
            {
                fromCurrent = doneCurrent;
            }*/
            var doteTotalPercent = doneTotal * 100 / fromTotal;
            var doteCurrentPercent = doneCurrent * 100 / fromCurrent;
            this.Dispatcher.BeginInvoke(new Action(() =>
            {
                _progressBar.SetProgress(0, doteTotalPercent);
                _progressBar.SetProgress(1, doteCurrentPercent);
                _progressBar.SetText(0, total);
                _progressBar.SetText(1, current);
            }));
        }



        #region Aux functions

        private void AssociationCheck()
        {
            try
            {
                using (RegistryKey regkey = Registry.CurrentUser.OpenSubKey(@"Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\", true))
                {
                    var biosecKey = regkey.OpenSubKey(".izbiosecure", true);
                    if (biosecKey != null)
                    {
                        //log.Info("RegKey is " + biosecKey.Name);
                        foreach (String key in biosecKey.GetSubKeyNames())
                            biosecKey.DeleteSubKey(key);
                        biosecKey.Close();
                        regkey.DeleteSubKeyTree(@".izbiosecure");
                    }
                }
            }
            catch (Exception ex)
            {
                Auxiliary.Logger._log.Error("File association check failed ", ex);
            }
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
                Auxiliary.Logger._log.Error(ex);
            }
        }

        public String Truncate(String value, int maxLength)
        {
            if (String.IsNullOrEmpty(value)) return value;
            return value.Length <= maxLength ? value : value.Substring(0, maxLength);
        }

        #endregion

        private void Window_Closing_1(object sender, CancelEventArgs e)
        {
            loggingService.Logout();
            IsConnected = false;
            foreach (var devCtrl in _pm.DeviceControls)
            {
                devCtrl.Dispose();
            }
            Ambassador.Dispose();
            if (_progressBar != null)
            {
                if (_fileOperationRes == BiosecLibClient.CryptoOperationRes.BIOSEC_NO_ERROR)
                {
                    if (_options.IsSecure)
                    {
                        _progressBar.HideWindowAfter(3000, "Encryption complete");
                    }
                    else
                    {
                        _progressBar.HideWindowAfter(3000, "Decryption complete");
                    }
                }
                else
                {
                    _progressBar.Destroy();
                }
            }
            if (_errorWindow != null)
            {
                _errorWindow.Close();
            }
            _stopFillingList = true;            
            if (_fileListFiller != null)
            {
                _fileListFiller.Join();
            }
            if (_fileChooser != null)
            {
                _fileChooser.Destroy();
            }
            _libClient.dispose();
            
        }       
    }
}
