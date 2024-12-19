using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using FilaMat.ViewModels;
using FilaMat.Views;

namespace FilaMat
{
    public partial class App : Application
    {
        public override void Initialize()
        {
            AvaloniaXamlLoader.Load(this);
        }

        public override void OnFrameworkInitializationCompleted()
        {
            if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
            {
                var main_window = new MainWindow
                {
                    DataContext = new MainWindowViewModel(),
                };

                main_window.RegistInteraction();

                desktop.MainWindow = main_window; 
            }

            base.OnFrameworkInitializationCompleted();
        }
    }
}