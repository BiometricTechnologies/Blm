using IdentaZone.IdentaMaster.UserEdit;
using System.Windows;
using System.Windows.Controls;

namespace IdentaZone.IdentaMaster.UI.UserEdit
{
    /// <summary>
    /// Interaction logic for InfoBox.xaml
    /// </summary>
    public partial class InfoBox : UserControl
    {
        Base Owner;
        public InfoBox(Base baseWindow)
        {
            Owner = baseWindow;
            InitializeComponent();
        }

        public void ProceedClick(object sender, RoutedEventArgs e)
        {
            Owner.InfoBoxProceed();
        }
    }
}
