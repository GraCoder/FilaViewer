using System;
using System.Linq;
using System.Threading.Tasks;
using Avalonia.Controls;
using FilaMat.ViewModels;
using ReactiveUI;

namespace FilaMat.Views
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private async Task OpenFileHandle(InteractionContext<string?, string[]?> context)
        {
            var topl = TopLevel.GetTopLevel(this);

            var storage_file = await topl!.StorageProvider.OpenFilePickerAsync(
                new Avalonia.Platform.Storage.FilePickerOpenOptions
                {
                    AllowMultiple = false,
                    Title = "wtf"
                });
            context.SetOutput(storage_file?.Select(x=>x.Name).ToArray());

            if(storage_file.Count > 0 )
            {
                vkview.load_file(storage_file[0]);
            }
        }

        public void RegistInteraction()
        {
            (DataContext as MainWindowViewModel).SelectFileInteraction.RegisterHandler(this.OpenFileHandle);
        }
    }
}