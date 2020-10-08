using Microsoft.Win32;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace WPF_ChromaScenePlayer
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private List<Button> _mRefButtons = new List<Button>();
        private bool _mInitialized = false;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            PlayerDLL.ApplicationStart();
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            PlayerDLL.ApplicationQuit();
        }
        
        private void AddButton(int i, string description)
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
            stackPanel.Background = new SolidColorBrush(Color.FromArgb(255, 34, 34, 34));
            stackPanel.HorizontalAlignment = HorizontalAlignment.Stretch;
            Button button = new Button();
            button.DataContext = i;
            if (i == 0)
            {
                button.Background = new SolidColorBrush(Color.FromArgb(255, 0, 255, 0));
            }
            else
            {
                button.Background = new SolidColorBrush(Colors.LightGray);
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
            textDescription.Text = description;
            textDescription.Foreground = new SolidColorBrush(Colors.White);
            textDescription.Margin = new Thickness(10);
            textDescription.HorizontalAlignment = HorizontalAlignment.Stretch;
            stackPanel.Children.Add(textDescription);

            _mButtons.Children.Add(stackPanel);
        }

        private void BtnLoadScene_Click(object sender, RoutedEventArgs e)
        {
            _mButtons.Children.Clear();
            _mRefButtons.Clear();

            OpenFileDialog openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == true)
            {
                string path = openFileDialog.FileName;
                if (!string.IsNullOrEmpty(path))
                {
                    try
                    {
                        string contents = File.ReadAllText(path);
                        JArray json = JArray.Parse(contents);
                        int i = 0;
                        foreach (JObject scene in json)
                        {
                            string description = (string)scene.GetValue("description");
                            AddButton(i, description);
                            ++i;
                        }
                    }
                    catch
                    {

                    }

                    PlayerDLL.LoadScene(path);
                }
            }
        }

        private void BtnPlay_Click(object sender, RoutedEventArgs e)
        {
            _mBtnPlay.Background = new SolidColorBrush(Color.FromArgb(255, 0, 255, 0));
            _mBtnStop.Background = new SolidColorBrush(Colors.LightGray);

            if (!_mInitialized)
            {
                _mInitialized = true;
                int result = PlayerDLL.PlayerChromaInit();
                if (result == 0)
                {
                    _mTextStatus.Text = "STATUS: CHROMA [ACTIVE]";
                }
                else
                {
                    _mTextStatus.Text = string.Format("STATUS: CHROMA [ERROR] Code: {0}", result);
                }
            }
        }

        private void BtnStop_Click(object sender, RoutedEventArgs e)
        {
            _mBtnStop.Background = new SolidColorBrush(Color.FromArgb(255, 0, 255, 0));
            _mBtnPlay.Background = new SolidColorBrush(Colors.LightGray);
            _mTextStatus.Text = "STATUS: CHROMA [OFFLINE]";

            if (_mInitialized)
            {
                _mInitialized = false;
                int result = PlayerDLL.PlayerChromaUninit();
                if (result == 0)
                {
                    _mTextStatus.Text = "STATUS: CHROMA [OFFLINE]";
                }
                else
                {
                    _mTextStatus.Text = string.Format("STATUS: CHROMA [ERROR] Code: {0}", result);
                }
            }
        }

        private void BtnScene_Click(object sender, RoutedEventArgs e)
        {
            foreach (Button button in _mRefButtons)
            {
                if (button == sender)
                {
                    button.Background = new SolidColorBrush(Color.FromArgb(255, 0, 255, 0));
                    PlayerDLL.PlayerSelectScene((int)button.DataContext);
                }
                else
                {
                    button.Background = new SolidColorBrush(Colors.LightGray);
                }
            }
        }
    }
}
