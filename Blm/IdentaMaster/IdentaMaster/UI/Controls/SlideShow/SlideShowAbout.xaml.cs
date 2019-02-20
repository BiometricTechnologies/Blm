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
using Transitionals.Controls;

namespace IdentaZone.IdentaMaster
{
    /// <summary>
    /// Interaction logic for SlideShowAbout.xaml
    /// </summary>
    public partial class SlideShowAbout : UserControl
    {
        private bool firstchange = true;
        public SlideShowAbout()
        {
            InitializeComponent();
            
        }

        private void Slideshow_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (firstchange)
            {
                (sender as Slideshow).AutoAdvanceDuration = TimeSpan.FromSeconds(2.5);
                firstchange = false;
            }
            //switch ((Sender as Slideshow).SelectedIndex){
            //    case 0:
            //        bull1.IsChecked = true;
            //        break;
            //    case 1:
            //        bull2.IsChecked = true;
            //        break;
            //    case 2:
            //        bull3.IsChecked = true;
            //        break;
            //    case 3:
            //        bull4.IsChecked = true;
            //        break;
            //    case 4:
            //        bull5.IsChecked = true;
            //        break;
            //    case 5:
            //        bull6.IsChecked = true;
            //        break;
            //}
        }

        private void bull1_Click(object sender, RoutedEventArgs e)
        {
            Slideshow.SelectedIndex = 0;
        }

        private void bull2_Click(object sender, RoutedEventArgs e)
        {
            Slideshow.SelectedIndex = 1;
        }

        private void bull3_Click(object sender, RoutedEventArgs e)
        {
            Slideshow.SelectedIndex = 2;
        }

        private void bull4_Click(object sender, RoutedEventArgs e)
        {
            Slideshow.SelectedIndex = 3;
        }

        private void bull5_Click(object sender, RoutedEventArgs e)
        {
            Slideshow.SelectedIndex = 4;
        }

        private void bull6_Click(object sender, RoutedEventArgs e)
        {
            Slideshow.SelectedIndex = 5;
        }




    }
}
