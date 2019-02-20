using System;
using System.Collections.Generic;
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
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace UIControlsINDSS
{
    /// <summary>
    /// Interaction logic for MainWindowTray.xaml
    /// </summary>
    public partial class MainWindowTray : Window
    {
        Licenser Licenser = new Licenser();

        public MainWindowTray()
        {
            InitializeComponent();
        }

        private void Button_Click_Ok(object sender, RoutedEventArgs e)
        {
            LockActivationForm();
            _statusBar.Content = "Activation in progress...";
            Licenser.Activate(_licenseCodTB.Text, _emailTB.Text, OnActivation);
        }

        private void LockActivationForm()
        {
            _tabControl.IsEnabled = false;
        }

        private void UnLockActivationForm()
        {
            _tabControl.IsEnabled = true;
        }

        private void Button_Click_Clear(object sender, RoutedEventArgs e)
        {
            _licenseCodTB.Text = "";
            _emailTB.Text="";
        }

        private void OnActivation(bool success, string reason)
        {
            if (success)
            {
                _statusBar.Content = "License successfully activated";
            }
            else
            {
                _statusBar.Content = reason;
            }
            UnLockActivationForm();
        }
    }
}
