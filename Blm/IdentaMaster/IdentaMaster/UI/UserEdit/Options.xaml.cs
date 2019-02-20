using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace IdentaZone.IdentaMaster.UserEdit
{
    /// <summary>
    /// Interaction logic for Options.xaml
    /// </summary>
    public partial class Options : UserControl
    {
        Base Owner;

        private void ShowMessage(String message)
        {
            Message.Content = message;
        }

        public Options(Base owner)
        {
            Owner = owner;
            InitializeComponent();
            this.loginType.SelectedIndex = (int)Owner.LoginType;
            pwdFirst.Password = Owner.Password;
        }

        private void ProceedClick(object sender, RoutedEventArgs e)
        {
            if (pwdFirst.Password == "")
            {
                pwdFirst.Password = "";
                MessageBox.Show("Password field should not be empty. If user has no Windows password, please, set it.", "Empty password", MessageBoxButton.OK, MessageBoxImage.Hand);
                ShowMessage("Password field shouldn't be empty");
                return;
            }
            if (!Auxiliary.CheckWinPassword(Owner.Username, pwdFirst.Password))
            {
                pwdFirst.Password = "";
                ShowMessage("Incorrect user password");
                pwdFirst.Focus();
                return;
            }
            XmlDB.LOGIN_TYPE loginType = new XmlDB.LOGIN_TYPE();
            switch (this.loginType.SelectedIndex)
            {
                case 0:
                    loginType = XmlDB.LOGIN_TYPE.PASS;
                    break;
                case 1:
                    loginType = XmlDB.LOGIN_TYPE.BIO;
                    break;
                case 2:
                    loginType = XmlDB.LOGIN_TYPE.MIXED;
                    break;
                default:
                    loginType = XmlDB.LOGIN_TYPE.PASS;
                    break;
            }
            Owner.OptionsProceed(pwdFirst.Password, loginType);
        }

        private void Options_Loaded(object sender, RoutedEventArgs e)
        {
            pwdFirst.Focus();
        }

        private void Grid_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                ProceedClick(sender, null);
            }
        }
    }
}
