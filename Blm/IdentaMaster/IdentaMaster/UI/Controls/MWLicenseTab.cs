using IdentaZone.BioControls.Auxiliary;
using System;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using csharp_aes_encryptor;
using Microsoft.Win32;
using System.Collections.Generic;

namespace IdentaZone.IdentaMaster
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        /// <summary>
        /// 
        /// </summary>
        /// 
        private List<string> CheckLDate()
        {
            try
            {
                Encryptor en = new AesEncryptor();
                using (RegistryKey skey = Registry.LocalMachine.OpenSubKey("Software", true))
                {
                    using (RegistryKey key = skey.OpenSubKey("IdentaMaster", RegistryKeyPermissionCheck.ReadWriteSubTree))
                    {
                        byte[] entropy = { 0x00, 0x01, 0x02, 0x01, 0x00, 0x04, 0xBB, 0x17, 0x8b, 0xf6, 0xa2, 0x15, 0xe2, 0x64, 0x11, 0x9a };
                        var appsetString = System.Text.Encoding.Default.GetString(en.Decrypt((byte[])key.GetValue("AppSet"), entropy));
                        int indof0 = appsetString.IndexOf("^^^");
                        string cleanAppsetString = appsetString.Substring(0, indof0);

                        string[] Separ = { "..." };
                        appSet = new List<string>(cleanAppsetString.Split(Separ, StringSplitOptions.None));
                        if (appSet.Count > 0)
                        {
                            return appSet;
                        }
                        else
                        {
                            throw new Exception("noDate");
                        }
                    }
                }
            }
            catch (Exception E)
            {
                Log.Error(E.Message);
                return appSet;
            }
        }
        //private bool CheckLDevice(string devName)
        //{
        //    string LDate = CheckLDate();
        //    Encryptor en = new AesEncryptor();
        //    if (LInfo != null)
        //    {

        //    }
        //    return true;
        //}
        private void UpdateAppset()
        {
            Encryptor en = new AesEncryptor();
            using (RegistryKey skey = Registry.LocalMachine.OpenSubKey("Software", true))
            {
                using (RegistryKey key = skey.CreateSubKey("IdentaMaster", RegistryKeyPermissionCheck.ReadWriteSubTree))
                {
                    byte[] entropy = { 0x00, 0x01, 0x02, 0x01, 0x00, 0x04, 0xBB, 0x17, 0x8b, 0xf6, 0xa2, 0x15, 0xe2, 0x64, 0x11, 0x9a };
                    appSet[0] = DateTime.Now.ToString();
                    string newappset = appSet[0];

                    foreach (var set in appSet)
                    {
                        if (newappset != set)
                        {
                            newappset += "..." + set;
                        }
                    }
                    foreach (var devctrl in pluginManager.DeviceControls)
                    {
                        if (!appSet.Contains(devctrl.ToString()))
                        {
                            appSet.Add(devctrl.ToString());
                            newappset += "..." + devctrl.ToString();
                        }
                    }
                    newappset += "^^^";
                    //newappset = newappset.Replace("\\0", "");
                    var encappset = System.Text.Encoding.Default.GetBytes(newappset);// System.Text.Encoding.UTF8.GetString();
                    byte[] toenc = new byte[(16 - encappset.Length % 16) + encappset.Length];
                    encappset.CopyTo(toenc, 0);
                    byte[] encAppSet = en.Encrypt(toenc, entropy);
                    key.SetValue("AppSet", encAppSet);
                }
            }
        }

        private void UpdateLDevices()
        {
            Encryptor en = new AesEncryptor();
            using (RegistryKey skey = Registry.LocalMachine.OpenSubKey("Software", true))
            {
                using (RegistryKey key = skey.CreateSubKey("IdentaMaster", RegistryKeyPermissionCheck.ReadWriteSubTree))
                {
                    byte[] entropy = { 0x00, 0x01, 0x02, 0x01, 0x00, 0x04, 0xBB, 0x17, 0x8b, 0xf6, 0xa2, 0x15, 0xe2, 0x64, 0x11, 0x9a };
                    string newappset = appSet[0];

                    foreach (var set in appSet)
                    {
                        if (newappset != set)
                        {
                            newappset += "..." + set;
                        }
                    }
                    foreach (var devctrl in pluginManager.DeviceControls)
                    {
                        if (!appSet.Contains(devctrl.ToString()))
                        {
                            appSet.Add(devctrl.ToString());
                            newappset += "..." + devctrl.ToString();
                        }
                    }
                    newappset += "^^^";
                    //newappset = newappset.Replace("\\0", "");
                    var encappset = System.Text.Encoding.Default.GetBytes(newappset);// System.Text.Encoding.UTF8.GetString();
                    byte[] toenc = new byte[(16 - encappset.Length % 16) + encappset.Length];
                    encappset.CopyTo(toenc, 0);
                    byte[] encAppSet = en.Encrypt(toenc, entropy);
                    key.SetValue("AppSet", encAppSet);
                }
            }
        }
        private void ResetLDate()
        {
            Encryptor en = new AesEncryptor();
            appSet.Clear();
            using (RegistryKey skey = Registry.LocalMachine.OpenSubKey("Software", true))
            {
                using (RegistryKey key = skey.CreateSubKey("IdentaMaster", RegistryKeyPermissionCheck.ReadWriteSubTree))
                {
                    byte[] entropy = { 0x00, 0x01, 0x02, 0x01, 0x00, 0x04, 0xBB, 0x17, 0x8b, 0xf6, 0xa2, 0x15, 0xe2, 0x64, 0x11, 0x9a };
                    
                    var newappset = DateTime.Now.ToString();
                    appSet.Add(newappset);
                    foreach (var devctrl in pluginManager.DeviceControls)
                    {
                        newappset += "..." + devctrl.ToString();
                        appSet.Add(devctrl.ToString());
                    }
                    newappset += "^^^";
                    var encappset = System.Text.Encoding.Default.GetBytes(newappset);// System.Text.Encoding.UTF8.GetString();
                    byte[] toenc = new byte[(16 - encappset.Length % 16) + encappset.Length];
                    encappset.CopyTo(toenc, 0);
                    byte[] encAppSet = en.Encrypt(toenc, entropy);
                    key.SetValue("AppSet", encAppSet);
                }
            }
        }
        
        ///
        Licenser Licenser = new Licenser();

        private void LicenseActivationCbc()
        {
            if (Licenser.State == Licenser.STATE.ACTIVATED)
            {
                this.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal,
                    new Action(() => uiLicenseActivated()));
            }
            if (Licenser.State == Licenser.STATE.NOT_REGISTERED)
            {
                this.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Normal,
                   new Action(() => uiLicenseNotRegistered()));
            }
        }

        private void LicenseTabInit()
        {
            try
            {
                foreach (var child in licenseStackPanel.Children)
                {
                    DataObject.AddPastingHandler((child as TextBox), new DataObjectPastingEventHandler(OnPaste));
                }
                LockActivationForm();
                Licenser.Init(LicenseActivationCbc);
                licensePrompt.Content = "";
            }
            catch (Exception ex)
            {
                Log.Error("licenser init crashed: ", ex);
            }
        }

        private String[] GetLicense(String buffer)
        {
            var match = Regex.Match(buffer, "([A-Z0-9]{6})-([A-Z0-9]{6})-([A-Z0-9]{6})-([A-Z0-9]{6})-([A-Z0-9]{6})-([A-Z0-9]{6})");
            if (match.Success)
            {
                String[] result = match.Captures[0].Value.Split('-');
                return result;
            }
            else
            {
                return null;
            }
        }

        private String GetMail(String buffer)
        {
            if (buffer.StartsWith("email="))
            {
                return buffer.Replace("email=", "");
            }
            else
            {
                return "";
            }
        }
        private String[] SplitLicense(String Buffer)
        {
            String[] license = null;
            String email = "";
            // Check if it has Email On-board
            var firstSplit = Buffer.Split('&');
            if (firstSplit.Length == 2)
            {
                license = GetLicense(firstSplit[0]);
                email = GetMail(firstSplit[1]);
            }
            else
            {
                license = GetLicense(Buffer);
            }

            if (license == null)
            {
                return null;
            }
            else
            {

                var result = new String[6 + 1];
                license.CopyTo(result, 0);
                result[6] = email;
                return result;
            }
        }

        private void OnPaste(object sender, DataObjectPastingEventArgs e)
        {
            var isText = e.SourceDataObject.GetDataPresent(System.Windows.DataFormats.Text, true);
            if (!isText) return;

            var text = e.SourceDataObject.GetData(DataFormats.Text) as string;

            var split = SplitLicense(text);

            if (split == null) return;

            int index = 0;

            foreach (var child in licenseStackPanel.Children)
            {
                (child as TextBox).Text = split[index++];
            }

            if (split[6] != "")
            {
                LicenseEmail.Text = split[6];
            }
        }

        /// <summary>
        /// Clears the license.
        /// </summary>
        /// <param name="Sender">The Sender.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void ClearLicense(object sender, RoutedEventArgs e)
        {
            ClearLicense();
        }
        List<string> BakKey = new List<string>();
        string BakEmail;
        private bool licenseUpdated = false;
        private void BackupLicense()
        {
            BakKey.Clear();
            foreach (var child in licenseStackPanel.Children)
            {
                BakKey.Add((child as TextBox).Text);
            }
            BakEmail = LicenseEmail.Text;
        }
        private void RestoreLicense()
        {
            int i = 0;
            foreach (var child in licenseStackPanel.Children)
            {
                (child as TextBox).Text = BakKey[i];
                i++;
            }
            LicenseEmail.Text = BakEmail;
        }
        private void ClearLicense()
        {
            licensePrompt.Content = "";
            foreach (var child in licenseStackPanel.Children)
            {
                (child as TextBox).Clear();
            }
            LicenseEmail.Clear();
        }

        private void uiLicenseNotRegistered()
        {
            licenseRedLabel.Visibility = System.Windows.Visibility.Collapsed;
            setProvidersUnActivated();
            UnlockActivationForm();
            UpdateTestTabLicenseState();
        }


        /// <summary>
        /// UIs the license activated.
        /// </summary>
        private void uiLicenseActivated()
        {
            enterLicensePromt.Content = "Your license number";
            setProvidersActivated();
            //licenseButtonsStackPanel.Visibility = System.Windows.Visibility.Collapsed;
            LicenseOk.Visibility = System.Windows.Visibility.Collapsed;
            LicenseClear.Visibility = System.Windows.Visibility.Collapsed;
            CancelLicenseUpdate.Visibility = System.Windows.Visibility.Collapsed;

            LicenseUpdate.Visibility = System.Windows.Visibility.Visible;

            licenseRedLabel.Visibility = System.Windows.Visibility.Collapsed;
            foreach (var child in licenseStackPanel.Children)
            {
                (child as TextBox).IsReadOnly = true;
            }
            LicenseEmail.IsReadOnly = true;

            int tbIndex = 0;
            string[] licenseKeySignsGroup = (Licenser.LicenseKey).Split('-');
            if (licenseKeySignsGroup.Length == 6)
            {
                foreach (var child in licenseStackPanel.Children)
                {
                    (child as TextBox).Text = licenseKeySignsGroup[tbIndex++];
                }
            }
            LicenseEmail.Text = Licenser.Email;
            ///
            CheckLDate();
            if (appSet.Count == 0)
            {
                ResetLDate();
            }
            else if (licenseUpdated)
            {
                UpdateAppset();
                licenseUpdated = false;
            }
            else if (DateTime.Now - DateTime.Parse(appSet[0]) < TimeSpan.FromDays(5.0))
            {
                UpdateLDevices();
            }
            
            UpdateAfterLicense = true;
            ///
            UnlockActivationForm();
            UpdateTestTab();
        }


        /// <summary>
        /// Enters the license.
        /// </summary>
        /// <param name="Sender">The Sender.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void EnterLicense(object sender, RoutedEventArgs e)
        {
            LockActivationForm();

            Dispatcher.Invoke(new Action(() => licensePrompt.Content = "Activation in progress..."));
            Licenser.Activate(AssemblyLicenseKey(), LicenseEmail.Text, OnActivation);
        }

        private void OnActivation(bool success, string reason)
        {
            if (success)
            {
                Dispatcher.Invoke(new Action(() => uiLicenseActivated()));
                Dispatcher.Invoke(new Action(() => licensePrompt.Content = "License successfully activated"));
            }
            else
            {
                Dispatcher.Invoke(new Action(() => licensePrompt.Content = reason));
            }
            Dispatcher.Invoke(new Action(() => UnlockActivationForm()));
        }

        private void LicenseUpdate_Click(object sender, RoutedEventArgs e)
        {
            licenseUpdated = true;
            BackupLicense();
            ClearLicense();
            LicenseUpdate.Visibility = System.Windows.Visibility.Collapsed;
            LicenseOk.Visibility = System.Windows.Visibility.Visible;
            LicenseClear.Visibility = System.Windows.Visibility.Visible;
            CancelLicenseUpdate.Visibility = System.Windows.Visibility.Visible;
            foreach (var child in licenseStackPanel.Children)
            {
                (child as TextBox).IsReadOnly = false;
            }
            LicenseEmail.IsReadOnly = false;
        }
        private void CancelLicenseUpdate_Click(object sender, RoutedEventArgs e)
        {
            RestoreLicense();
            licensePrompt.Content = "";
            licenseUpdated = false;
            LicenseOk.Visibility = System.Windows.Visibility.Collapsed;
            LicenseClear.Visibility = System.Windows.Visibility.Collapsed;
            CancelLicenseUpdate.Visibility = System.Windows.Visibility.Collapsed;

            LicenseUpdate.Visibility = System.Windows.Visibility.Visible;
            foreach (var child in licenseStackPanel.Children)
            {
                (child as TextBox).IsReadOnly = true;
            }
            LicenseEmail.IsReadOnly = true;
        }

        /// <summary>
        /// Assemblies the license key.
        /// </summary>
        /// <returns></returns>
        private string AssemblyLicenseKey()
        {
            System.Text.StringBuilder sb = new System.Text.StringBuilder();
            foreach (var child in licenseStackPanel.Children)
            {
                sb.Append((child as TextBox).Text);
                sb.Append("-");
            }
            sb.Remove(sb.Length - 1, 1);
            return sb.ToString();
        }

        /// <summary>
        /// Licenses the text changed.
        /// </summary>
        /// <param name="Sender">The Sender.</param>
        /// <param name="e">The <see cref="TextChangedEventArgs"/> instance containing the event data.</param>
        private void LicenseTextChanged(object sender, TextChangedEventArgs e)
        {
            
            if ((sender as TextBox).Text.Length == (sender as TextBox).MaxLength)
            {
                TraversalRequest tRequest = new TraversalRequest(FocusNavigationDirection.Next);
                UIElement keyboardFocus = Keyboard.FocusedElement as UIElement;

                if (keyboardFocus != null)
                {
                    keyboardFocus.MoveFocus(tRequest);
                }
            }
        }

        /// <summary>
        /// Activates the providers.
        /// </summary>
        private void setProvidersActivated()
        {
            SolidColorBrush customBrush = (SolidColorBrush)(new BrushConverter().ConvertFrom(greenColor));
            //dpActivatedLabel.Foreground = customBrush;
            //ibActivatedLabel.Foreground = customBrush;
            //sgActivatedLabel.Foreground = customBrush;
            //dpActivatedImg.Source = (ImageSource)new ImageSourceConverter().ConvertFrom(new Uri(@"pack://application:,,,/images/checkbox/green.png"));
            //ibActivatedImg.Source = (ImageSource)new ImageSourceConverter().ConvertFrom(new Uri(@"pack://application:,,,/images/checkbox/green.png"));
            //sgActivatedImg.Source = (ImageSource)new ImageSourceConverter().ConvertFrom(new Uri(@"pack://application:,,,/images/checkbox/green.png"));
        }

        private void setProvidersUnActivated()
        {
            SolidColorBrush customBrush = (SolidColorBrush)(new SolidColorBrush(Colors.Red));
            //dpActivatedLabel.Foreground = customBrush;
            //ibActivatedLabel.Foreground = customBrush;
            //sgActivatedLabel.Foreground = customBrush;
            //dpActivatedImg.Source = (ImageSource)new ImageSourceConverter().ConvertFrom(new Uri(@"pack://application:,,,/images/checkbox/red.png"));
            //ibActivatedImg.Source = (ImageSource)new ImageSourceConverter().ConvertFrom(new Uri(@"pack://application:,,,/images/checkbox/red.png"));
            //sgActivatedImg.Source = (ImageSource)new ImageSourceConverter().ConvertFrom(new Uri(@"pack://application:,,,/images/checkbox/red.png"));
        }


        /// <summary>
        /// Licenses the focused.
        /// </summary>
        /// <param name="Sender">The Sender.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void LicenseFocused(object sender, RoutedEventArgs e)
        {
            (sender as TextBox).SelectAll();
        }

        private void LockActivationForm()
        {
            foreach (var child in licenseStackPanel.Children)
            {
                (child as TextBox).IsEnabled = false;
            }

            LicenseOk.IsEnabled = false;
            LicenseClear.IsEnabled = false;
            LicenseEmail.IsEnabled = false;
            CancelLicenseUpdate.IsEnabled = false;
        }

        private void UnlockActivationForm()
        {
            foreach (var child in licenseStackPanel.Children)
            {
                (child as TextBox).IsEnabled = true;
            }

            LicenseOk.IsEnabled = true;
            LicenseClear.IsEnabled = true;
            LicenseEmail.IsEnabled = true;
            CancelLicenseUpdate.IsEnabled = true;
        }

        private void LicesingTabSelected()
        {
            //Workaround!
            System.Threading.ThreadPool.QueueUserWorkItem(
                  (a) =>
                  {
                      System.Threading.Thread.Sleep(100);
                      licenseTb0.Dispatcher.Invoke(
                      new Action(() =>
                      {
                          licenseTb0.Focus();

                      }));
                  }
                  );
        }
    }
}