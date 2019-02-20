using IdentaZone.IdentaMaster.UserEdit;
using System.Windows;
using System.Windows.Controls;

namespace IdentaZone.IdentaMaster
{
    /// <summary>
    /// Interaction logic for ActivateUser.xaml
    /// </summary>
    public partial class Activate : UserControl
    {
        Base Owner;
        public Activate(Base owner)
        {
            Owner = owner;
            InitializeComponent();
            Fullname.Text = Owner.Fullname;
        }

        private void ProceedClick(object sender, RoutedEventArgs e)
        {
            Owner.ActivateProceed();
        }

        private void CancelClick(object sender, RoutedEventArgs e)
        {
            Owner.ActivateCancel();
        }
    }
}
