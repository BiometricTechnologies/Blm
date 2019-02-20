using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;

namespace IdentaZone.IdentaMaster.UserEdit
{
    /// <summary>
    /// Interaction logic for FingerChooser.xaml
    /// </summary>
    public partial class FingerChooser : UserControl
    {
        Action<int> FingerClicked;
        int selectedFinger = -1;
        int highlightedFinger = -1;
        public FingerChooser(List<int> EnrolledFingers, Action<int> fingerClicked)
        {
            InitializeComponent();
            FingerClicked = fingerClicked;
            UpdateFingers(EnrolledFingers);
        }

        private void FingerMouseDown(object sender, MouseButtonEventArgs e)
        {
            string content = (sender as Path).Name.ToString();
            int num = Convert.ToInt32(content.Substring(1));
            FingerClicked(num);
        }

        private void FingerMouseEnter(object sender, MouseEventArgs e)
        {
            Image img = null;
            if (highlightedFinger != -1)
            {
                img = (Image)FindName("i" + Convert.ToString(highlightedFinger));
                //img.Source = (ImageSource)new ImageSourceConverter().ConvertFrom(new Uri(@"pack://application:,,,/images/fingers/" + Convert.ToString(num) + ".png"));
                img.Visibility = Visibility.Hidden;
            }

            string content = (sender as Path).Name.ToString();
            int num = Convert.ToInt32(content.Substring(1));
            img = (Image)FindName("i" + Convert.ToString(num));
            img.Source = (ImageSource)new ImageSourceConverter().ConvertFrom(new Uri(@"pack://application:,,,/images/fingers/" + Convert.ToString(num) + ".png"));
            img.Visibility = Visibility.Visible;
        }

        private void FingerMouseLeave(object sender, MouseEventArgs e)
        {
            string content = (sender as Path).Name.ToString();
            int num = Convert.ToInt32(content.Substring(1));
            if (num == selectedFinger)
            {
                return;
            }
            Image img = (Image)FindName("i" + Convert.ToString(num));
            img.Visibility = Visibility.Hidden;
        }

        internal void SelectFinger(int fingerNum)
        {
            Image img = (Image)FindName("i" + Convert.ToString(fingerNum));
            img.Visibility = Visibility.Visible;
            selectedFinger = fingerNum;
        }

        internal void DeSelectFinger(int fingerNum)
        {
            Image img = (Image)FindName("i" + Convert.ToString(fingerNum));
            img.Visibility = Visibility.Hidden;
            selectedFinger = -1;
        }

        public void ShowFinger(int fingerNum)
        {
            if (fingerNum == highlightedFinger)
            {
                return;
            }
            Image img = null;
            if (highlightedFinger != -1)
            {
                img = (Image)FindName("i" + Convert.ToString(highlightedFinger));
                img.Source = (ImageSource)new ImageSourceConverter().ConvertFrom(new Uri(@"pack://application:,,,/images/fingers/g" + highlightedFinger +".png"));
                img.Visibility = Visibility.Hidden;
            }

            img = (Image)FindName("i" + Convert.ToString(fingerNum));
            img.Source = (ImageSource)new ImageSourceConverter().ConvertFrom(new Uri(@"pack://application:,,,/images/fingers/g" + fingerNum + ".png"));
            img.Visibility = Visibility.Visible;
            highlightedFinger = fingerNum;
        }

        internal void UpdateFingers(List<int> newFingers)
        {
            for (int i = 0; i < 10; i++)
            {
                Image img = (Image)FindName("p" + i);
                img.Visibility = Visibility.Hidden;
            }
            foreach (int finger in newFingers)
            {
                Image img = (Image)FindName("p" + finger);
                img.Visibility = Visibility.Visible;
            }

            if (highlightedFinger != -1)
            {
                Image img = (Image)FindName("i" + Convert.ToString(highlightedFinger));
                //Path path = (Path)FindName("f" + Convert.ToString(highlightedFinger));
                img.Visibility = Visibility.Hidden;
                highlightedFinger = -1;
            }
        }
    }
}
