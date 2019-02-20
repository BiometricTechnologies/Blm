using IdentaZone.BioControls;
using IdentaZone.IMPlugin;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Security.Permissions;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Navigation;
using System.Windows.Threading;

namespace IdentaZone.IdentaMaster
{

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>

    public partial class MainWindow : Window
    {
        /// Some info about usage of Invoke, InvokeLater and BackgroundWorker
        /// BW is used to most complicated job - refresh. It could freeze, so better do it in separate thread
        /// InvokeLater must be used where invokation needs to use Ambassador class, or you get deadlock
        /// Invoke is used for simple GUI updating.


        // Delegates to be used in placking jobs onto the Dispatcher.
        private delegate void refreshFetcher();
        private delegate void biometricsFetcher(Object sender, FingerImage image);
        private delegate void textFetcher(Object sender, String text);

        private BackgroundWorker imageFetcher = new BackgroundWorker();
        private BackgroundWorker refresher = new BackgroundWorker();

        // Timer for fingerImageBoxClearing
        int imageReset;
        // Timer for reseting textBox
        int textReset;
        // how frequent is timer checked
        const int RESETER_TIMER_PERIOD_MS = 100;
        // How long will Image last on page
        const int RESETER_IMAGE_LIFETIME_MS = 2000;
        const int RESETER_TEXT_LIFETIME_MS = 2000;
        DispatcherTimer imageReseter = null;

        FingerDisplay checkFinger = null;
        RadioButton currentTestDevice = null;

        const String unactiveText = "Please, register and activate the program";
        const String loadingText = "Please, wait while license is validated";
        const String baseText = "You have installed and activated ";

        private void UpdateTestTabLicenseState()
        {
            switch (Licenser.State)
            {
                
            }
        }
        private void UpdateTestTab()
        {
            BuildTestUserList();
            if (UpdateAfterLicense)
            {
                RefreshTestPage();
                UpdateAfterLicense = false;
            }
            foreach (var radiobutton in TestDevicePanel.Children.OfType<RadioButton>())
            {
                if (radiobutton.IsChecked == true)
                {
                    TestPageDeviceSelected(radiobutton, null);
                }
            }
        }

        /// <summary>
        /// Initializes the test tab.
        /// </summary>
        private void InitTestTab()
        {
            // Image Reseter
            imageReseter = new System.Windows.Threading.DispatcherTimer();
            imageReseter.Tick += new EventHandler(imageClearTimerHandler);
            imageReseter.Interval = new TimeSpan(0, 0, 0, 0, RESETER_TIMER_PERIOD_MS);
            imageReseter.Start();


            // populate device ctrl tab
            if (UpdateAfterLicense)
            {
                foreach (var deviceCtrl in pluginManager.DeviceControls)
                {
                    deviceCtrl.EnumerateDevices();
                    if (deviceCtrl.ActiveDevices.Count == 0)
                    {
                        RadioButton radioButton = new RadioButton();
                        radioButton.Checked += TestPageDeviceSelected;
                        radioButton.Unchecked += TestPageDeviceDeselected;

                        StackPanel panel = new StackPanel();

                        radioButton.Content = deviceCtrl.ToString();
                        radioButton.IsChecked = false;
                        radioButton.IsEnabled = true;
                        radioButton.Style = Resources["IZRadioButton"] as Style;
                        radioButton.ToolTip = deviceCtrl.ToString();

                        TestDevicePanel.Children.Add(radioButton);
                    }
                    else
                    {
                        for (var dev = 0; dev < deviceCtrl.ActiveDevices.Count; dev++)
                        {

                            RadioButton radioButton = new RadioButton();
                            radioButton.Checked += TestPageDeviceSelected;
                            radioButton.Unchecked += TestPageDeviceDeselected;

                            radioButton.Content = deviceCtrl.ActiveDevices[dev];
                            radioButton.IsChecked = false;
                            radioButton.Style = Resources["IZRadioButton"] as Style;
                            radioButton.ToolTip = deviceCtrl.ActiveDevices[dev].Description.Replace(", ", "\n");
                            TestDevicePanel.Children.Add(radioButton);
                        }
                    }
                }
                UpdateAfterLicense = false;
            }

            // setup background worker for refresh
            refresher.DoWork += refreshOneDevice;
            imageFetcher.DoWork += fetchOneImage;
        }


        /// <summary>
        /// Handles the RequestNavigate event of the Hyperlink control.
        /// </summary>
        /// <param name="Sender">The source of the event.</param>
        /// <param name="e">The <see cref="RequestNavigateEventArgs"/> instance containing the event data.</param>
        private void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
            Process.Start(new ProcessStartInfo(e.Uri.AbsoluteUri));
            e.Handled = true;
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
                //Log.Info(ex);
            }
        }

        public object ExitFrame(object f)
        {
            ((DispatcherFrame)f).Continue = false;
            return null;
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

        /// <summary>
        /// Refreshes the test page.
        /// </summary>
        private void RefreshTestPage()
        {
            //var allRadioButtons = TestDevicePanel.Children.OfType<RadioButton>();
            while (TestDevicePanel.Children.OfType<RadioButton>().Count() > 0)
            {
                TestDevicePanel.Children.Remove(TestDevicePanel.Children.OfType<RadioButton>().ElementAt(0));
            }
            Action EmptyDelegate = delegate() { };
       //     TestDevicePanel.Dispatcher.Invoke(DispatcherPriority.Render, EmptyDelegate);
            // populate device ctrl tab
            foreach (var deviceCtrl in pluginManager.DeviceControls)
            {
                if (!appSet.Contains(deviceCtrl.ToString()))
                {
                    continue;
                }
                deviceCtrl.EnumerateDevices();
                if (deviceCtrl.ActiveDevices.Count == 0)
                {
                    RadioButton radioButton = new RadioButton();
                    radioButton.Checked += TestPageDeviceSelected;
                    radioButton.Unchecked += TestPageDeviceDeselected;

                    StackPanel panel = new StackPanel();

                    radioButton.Content = deviceCtrl.ToString();
                    radioButton.IsChecked = false;
                    radioButton.IsEnabled = true;
                    radioButton.Style = Resources["IZRadioButton"] as Style;
                    radioButton.ToolTip = deviceCtrl.ToString();

                    TestDevicePanel.Children.Add(radioButton);
                    radioButton.Dispatcher.Invoke(DispatcherPriority.Render, EmptyDelegate);
                }
                else
                {
                    for (var dev = 0; dev < deviceCtrl.ActiveDevices.Count; dev++)
                    {

                        RadioButton radioButton = new RadioButton();
                        radioButton.Checked += TestPageDeviceSelected;
                        radioButton.Unchecked += TestPageDeviceDeselected;

                        radioButton.Content = deviceCtrl.ActiveDevices[dev];
                        radioButton.IsChecked = false;
                        radioButton.Style = Resources["IZRadioButton"] as Style;
                        radioButton.ToolTip = deviceCtrl.ActiveDevices[dev].Description.Replace(", ", "\n");
                        TestDevicePanel.Children.Add(radioButton);
                        radioButton.Dispatcher.Invoke(DispatcherPriority.Render, EmptyDelegate);
                    }
                }
            }
            TestRefreshButton.IsEnabled = true;
        }

        void TestPageDeviceSelected(object sender, RoutedEventArgs e)
        {
            RadioButton radioButton = (sender as RadioButton);
            try
            {
                // If Provider has an active device
                if (radioButton.Content is IFingerDevice)
                {
                    currentTestDevice = (sender as RadioButton);
                    currentTestDevice.IsEnabled = false;
                    IFingerDevice device = (sender as RadioButton).Content as IFingerDevice;

                    TestScannerPage.Children.Clear();
                    checkFinger = new FingerDisplay();
                    TestScannerPage.Children.Add(checkFinger);

                    //checkFinger.SetText("Please, touch the scanner");
                    Ambassador.SetCallback(DispatchMessage);
                    device.Dispatch(COMMAND.LIVECAPTURE_START);
                    imageReseter.Start();
                }
                else
                {
                    Ambassador.ClearCallback();
                    if (checkFinger != null)
                    {
                        TestScannerPage.Children.Clear();
                        checkFinger = null;
                    }
                    checkFinger = new FingerDisplay(Properties.Resources.no_device);
                    TestScannerPage.Children.Add(checkFinger);
                    checkFinger.SetText("Connect device and press refresh");
                }
            }
            catch (Exception ex)
            {
                Log.Error(ex);
            }
        }

        private void DispatchMessage(Message msg)
        {
            if (msg is MBiometricsLiveCaptured)
            {
                BiometricsCaptured(msg.Sender, (msg as MBiometricsLiveCaptured).Image);
            }
            else if (msg is MBiometricsSingleCaptured)
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

        private void TestPageDeviceDeselected(object sender, RoutedEventArgs e)
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
                //Log.Warn(ex);
            }
        }

        struct WorkerJob
        {
            public IFingerDevice Sender;
            public FingerImage Image;
        }

        private void fetchOneImage(object sender, DoWorkEventArgs e)
        {
            var job = (WorkerJob)e.Argument;
            try
            {
                
                FingerPicture pic = null;
                FingerTemplate extracted = null;
                pic = job.Image.MakePicture();
                Dispatcher.Invoke(new Action(() => checkFinger.ShowImage(pic.Image, pic.Width, pic.Height)));
                Dispatcher.Invoke(new Action(() => imageClearTimerStart()));
                extracted = job.Sender.Extract(job.Image);

                if (extracted != null)
                {
                    job.Sender.Dispatch(COMMAND.LIVECAPTURE_STOP);
                    if (templates.Keys.Contains(job.Sender.ToString()))
                    {
                        Dictionary<FingerTemplate, String> currentTemplates = templates[job.Sender.ToString()];
                        if (currentTemplates != null)
                        {
                            var candidates = currentTemplates.Keys.ToArray();
                            /////////////
                            List<FingerTemplate> matchedTemplates;
                            foreach (var devCtrl in pluginManager.DeviceControls)
                            {
                                if (devCtrl.ActiveDevices.Contains(job.Sender))
                                {
                                    int result = devCtrl.Match(extracted, candidates, out matchedTemplates);
                                    if (result != 0)
                                    {
                                        this.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal, (new Action(() =>
                                        {
                                            if (job.Sender == currentTestDevice.Content) checkFinger.ShowPopUp("Identified as " + currentTemplates[matchedTemplates[0]]);
                                        }
                                            )));
                                    }
                                }
                            }

                            /////////////
                        }
                    }
                    job.Sender.Dispatch(COMMAND.LIVECAPTURE_START);
                }
                else
                {
                    //Log.Error("Extracted = " + extracted.ToString());
                }
            }
            catch (Exception ex)
            {
                Log.Error(ex);
            }
        }
        void ProcessText(Object sender, String text)
        {
            try
            {
                if (currentTestDevice.Content == sender)
                {
                    this.Dispatcher.Invoke(new Action(() => checkFinger.SetText(text)));
                }
            }
            catch (Exception ex)
            {

            }
        }
        void ProcessImage(Object sender, FingerImage image)
        {
            try
            {
                // Do not process signals from another devices
                if (sender == currentTestDevice.Content)
                {
                    currentTestDevice.IsEnabled = true;

                    if (image != null)
                    {
                        if (!imageFetcher.IsBusy)
                        {
                            imageFetcher.RunWorkerAsync(new WorkerJob() { Image = image, Sender = sender as IFingerDevice });
                        }
                    }
                }
            }
            catch (Exception e)
            {
                //Log.Info(e);
            }
        }

        public void BiometricsCaptured(Object sender, FingerImage fingerImage)
        {
            try
            {
                this.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal,
                new biometricsFetcher(ProcessImage), sender, fingerImage);
            }
            catch (Exception ex)
            {
                //Log.Info(ex);
            }
        }

        public void MessageSend(Object sender, String text)
        {
            try
            {
                this.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal,
                new textFetcher(ProcessText), sender, text);
            }
            catch (Exception ex)
            {
                //Log.Info(ex);
            }
        }


        public void DeviceFailure(Object sender)
        {
            // only for our device
            foreach (var radioButton in TestDevicePanel.Children.OfType<RadioButton>().Where(rb => rb.Content == sender))
            {
                radioButton.Content = sender.ToString();

                checkFinger.SetText("");
                textClearTimerStop();
                Ambassador.ClearCallback();
                if (checkFinger != null)
                {
                    TestScannerPage.Children.Clear();
                    checkFinger = null;
                }
                checkFinger = new FingerDisplay(Properties.Resources.no_device);
                TestScannerPage.Children.Add(checkFinger);
                MessageSend(sender, "Connect device and press refresh");

                radioButton.IsEnabled = true;

            }
        }

        public void SignalCaptured(Object sender, MSignal.SIGNAL sig)
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
                //Log.Info(ex);
            }
        }

        Dictionary<String, Dictionary<FingerTemplate, String>> templates = new Dictionary<String, Dictionary<FingerTemplate, String>>();
        private bool UpdateAfterLicense = false;
        void BuildTestUserList()
        {
            try
            {
                List<WinUser> userList = _db.GetAllUsers();

                templates.Clear();
                foreach (var user in userList)
                {
                    foreach (var cred in user.credentials_List.OfType<FingerCredential>())
                    {
                        Dictionary<FingerTemplate, String> deviceTemplates;
                        if (templates.ContainsKey(cred.device))
                        {
                            deviceTemplates = templates[cred.device];
                        }
                        else
                        {
                            deviceTemplates = new Dictionary<FingerTemplate, String>();
                            templates.Add(cred.device, deviceTemplates);
                        }
                        foreach (var finger in cred.fingers)
                        {
                            if (finger.Template != null)
                            {
                                deviceTemplates.Add(finger.Template, user.ToString());
                            }
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Log.Error(e);
            }
        }




        private void textClearTimerStart()
        {
            textReset = RESETER_TEXT_LIFETIME_MS;
        }


        /// <summary>
        /// Call this to start Image clear timer
        /// </summary>
        private void imageClearTimerStart()
        {
            imageReset = RESETER_IMAGE_LIFETIME_MS;
        }

        /// <summary>
        /// Call this to abort Image clear timer
        /// </summary>
        private void imageClearTimerStop()
        {
            imageReset = 0;
        }

        private void textClearTimerStop()
        {
            textReset = 0;
        }

        /// <summary>
        /// Every IMAGE_RESETER_TIMER_PERIOD_MS check if we need to Clear some Image 
        /// </summary>
        /// <param name="Sender"></param>
        /// <param name="e"></param>
        private void imageClearTimerHandler(object sender, EventArgs e)
        {

            if (imageReset > 0)
            {
                imageReset -= RESETER_TIMER_PERIOD_MS;
                if (imageReset <= 0)
                {
                    try
                    {
                        checkFinger.ClearImage();
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
                       // checkFinger.SetText("Please, touch the scanner");
                    }
                    catch (Exception ex)
                    {
                        //Log.Error("Error while stopping the timer" + ex);
                    }
                }
            }
        }


        private void TestScannerDisable()
        {
            if (currentTestDevice != null)
            {
                if (currentTestDevice.Content is IFingerDevice)
                {
                    (currentTestDevice.Content as IFingerDevice).Dispatch(COMMAND.LIVECAPTURE_STOP);
                    Ambassador.ClearCallback();
                    checkFinger.ClearImage();
                }
            }
            if (imageReseter != null)
            {
                imageReseter.Stop();
            }
        }


        private void TestRefresh_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                TestRefreshButton.IsEnabled = false;
                this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal,
                    new refreshFetcher(RefreshTestPage));
            }
            catch (Exception ex)
            {
                //Log.Info(ex);
            }
        }
    }
}