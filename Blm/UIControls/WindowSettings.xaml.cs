using System.ComponentModel;
using System.Windows;
using UIControlsINDSS.Properties;

namespace UIControlsINDSS
{
    /// <summary>
    /// Interaction logic for WindowSettings.xaml
    /// </summary>
    public partial class WindowSettings : Window, INotifyPropertyChanged
    {


        public WindowSettings()
        {
            InitializeComponent();
        }

        #region INotifyPropertyChanged Members

        public event PropertyChangedEventHandler PropertyChanged;

        public void OnPropertyChanged(string propertyName)
        {
            PropertyChangedEventHandler handler = this.PropertyChanged;
            if (handler != null)
                handler(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion // INotifyPropertyChanged Members

        private void ButtonBase_OnClick(object sender, RoutedEventArgs e)
        {
            Settings.Default.Save();
            this.DialogResult = true;
        }
    }
}
