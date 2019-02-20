using IdentaZone.BioControls;
using IdentaZone.IMPlugin;
using log4net;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;
using System.Windows.Threading;
using UIControlsINDSS;

namespace IdentaZone.IdentaMaster.UserEdit
{

    /// <summary>
    /// Interaction logic for Biometrics.xaml
    /// </summary>
    public partial class Biometrics : UserControl
    {
        //
        private string _baseHint;
        private string baseHint
        {
            set
            {
                _baseHint = value;
                baseMessage = value;
                Info.Content = baseMessage;
            }
            get
            {
                return _baseHint;
            }
        }
        //
        private ControlTypeEnum _controlType = ControlTypeEnum.PALM_CONTROL_TYPE;
        private readonly ILog Log = LogManager.GetLogger(typeof(Biometrics));

        // how frequent is timer checked
        const int RESETER_TIMER_PERIOD_MS = 100;
        // How long will Image last on page
        const int RESETER_IMAGE_LIFETIME_MS = 2000;

        const int RESETER_TEXT_LIFETIME_MS = 2000;
        DispatcherTimer imageReseter = null;
        // Timer for fingerImageBoxClearing
        int nextReset = 0;
        int textReset = 0;
        public String baseMessage;

        Base Owner;

        IFingerDevice currentDevice;
        bool isOnline = false;

        FingerCredential currentCredential;
        

        List<IFingerDevice> devices;

        FingerChooser fingerChooser;
        FingerDisplay fingerDisplay = new FingerDisplay();

        public Action updateImage;

        int fingerNum;
        bool isBusy = false;
        /// <summary>
        /// palm specific 
        /// </summary>
        private bool isVerifyingPalm = false;
        List<FingerTemplate> enrollmentToVerify;
        private Dictionary<string, FingerCredential> startCredential = new Dictionary<string,FingerCredential>();
        private int highlightedPalm = -1;
        public Biometrics(Base owner)
        {
            Owner = owner;
            fingerChooser = new FingerChooser(new List<int>(), fingerClicked);

            InitializeComponent();
            palmEnrollControl.ClickFingerAction += fingerClicked;

            // Image Reseter
            imageReseter = new System.Windows.Threading.DispatcherTimer();
            imageReseter.Tick += new EventHandler(imageClearTimerHandler);
            imageReseter.Interval = new TimeSpan(0, 0, 0, 0, RESETER_TIMER_PERIOD_MS);
            imageReseter.Start();

            Embed.Children.Add(fingerChooser);
            if (_controlType == ControlTypeEnum.FINGERPRINT_CONTROL_TYPE)
            {
                palmEnrollControl.Visibility = Visibility.Collapsed;
            }
            else if (_controlType == ControlTypeEnum.PALM_CONTROL_TYPE)
            {
                fingerChooser.Visibility = Visibility.Collapsed;
            }
            Scanner.Children.Add(fingerDisplay);
            UpdateDeviceList();
        }

        private void UpdateDeviceList()
        {
            devices = new List<IFingerDevice>();
            foreach (var deviceCtrl in Owner.PluginManager.DeviceControls)
            {
                if (!Owner.appSet.Contains(deviceCtrl.ToString()))
                {
                    continue;
                }
                deviceCtrl.EnumerateDevices();
                devices.AddRange(deviceCtrl.ActiveDevices);
            }
            foreach (var device in devices)
            {
                ProviderBox.Items.Add(device);
            }
            if (devices.Count == 0)
            {
                Info.Content = "Biometric devices not detected";
                Info.FontSize = 22;
                Info.FontWeight = FontWeights.Bold;
                Info.Foreground = Brushes.Red;
                Info2.Visibility = System.Windows.Visibility.Visible;
                fingerDisplay = new FingerDisplay(Properties.Resources.no_device);
                Scanner.Children.Clear();
                Scanner.Children.Add(fingerDisplay);
                ProviderBox.IsEnabled = false;
                EnrollmentProgress.IsEnabled = false;
                Embed.IsEnabled = false;
            }
            else
            {
                switch (_controlType)
                {
                    case ControlTypeEnum.FINGERPRINT_CONTROL_TYPE:
                        baseHint = "Please, select finger to enroll or touch the scanner to verify it";//
                        showMessageBase(baseHint);
                        break;
                    case ControlTypeEnum.PALM_CONTROL_TYPE:
                        baseHint = "Please, select palm to enroll or place your hand above the scanner to verify it";//
                        showMessageBase(baseHint);
                        break;
                    default:
                        break;
                }
            }
        }

        private void showMessagePopup(String message)
        {
            Info.Content = message;
            textReset = RESETER_TEXT_LIFETIME_MS;
            Info.Visibility = Visibility.Visible;
        }

        /// <summary>
        /// Call this to start Image clear timer
        /// </summary>
        private void showMessageBase(String message)
        {
            if (textReset <= 0)
            {
                textReset = 1;
            }
            if (isBusy)
            {
                Info.Content = message;
            }
            Info.Visibility = Visibility.Visible;
        }
        private void forceShowMessageBase(String message)
        {
            Info.Content = message;
            Info.Visibility = Visibility.Visible;
        }

        private void showImage(FingerPicture pic)
        {
            fingerDisplay.ShowImage(pic.Image,pic.Width,pic.Height);
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
                        fingerDisplay.ClearImage();
                    }
                    catch (Exception ex)
                    {
                        //Log.Error("Error while stopping the timer" + ex);
                    }
                }
            }

            if (textReset > 0)
            {
                textReset -= RESETER_TIMER_PERIOD_MS;
                if (textReset <= 0)
                {
                    try
                    {
                        if (!isBusy)
                        {
                            Info.Content = baseMessage;
                        }
                        if (_controlType == ControlTypeEnum.FINGERPRINT_CONTROL_TYPE)
                        {
                            UpdateFingers();
                        }
                        else if (_controlType == ControlTypeEnum.PALM_CONTROL_TYPE)
                        {
                            palmEnrollControl.SetOriginalColor();
                            switch (highlightedPalm)
                            {
                                case 10:
                                    palmEnrollControl.LeftStatePalmP = StatePalm.WhiteBlack;
                                    highlightedPalm = -1;
                                    break;
                                case 11:
                                    palmEnrollControl.RightStatePalmP = StatePalm.WhiteBlack;
                                    highlightedPalm = -1;
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        //Log.Error("Error while stopping the timer" + ex);
                    }
                }
            }
        }


        public void UpdateFingers()
        {
            List<int> newFingers = new List<int>();
            List<int> startFingers = new List<int>();

            if (currentCredential != null)
            {
                newFingers.AddRange(currentCredential.getFingerList());
            }
            if (startCredential.Count() == 0)
            {
                foreach (var fingerCred in Owner.Credentials.OfType<FingerCredential>())
                {
                    startCredential.Add(fingerCred.device, new FingerCredential(fingerCred));
                }
            }
            if (startCredential.ContainsKey(currentDevice.ToString()))
            {
                startFingers.AddRange(startCredential[currentDevice.ToString()].getFingerList());
            }

            if (_controlType == ControlTypeEnum.FINGERPRINT_CONTROL_TYPE)
            {
                fingerChooser.UpdateFingers(newFingers);
            }
            else if (_controlType == ControlTypeEnum.PALM_CONTROL_TYPE)
            {
                if(newFingers.Contains(10)){
                    if (startFingers.Contains(10))
                    {
                        palmEnrollControl.LeftStatePalmP = StatePalm.WhiteBlack;
                    }
                    else
                    {
                        palmEnrollControl.LeftStatePalmP = StatePalm.Green;
                    }
                }
                else
                {
                    palmEnrollControl.LeftStatePalmP = StatePalm.White;
                }
                if (newFingers.Contains(11))
                {
                    if (startFingers.Contains(11))
                    {
                        palmEnrollControl.RightStatePalmP = StatePalm.WhiteBlack;
                    }
                    else
                    {
                        palmEnrollControl.RightStatePalmP = StatePalm.Green;
                    }
                }
                else
                {
                    palmEnrollControl.RightStatePalmP = StatePalm.White;
                }
            }
        }

        /// <summary>
        /// Call this function to end / abort enrollment process
        /// </summary>
        public void endEnrollment()
        {
            EnrollmentProgress.Value = 0;

            currentDevice.Dispatch(COMMAND.ENROLLMENT_STOP);
            Ambassador.ClearCallback();

            //currentDevice.StopEnrollment();
            UpdateFingers();
            isBusy = false;
            if (_controlType == ControlTypeEnum.FINGERPRINT_CONTROL_TYPE)
            {
                fingerDisplay.ClearImage();
                fingerChooser.DeSelectFinger(fingerNum);
            }
            else if (_controlType == ControlTypeEnum.PALM_CONTROL_TYPE)
            {
                palmEnrollControl.SetOriginalColor();

            }
            Cancel.Visibility = Visibility.Collapsed;
            UpdateBtn.Visibility = Visibility.Visible;
            ProviderBox.IsEnabled = true;
            EnrollmentProgress.IsEnabled = false;

            Ambassador.SetCallback(DispatchMessage);
            currentDevice.Dispatch(COMMAND.SINGLECAPTURE_START);
            //showMessageBase("Please, select finger to enroll or touch the scanner to verify it");
            switch (_controlType)
            {
                case ControlTypeEnum.FINGERPRINT_CONTROL_TYPE:
                    forceShowMessageBase(baseHint);
                    break;
                case ControlTypeEnum.PALM_CONTROL_TYPE:
                    forceShowMessageBase(baseHint);
                    break;
                default:
                    break;
            }
        }

        private void DispatchMessage(Message msg)
        {
            if (msg is MBiometricsSingleCaptured)
            {
                SingleCaptured(msg.Sender, (msg as MBiometricsSingleCaptured).Image);
            }
            if (msg is MBiometricsLiveCaptured)
            {
                LiveCaptured(msg.Sender, (msg as MBiometricsLiveCaptured).Image);
            }
            else if (msg is MShowText)
            {
                showMessage(msg.Sender, (msg as MShowText));
            }
            else if (msg is MSignal)
            {
                SignalCaptured(msg.Sender, (msg as MSignal).Signal);
            }
            if (msg is MUpdateProgress)
            {
                updateProgress((msg as MUpdateProgress).Value);
            }
            if (msg is MBiometricsEnrolled)
            {
                BiometricsEnrolled(msg.Sender, (msg as MBiometricsEnrolled).Biometrics);
            }
        }

        private void SignalCaptured(object sender, MSignal.SIGNAL signal)
        {
            try
            {
                switch (signal)
                {
                    case MSignal.SIGNAL.DEVICE_FAILURE:
                        this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Background,
                            new Action(() => DeviceFailure(sender)));
                        break;
                }
            }
            catch (Exception ex)
            {
                //Log.Info(ex);
            }
        }

        private void DeviceFailure(object sender)
        {
            if (isBusy)
            {
                endEnrollment();
            }

            fingerDisplay = new FingerDisplay(Properties.Resources.no_device);
            Scanner.Children.Clear();
            Scanner.Children.Add(fingerDisplay);
            showMessageBase("Biometric device failed, please reconnect device and restart enrollment");
            //currentDevice = null;
            isOnline = false;
        }

        public void showMessage(Object sender, MShowText msg)
        {
            this.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal,
                new textFetcher(ProcessTextMessage), sender, msg);
        }

        public void updateProgress(int value)
        {
            Dispatcher.Invoke(new Action(() => EnrollmentProgress.Value = value));
        }


        public void fingerClicked(int fingerNum)
        {
            if (isBusy)
            {
                showMessagePopup("Please, finish current enrollment");
                return;
            }
           

            if (currentCredential.getFingerList().Contains(fingerNum))
            {
                MessageBoxButton btnMessageBox = MessageBoxButton.YesNo;
                MessageBoxImage icnMessageBox = MessageBoxImage.Warning;
                MessageBoxResult rsltMessageBox = MessageBox.Show("Do you want to remove enrollment?", "Remove enrollment", btnMessageBox, icnMessageBox);

                switch (rsltMessageBox)
                {
                    case MessageBoxResult.Yes:
                        if (startCredential.ContainsKey(currentDevice.ToString()))
                        {
                            if (startCredential[currentDevice.ToString()].getFingerList().Contains(fingerNum))
                            {
                                startCredential[currentDevice.ToString()].RemoveFinger(fingerNum);
                            }
                        }
                        currentCredential.RemoveFinger(fingerNum);
                        UpdateFingers();
                        break;
                    case MessageBoxResult.No:
                        break;
                }
            }
            else
            {
                if (currentDevice == null)
                {
                    return;
                }
                if (!isOnline)
                {
                    return;
                }
                this.fingerNum = fingerNum;
                isBusy = true;
                UpdateFingers();
                if (_controlType == ControlTypeEnum.FINGERPRINT_CONTROL_TYPE)
                {
                    fingerChooser.SelectFinger(fingerNum);
                }
                else if (_controlType == ControlTypeEnum.PALM_CONTROL_TYPE)
                {
                    if (fingerNum == 10)
                    {
                        palmEnrollControl.LeftStatePalmP = StatePalm.Orange;
                    }
                    else if (fingerNum == 11)
                    {
                        palmEnrollControl.RightStatePalmP = StatePalm.Orange;
                    }
                }
                Cancel.Visibility = Visibility.Visible;
                UpdateBtn.Visibility = Visibility.Collapsed;
                ProviderBox.IsEnabled = false;
                EnrollmentProgress.IsEnabled = true;

                this.UpdateLayout();
                currentDevice.Dispatch(COMMAND.SINGLECAPTURE_STOP);
                Ambassador.SetCallback(DispatchMessage);
                currentDevice.Dispatch(COMMAND.ENROLLMENT_START); //BiometricsCaptured, SendMessage, BiometricsEnrolled
            }
        }

        private void BiometricsEnrolled(Object sender, List<FingerTemplate> bios)
        {
            try
            {
                if (_controlType == ControlTypeEnum.PALM_CONTROL_TYPE)
                {
                    if (isVerifyingPalm == false)
                    {
                        enrollmentToVerify = bios;
                        isVerifyingPalm = true;
                        currentDevice.Dispatch(COMMAND.SINGLECAPTURE_START);
                        return;
                    }
                }
                if (bios != null)
                {
                    // Check whether it is already enrolled

                    IDeviceControl currDeviceControl = null;
                    List<FingerTemplate> matchedTemplates;
                    foreach (var devCtrl in Owner.PluginManager.DeviceControls)
                    {
                        if (devCtrl.ActiveDevices.Contains(currentDevice))
                        {
                            currDeviceControl = devCtrl;
                        }
                    }

                    if (isVerifyingPalm)
                    {
                        int matchesCount = currDeviceControl.Match(bios[0], enrollmentToVerify, out matchedTemplates);
                        isVerifyingPalm = false;
                    }
                    foreach (var user in Owner.Users)
                    {
                        if (user.SID == Owner.SID)
                        {
                            continue;
                        }
                        foreach (var fingerCred in user.credentials_List.OfType<FingerCredential>())
                        {
                            if (fingerCred.device.Equals(currentDevice.ToString()))
                            {
                                var candidates = new ComparsionCandidates(fingerCred.fingers);
                                foreach (var bio in bios)
                                {
//                                    int fingerIndex = currentDevice.Match(bio, candidates.Candidates);
                                    int matchesCount = currDeviceControl.Match(bio, candidates.Candidates, out matchedTemplates);
                                    if (matchesCount != 0)
                                    {
                                        Dispatcher.Invoke(new Action(() => showMessagePopup("Already registered as "
                                       + FingerCredential.GetFingerName(candidates.PositionOf(matchedTemplates[0])) +
                                       (_controlType == ControlTypeEnum.PALM_CONTROL_TYPE ? " " : " finger") +
                                       " of user " +
                                       user.ToString())));
                                        goto end;
                                    }
                                }
                            }
                        }
                    }
                    ////check if it is first time enrollment
                    if (currentCredential != null)
                    {
                        var candidates = new ComparsionCandidates(currentCredential.fingers);
                        foreach (var bio in bios)
                        {
                            //int fingerIndex = currentDevice.Match(bio, candidates.Candidates);
                            //if (fingerIndex != -1)
                            int matchesCount = currDeviceControl.Match(bio, candidates.Candidates, out matchedTemplates);
                            if (matchesCount != 0)
                            {
                                Dispatcher.Invoke(new Action(() => showMessagePopup("Already registered as "
                               + FingerCredential.GetFingerName(candidates.PositionOf(matchedTemplates[0])) +
                                        (_controlType == ControlTypeEnum.PALM_CONTROL_TYPE ? " " : " finger") +
                                       " of user " +
                               Owner.Username)));
                                goto end;
                            }
                        }
                    }
                    ////
                    if (_controlType == ControlTypeEnum.FINGERPRINT_CONTROL_TYPE)
                    {
                        foreach (var tmpl in bios)
                        {
                            String str = Convert.ToBase64String(tmpl.Serialize());
                            currentCredential.AddFinger(new Finger(fingerNum, str, tmpl));
                        }
                    }
                    else if(_controlType == ControlTypeEnum.PALM_CONTROL_TYPE)
                    {
                        showMessagePopup((fingerNum == 10 ? "Left palm" : "Right palm") + " has been successfully registered");
                        foreach (var tmpl in enrollmentToVerify)
                        {
                            String str = Convert.ToBase64String(tmpl.Serialize());
                            currentCredential.AddFinger(new Finger(fingerNum, str, tmpl));
                        }
                    }
                }

            end:
                Task.Factory.StartNew(() =>
                    {
                        Dispatcher.Invoke(new Action(() => endEnrollment()));
                    });
            }
            catch (Exception ex)
            {
                Log.Error("enrollment exception" + ex);
            }
        }


        private delegate void biometricsFetcher(Object sender, FingerImage image);
        private void ProcessImage(Object sender, FingerImage image)
        {
            try
            {
                if (sender == currentDevice)
                {
                    FingerPicture pic = image.MakePicture();
                    Dispatcher.Invoke(new Action(() => showImage(pic)));
                }
            }
            catch (Exception e)
            {
                //Log.Info(e);
            }
        }
        private delegate void textFetcher(Object sender, MShowText text);
        private void ProcessTextMessage(Object sender, MShowText msg)
        {
            if (sender == currentDevice)
            {
                switch (msg.Type)
                {
                    case MShowText.TYPE.BASE:
                        showMessageBase(msg.Message);
                        break;
                    case MShowText.TYPE.POPUP:
                        if (isBusy)
                        {
                            showMessagePopup(msg.Message);
                        }
                        break;
                }
            }
        }
        private void LiveCaptured(Object sender, FingerImage image)
        {
            try
            {
                this.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal,
                    new biometricsFetcher(ProcessImage), sender, image);
            }
            catch (Exception e)
            {
                //Log.Info(e);
            }
        }

        private class ComparsionCandidates
        {
            public FingerTemplate[] Candidates {get; set;}
            public int[] Positions {get; set;}
            public ComparsionCandidates(List<Finger> fingers)
            {
                Candidates = new FingerTemplate[fingers.Count];
                Positions = new int[fingers.Count];
                for (int i = 0; i < fingers.Count; i++)
                {
                    Candidates[i] = fingers[i].Template;
                    Positions[i] = fingers[i].FingerNum;
                }
            }
            public int PositionOf(FingerTemplate template){
                return Positions[Array.IndexOf(this.Candidates, template)];
            }
        }

        private void ProcessSingle(Object sender, FingerImage image)
        {
            try
            {
                if (isBusy && !isVerifyingPalm)
                {
                    return;
                }
                if (sender == currentDevice)
                {
                    FingerPicture pic = image.MakePicture();
                    showImage(pic);
                    if (isVerifyingPalm)
                    {
                        var verifyList = new List<FingerTemplate>();
                        verifyList.Add(currentDevice.Extract(image));
                        BiometricsEnrolled(this, verifyList);
                        return;
                    }

                    var candidates = new ComparsionCandidates(currentCredential.fingers);
                   
                    /////////////
                    IDeviceControl currDeviceControl;
                    List<FingerTemplate> matchedTemplates;
                    foreach (var devCtrl in Owner.PluginManager.DeviceControls)
                    {
                        if (devCtrl.ActiveDevices.Contains(currentDevice))
                        {
                            currDeviceControl = devCtrl;
                            int fingerIndex = currDeviceControl.Match(currentDevice.Extract(image), candidates.Candidates.ToArray(), out matchedTemplates);

                            if (fingerIndex == 0)
                            {
                                if (_controlType == ControlTypeEnum.FINGERPRINT_CONTROL_TYPE)
                                {
                                    fingerDisplay.ShowPopUp("Finger is not registered");
                                }
                                else if (_controlType == ControlTypeEnum.PALM_CONTROL_TYPE)
                                {
                                    fingerDisplay.ShowPopUp("Palm is not registered");
                                }
                                UpdateFingers();
                            }
                            else
                            {
                                if (_controlType == ControlTypeEnum.FINGERPRINT_CONTROL_TYPE)
                                {
                                    fingerDisplay.ShowPopUp("Finger is registered as " + FingerCredential.GetFingerName(candidates.PositionOf(matchedTemplates[0])));
                                    fingerChooser.ShowFinger(candidates.PositionOf(matchedTemplates[0]));
                                }
                                else if (_controlType == ControlTypeEnum.PALM_CONTROL_TYPE)
                                {
                                    fingerDisplay.ShowPopUp("Palm is registered as " + FingerCredential.GetFingerName(candidates.PositionOf(matchedTemplates[0])));
                                    if (candidates.PositionOf(matchedTemplates[0]) == 10)
                                    {
                                        if (highlightedPalm == 11)
                                        {
                                            palmEnrollControl.RightStatePalmP = StatePalm.WhiteBlack;
                                        }
                                        if (palmEnrollControl.LeftStatePalmP != StatePalm.Green)
                                        {
                                            palmEnrollControl.LeftStatePalmP = StatePalm.GreenBlack;
                                            highlightedPalm = 10;
                                        }
                                    }
                                    else if (candidates.PositionOf(matchedTemplates[0]) == 11)
                                    {
                                        if (highlightedPalm == 10)
                                        {
                                            palmEnrollControl.LeftStatePalmP = StatePalm.WhiteBlack;
                                        }
                                        if (palmEnrollControl.RightStatePalmP != StatePalm.Green)
                                        {
                                            palmEnrollControl.RightStatePalmP = StatePalm.GreenBlack;
                                            highlightedPalm = 11;
                                        }
                                    }

                                }
                                // Reset fingers after all
                                textReset = RESETER_TEXT_LIFETIME_MS;
                            }
                        }
                    }
                    /////////////
                    currentDevice.Dispatch(COMMAND.SINGLECAPTURE_START);
                }
            }
            catch (Exception ex)
            {
                //Log.Info(ex);
            }
        }

        private void SingleCaptured(Object sender, FingerImage image)
        {
            try
            {
                this.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal,
                    new biometricsFetcher(ProcessSingle), sender, image);
            }
            catch (Exception e)
            {
                //Log.Info(e);
            }
        }

        public void updateImageAction()
        {
            updateImage();
        }

        private void UpdateCredential()
        {
            var prevCredential = Owner.Credentials.OfType<FingerCredential>().FirstOrDefault(fc =>
                fc.device.Equals(currentDevice.ToString())
            );
            if (prevCredential != null)
            {
                Owner.Credentials.Remove(prevCredential);
            }

            Owner.Credentials.Add(currentCredential);
        }

        private void UpdateEnrollment(object sender, RoutedEventArgs e)
        {
               //var prevCredential = Owner.Credentials.OfType<FingerCredential>().FirstOrDefault(fc => fc.device == currentDevice.ToString());
            if (currentDevice != null)
            {
                UpdateCredential();
            }
            Owner.BiometricsUpdate();
        }

        private void ProviderBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (currentCredential != null)
            {
                UpdateCredential();
            }
            currentCredential = null;
            EnrollmentProgress.Value = 0;
            IFingerDevice device = (sender as ComboBox).SelectedItem as IFingerDevice;
            //if (currentDevice != null)
            if (isOnline)
                {
                Ambassador.ClearCallback();
                currentDevice.Dispatch(COMMAND.SINGLECAPTURE_STOP);
            }

            currentDevice = device;
            isOnline = true;
            if (device.BSPCode == 24)
            {
                _controlType = ControlTypeEnum.PALM_CONTROL_TYPE;
                fingerChooser.Visibility = Visibility.Collapsed;
                palmEnrollControl.Visibility = Visibility.Visible;
            }
            else
            {
                _controlType = ControlTypeEnum.FINGERPRINT_CONTROL_TYPE;
                fingerChooser.Visibility = Visibility.Visible;
                palmEnrollControl.Visibility = Visibility.Collapsed;
            }

            Ambassador.SetCallback(DispatchMessage);
            currentDevice.Dispatch(COMMAND.SINGLECAPTURE_START);


            
            foreach (var iface in currentDevice.GetType().GetInterfaces())
            {
                foreach (var fingerCred in Owner.Credentials.OfType<FingerCredential>())
                {
                    if (fingerCred.device.Equals(currentDevice.ToString()))
                    {
                        currentCredential = new FingerCredential(fingerCred);
                    }
                }
            }

            if (currentCredential == null)
            {
                currentCredential = new FingerCredential();
                currentCredential.device = currentDevice.ToString();
                currentCredential.deviceName = currentDevice.ToString();
                //Owner.Credentials.Add(currentCredential);
            }

            fingerDisplay = new FingerDisplay();
            Scanner.Children.Clear();
            Scanner.Children.Add(fingerDisplay); 
            fingerDisplay.ClearImage();
            Cancel.Visibility = Visibility.Collapsed;
            UpdateBtn.Visibility = Visibility.Visible;
            ProviderBox.IsEnabled = true;
            EnrollmentProgress.IsEnabled = false;

            UpdateFingers();
//            showMessageBase("Please, select finger to enroll or touch the scanner to verify it");
            switch (_controlType)
            {
                case ControlTypeEnum.FINGERPRINT_CONTROL_TYPE:
                    baseHint = "Please, select finger to enroll or touch the scanner to verify it";//
                    showMessageBase(baseHint);
                    break;
                case ControlTypeEnum.PALM_CONTROL_TYPE:
                    baseHint = "Please, select palm to enroll or place your hand above the scanner to verify it";//
                    showMessageBase(baseHint);
                    break;
                default:
                    break;
            }
        }

        private void CancelEnrollment(object sender, RoutedEventArgs e)
        {
            endEnrollment();
        }

        private void UserControl_Unloaded(object sender, RoutedEventArgs e)
        {
            if (currentDevice != null)
            {
                currentDevice.Dispatch(COMMAND.ENROLLMENT_STOP);
            }
            Ambassador.ClearCallback();
        }

    }
}
