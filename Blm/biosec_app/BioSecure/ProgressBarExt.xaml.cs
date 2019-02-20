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

namespace IdentaZone.BioSecure
{
    /// <summary>
    /// Interaction logic for ProgressBarExt.xaml
    /// </summary>
    public partial class ProgressBarExt : UserControl
    {
        public int Value
        {
            get
            {
                return (int)ProgressBar.Value;
            }
            set
            {
                ProgressBar.Value = value;
            }
        }

        public String Text
        {
            set
            {
                Label.Text = value;
            }
        }
        public ProgressBarExt()
        {
            InitializeComponent();
        }
    }
}
