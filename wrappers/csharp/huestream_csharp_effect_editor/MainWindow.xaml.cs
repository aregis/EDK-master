using System.IO;
using System.Windows;
using System.Windows.Input;
using huestream;


namespace HueEffectEditor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow
    {
        private LightScript _lightScript;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void button_SaveClick(object sender, RoutedEventArgs e)
        {
            var ser = _lightScript.SerializeText();
            File.WriteAllText("obj.json", ser);
        }

        private void OnBtnLoadClick(object sender, RoutedEventArgs e)
        {
            _lightScript = new LightScript();
            _lightScript.DeserializeText(File.ReadAllText("obj.json"));
            var manager = (PropertyGridManager)DataContext;
            manager.ActiveObject = _lightScript;
        }

        private void OnBtnNewClick(object sender, RoutedEventArgs e)
        {
            _lightScript = new LightScript();
            var manager = (PropertyGridManager)DataContext;
            manager.ActiveObject = _lightScript;
        }

        private void OnPropertyGridMouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (_propertyGrid.SelectedProperty == null)
                return;

            var manager = (PropertyGridManager)DataContext;
            manager.SelectProperty((string)_propertyGrid.SelectedProperty);
        }

        private void BtnBackClick(object sender, RoutedEventArgs e)
        {
            var manager = (PropertyGridManager)DataContext;
            manager.Back();
        }

        private void BtnNewClick(object sender, RoutedEventArgs e)
        {
            var manager = (PropertyGridManager)DataContext;
            manager.NewItem((string)_propertyGrid.SelectedProperty);
        }


        private void OnSelectedPropertyItemChanged(object sender, RoutedPropertyChangedEventArgs<Xceed.Wpf.Toolkit.PropertyGrid.PropertyItemBase> e)
        {
            if (_propertyGrid.SelectedProperty == null)
                return;
            var manager = (PropertyGridManager)DataContext;
            manager.HighProperty((string)_propertyGrid.SelectedProperty);

        }

    }
}
