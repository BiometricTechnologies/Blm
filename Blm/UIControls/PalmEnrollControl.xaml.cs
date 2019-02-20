using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace UIControlsINDSS
{
    /// <summary>
    /// Interaction logic for PalmEnrollControl.xaml
    /// </summary>
    public partial class PalmEnrollControl : UserControl
    {
        public PalmEnrollControl()
        {
            InitializeComponent();

            TypeControl = TypeEnum.Palm;
            LeftStatePalmP = StatePalm.White;
            RightStatePalmP = StatePalm.White;
        }

        public static readonly DependencyProperty FingerClickedProperty = DependencyProperty.Register(
                "FingerClicked",
                typeof(Action<int>),
                typeof(PalmEnrollControl),
                new FrameworkPropertyMetadata(null,
                    FrameworkPropertyMetadataOptions.None)
    );

        public Action<int> FingerClicked
        {
            get { return (Action<int>)GetValue(FingerClickedProperty); }
            set { SetValue(FingerClickedProperty, value); }
        }

        //public static readonly DependencyProperty SelectedFingerProperty = DependencyProperty.Register(
        //    "SelectedFinger",
        //    typeof(int),
        //    typeof(PalmEnrollControl),
        //    new FrameworkPropertyMetadata(-1,
        //        FrameworkPropertyMetadataOptions.AffectsRender, SelectedFingerChangedCbc));

        //public int SelectedFinger
        //{
        //    get { return (int)GetValue(SelectedFingerProperty); }
        //    set { SetValue(SelectedFingerProperty, value); }
        //}


        public static StatePalm GetLeftStatePalm(DependencyObject obj)
        {
            return (StatePalm)obj.GetValue(LeftStatePalmProperty);
        }

        public static void SetLeftStatePalm(DependencyObject obj, StatePalm value)
        {
            obj.SetValue(LeftStatePalmProperty, value);
        }

        // Using a DependencyProperty as the backing store for LeftStatePalm.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty LeftStatePalmProperty =
            DependencyProperty.RegisterAttached("LeftStatePalm", typeof(StatePalm), typeof(PalmEnrollControl), new PropertyMetadata(StatePalm.White, LeftStatePalmChangedCallback));

        private static void LeftStatePalmChangedCallback(DependencyObject dependencyObject, DependencyPropertyChangedEventArgs e)
        {
            var obj = (PalmEnrollControl)dependencyObject;
            obj.LeftStatePalmP = (StatePalm) e.NewValue;
        }

        private StatePalm _leftStatePalmP = StatePalm.White;
        public StatePalm LeftStatePalmP
        {
            get { return _leftStatePalmP; }
            set
            {
                _leftStatePalmP = value;
                switch (_leftStatePalmP)
                {
                    case StatePalm.White:
                        LeftWhite();
                        break;
                    case StatePalm.Orange:
                        LeftOrange();
                        break;
                    case StatePalm.Green:
                        LeftGreen();
                        break;
                    case StatePalm.WhiteBlack:
                        LeftWhiteBlack();
                        break;
                    case StatePalm.OrangeBlack:
                        LeftOrangeBlack();
                        break;
                    case StatePalm.GreenBlack:
                        LeftGreenBlack();
                        break;
                }

            }
        }


        public static StatePalm GetRightStatePalm(DependencyObject obj)
        {
            return (StatePalm)obj.GetValue(RightStatePalmProperty);
        }

        public static void SetRightStatePalm(DependencyObject obj, StatePalm value)
        {
            obj.SetValue(RightStatePalmProperty, value);
        }

        // Using a DependencyProperty as the backing store for RightStatePalm.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty RightStatePalmProperty =
            DependencyProperty.RegisterAttached("RightStatePalm", typeof(StatePalm), typeof(PalmEnrollControl), new PropertyMetadata(StatePalm.White, RightStatePalmChangedCallback));

        private static void RightStatePalmChangedCallback(DependencyObject dependencyObject, DependencyPropertyChangedEventArgs e)
        {
            var obj = (PalmEnrollControl) dependencyObject;
            obj.RightStatePalmP = (StatePalm) e.NewValue;
        }

        private StatePalm _rightStatePalmP=StatePalm.White;
        public StatePalm RightStatePalmP {
            get { return _rightStatePalmP; }
            set
            {
                _rightStatePalmP = value;
                switch (_rightStatePalmP)
                {
                    case StatePalm.White:
                        RightWhite();
                        break;
                    case StatePalm.Orange:
                        RightOrange();
                        break;
                    case StatePalm.Green:
                        RightGreen();
                        break;
                    case StatePalm.WhiteBlack:
                        RightWhiteBlack();
                        break;
                    case StatePalm.OrangeBlack:
                        RightOrangeBlack();
                        break;
                    case StatePalm.GreenBlack:
                        RightGreenBlack();
                        break;
                }
                
            }
        }

        public Action<int> ClickFingerAction { get; set; }

        private TypeEnum _typeControl = TypeEnum.Palm;

        public TypeEnum TypeControl
        {
            get { return _typeControl; }
            set
            {
                _typeControl = value;
                if (_typeControl == TypeEnum.Palm)
                {
                    path011.Visibility = Visibility.Visible;
                    path012.Visibility = Visibility.Visible;
                    path021.Visibility = Visibility.Visible;
                    path022.Visibility = Visibility.Visible;
                    path031.Visibility = Visibility.Collapsed;
                    path041.Visibility = Visibility.Collapsed;
                    return;
                }
                if (_typeControl == TypeEnum.Palm)
                {
                    path011.Visibility = Visibility.Collapsed;
                    path012.Visibility = Visibility.Collapsed;
                    path021.Visibility = Visibility.Collapsed;
                    path022.Visibility = Visibility.Collapsed;
                    path031.Visibility = Visibility.Visible;
                    path041.Visibility = Visibility.Visible;

                    return;
                }
                throw new Exception("Unknown type");
            }
        }

        private Brush _whiteBrush= new SolidColorBrush(Colors.White );
        private Brush _orangeBrush = new SolidColorBrush(Colors.Orange);
        private Brush _greenBrush = new SolidColorBrush(Colors.Green);
        private Brush _blackBrush = new SolidColorBrush(Colors.Black);
        private Brush _redBrush = new SolidColorBrush(Colors.Red);

        private void RightWhite()
        {
            path021.Fill = _whiteBrush;
            path022.Fill = _whiteBrush;
            //path023.Fill = _whiteBrush;
            //path024.Fill = _whiteBrush;
            //path025.Fill = _whiteBrush;
            //path026.Fill = _whiteBrush;
            //path027.Fill = _whiteBrush;
        }
        private void RightOrange()
        {
            path021.Fill = _orangeBrush;
            path022.Fill = _orangeBrush;
            //path023.Fill = _orangeBrush;
            //path024.Fill = _orangeBrush;
            //path025.Fill = _orangeBrush;
            //path026.Fill = _orangeBrush;
            //path027.Fill = _orangeBrush;
        }

        private void RightGreen()
        {
            path021.Fill = _greenBrush;
            path022.Fill = _greenBrush;
            //path023.Fill = _greenBrush;
            //path024.Fill = _greenBrush;
            //path025.Fill = _greenBrush;
            //path026.Fill = _greenBrush;
            //path027.Fill = _greenBrush;
        }

        private void RightWhiteBlack()
        {
            path021.Fill = _whiteBrush;
            path022.Fill = _redBrush;
            //path023.Fill = _blackBrush;
            //path024.Fill = _blackBrush;
            //path025.Fill = _blackBrush;
            //path026.Fill = _blackBrush;
            //path027.Fill = _blackBrush;
        }
        private void RightOrangeBlack()
        {
            path021.Fill = _orangeBrush;
            path022.Fill = _redBrush;
            //path023.Fill = _blackBrush;
            //path024.Fill = _blackBrush;
            //path025.Fill = _blackBrush;
            //path026.Fill = _blackBrush;
            //path027.Fill = _blackBrush;
        }

        private void RightGreenBlack()
        {
            path021.Fill = _greenBrush;
            path022.Fill = _redBrush;
            //path023.Fill = _blackBrush;
            //path024.Fill = _blackBrush;
            //path025.Fill = _blackBrush;
            //path026.Fill = _blackBrush;
            //path027.Fill = _blackBrush;
        }

        private void LeftWhite()
        {
            path011.Fill = _whiteBrush;
            path012.Fill = _whiteBrush;
            //path013.Fill = _whiteBrush;
            //path014.Fill = _whiteBrush;
            //path015.Fill = _whiteBrush;
        }

        private void LeftOrange()
        {
            path011.Fill = _orangeBrush;
            path012.Fill = _orangeBrush;
            //path013.Fill = _orangeBrush;
            //path014.Fill = _orangeBrush;
            //path015.Fill = _orangeBrush;
        }

        private void LeftGreen()
        {
            path011.Fill = _greenBrush;
            path012.Fill = _greenBrush;
            //path013.Fill = _greenBrush;
            //path014.Fill = _greenBrush;
            //path015.Fill = _greenBrush;
        }

        private void LeftWhiteBlack()
        {
            path011.Fill = _whiteBrush;
            path012.Fill = _redBrush;
            //path013.Fill = _blackBrush;
            //path014.Fill = _blackBrush;
            //path015.Fill = _blackBrush;
        }

        private void LeftOrangeBlack()
        {
            path011.Fill = _orangeBrush;
            path012.Fill = _redBrush;
            //path013.Fill = _blackBrush;
            //path014.Fill = _blackBrush;
            //path015.Fill = _blackBrush;
        }

        private void LeftGreenBlack()
        {
            path011.Fill = _greenBrush;
            path012.Fill = _redBrush;
            //path013.Fill = _blackBrush;
            //path014.Fill = _blackBrush;
            //path015.Fill = _blackBrush;
        }

        private void MouseLeftButtonUpPath11(object sender, MouseButtonEventArgs e)
        {
            if (ClickFingerAction != null) ClickFingerAction(10);
            if (FingerClicked != null) FingerClicked(10);
            path011.Opacity = 1;
            path012.Opacity = 1;
        }

        private void MouseLeftButtonUpPath12(object sender, MouseButtonEventArgs e)
        {
            if (ClickFingerAction != null) ClickFingerAction(11);
            if (FingerClicked != null) FingerClicked(11);
            path021.Opacity = 1;
            path022.Opacity = 1;
        }

        public void SetOriginalColor()
        {
            if (!flEnterR) RightStatePalmP = _rightStatePalmP;

            if (!flEnterL) LeftStatePalmP = _leftStatePalmP;
        }
        private bool flEnterL = false;
        private bool flEnterR = false;
        private void MouseEnterPath11(object sender, MouseEventArgs e)
        {
            flEnterL = true;
            if ((_leftStatePalmP != StatePalm.Orange) && (_leftStatePalmP != StatePalm.OrangeBlack))
            {
                path011.Opacity = 0.5;
                path012.Opacity = 0.5;
            }
            LeftOrange();
        }

        private void MouseLeavePath11(object sender, MouseEventArgs e)
        {
            flEnterL = false;
            path011.Opacity = 1;
            path012.Opacity = 1;
            LeftStatePalmP = _leftStatePalmP;
            
        }

        private void MouseEnterPath12(object sender, MouseEventArgs e)
        {
            flEnterR = true;
            if ((_rightStatePalmP != StatePalm.Orange) && (_rightStatePalmP != StatePalm.OrangeBlack))
            {
                path021.Opacity = 0.5;
                path022.Opacity = 0.5;
            }
            RightOrange();
        }

        private void MouseLeavePath12(object sender, MouseEventArgs e)
        {
            flEnterR = false;
            path021.Opacity = 1;
            path022.Opacity = 1;
            RightStatePalmP = _rightStatePalmP;
        }

    }

    public enum StatePalm
    {
        White, Orange, Green, OrangeBlack, GreenBlack, WhiteBlack
    }

    public enum TypeEnum
    {
        Palm, Eyes
    }
}
