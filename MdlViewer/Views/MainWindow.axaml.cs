using System;
using System.Linq;
using System.Threading.Tasks;
using Avalonia.Controls;
using Avalonia.Platform.Storage;
using MdlViewer.ViewModels;
using ReactiveUI;

namespace MdlViewer.Views;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
    }

    private async Task OpenFileHandle(IInteractionContext<string?, string[]?> context)
    {
        var topl = TopLevel.GetTopLevel(this);

        var storage_file = await topl!.StorageProvider.OpenFilePickerAsync(
            new Avalonia.Platform.Storage.FilePickerOpenOptions
            {
                AllowMultiple = false,
                Title = "wtf"
            });
        context.SetOutput(storage_file?.Select(x=>x.Name).ToArray());

        for( var i = 0; i < storage_file.Count; i++ ) {
            var name = storage_file[i].Name;
            int id = vkview.load_file(storage_file[i].TryGetLocalPath());
            if (id == -1) 
                continue;
            mdllist.AddModel(id, name);
        }
    }

    public void RegistInteraction()
    {
        (DataContext as MainWindowViewModel).SelectFileInteraction.RegisterHandler(this.OpenFileHandle);
    }
}