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

namespace WPF_ChromaScenePlayer
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private List<Button> _mRefButtons = new List<Button>();

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Closed(object sender, EventArgs e)
        {

        }

        private void BtnLoadScene_Click(object sender, RoutedEventArgs e)
        {
            _mButtons.Children.Clear();
            _mRefButtons.Clear();

            for (int i = 0; i < 20; ++i)
            {
                StackPanel stackPanel = new StackPanel();
                stackPanel.Orientation = Orientation.Horizontal;
                if (i == 0)
                {
                    stackPanel.Margin = new Thickness(20, 20, 0, 0);
                }
                else
                {
                    stackPanel.Margin = new Thickness(20, 10, 0, 0);
                }
                stackPanel.Background =  new SolidColorBrush(Color.FromArgb(255, 34, 34, 34));
                stackPanel.HorizontalAlignment = HorizontalAlignment.Stretch;
                Button button = new Button();
                button.DataContext = i;
                if (i == 0)
                {
                    button.Background = new SolidColorBrush(Color.FromArgb(255, 0, 255, 0));
                }
                else
                {
                    button.Background = new SolidColorBrush(Colors.White);
                }
                TextBlock textButton = new TextBlock();
                textButton.Text = string.Format("{0}", i + 1);
                textButton.Foreground = new SolidColorBrush(Colors.Black);
                textButton.Padding = new Thickness(10);
                button.Content = textButton;
                button.Click += BtnScene_Click;
                _mRefButtons.Add(button);
                stackPanel.Children.Add(button);
                TextBlock textDescription = new TextBlock();
                textDescription.Text = "Description";
                textDescription.Foreground = new SolidColorBrush(Colors.White);
                textDescription.Margin = new Thickness(10);
                textDescription.HorizontalAlignment = HorizontalAlignment.Stretch;
                stackPanel.Children.Add(textDescription);

                _mButtons.Children.Add(stackPanel);
            }
        }

        private void BtnPlay_Click(object sender, RoutedEventArgs e)
        {

        }

        private void BtnStop_Click(object sender, RoutedEventArgs e)
        {

        }

        private void BtnScene_Click(object sender, RoutedEventArgs e)
        {
            foreach (Button button in _mRefButtons)
            {
                if (button == sender)
                {
                    button.Background = new SolidColorBrush(Color.FromArgb(255, 0, 255, 0));
                }
                else
                {
                    button.Background = new SolidColorBrush(Colors.White);
                }
            }
        }
    }
}
