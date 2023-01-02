using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using huestream;
using HueStreamExampleWPF;

namespace Example_Effects_wpf_csharp
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Initialized(object sender, EventArgs e)
        {
            var hue = new Hue("HueExampleCSharp", "PC", "PersistenceEncryptionKey");
            var dc = (HueObservable) DataContext;
            dc.SetModel(hue, Dispatcher);
        }

        private void MainWindow_OnClosed(object sender, EventArgs e)
        {
            var dc = (HueObservable)DataContext;
            dc.ShutDown();
        }
    }
}
