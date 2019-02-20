using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Media;

namespace UIControlsINDSS
{
    /// <summary>
    /// Interaction logic for MessageBoxINDSS.xaml
    /// </summary>
    public partial class MessageBoxINDSS : Window, INotifyPropertyChanged
    {
        private MessageBoxType _type;


        public string Message { get; set; }
        public string TitleTxt { get; set; }
        public SolidColorBrush MessageColor
        {
            get
            {
                switch (_type)
                {
                    case MessageBoxType.Error:
                        return new SolidColorBrush(Colors.White);
                        break;                
                    case MessageBoxType.Info:
                    default:
                        return new SolidColorBrush(Colors.White);
                        break;
                }
            }
        }

        public MessageBoxINDSS(String message, MessageBoxType type, string title, MessageBoxTypeDialog typeDialog )
        {
            TitleTxt = title;
            Message = message;
            _type = type;


            InitializeComponent();
            if (typeDialog == MessageBoxTypeDialog.Ok)
            {
                _dialogOk.Visibility = Visibility.Visible;
                _dialogYesNo.Visibility = Visibility.Collapsed;
            }
            if (typeDialog == MessageBoxTypeDialog.YesNo)
            {
                _dialogOk.Visibility = Visibility.Collapsed;
                _dialogYesNo.Visibility = Visibility.Visible;
            }
        }

        private void Ok()
        {
            this.Close();
        }

        public enum MessageBoxType
        {
            Info,
            Error
        }

        public enum MessageBoxTypeDialog
        {
            Ok,
            YesNo
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

        private void ButtonBase_OnClickOk(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
            //this.Close();
        }
        private void ButtonBase_OnClickYes(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
            //this.Close();
            
        }
        private void ButtonBase_OnClickNo(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            //this.Close();
        }

    }
}
