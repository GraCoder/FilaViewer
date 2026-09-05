using System.Linq;
using System.Threading.Tasks;
using System.Text.Json;
using Avalonia.Controls;
using Avalonia.Platform.Storage;
using MdlViewer.ViewModels;
using ReactiveUI;
using Avalonia.Threading;

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
            int id = _vkView.loadFile(storage_file[i].TryGetLocalPath());
            if (id == -1) 
                continue;
            _mdls.AddModel(id, name);
        }
    }

    public void RegistInteraction()
    {
        (DataContext as MainWindowViewModel).SelectFileInteraction.RegisterHandler(this.OpenFileHandle);
    }

    public void AddPrimitive(string name)
    {
        var cube = new AddPrimitive(name, 1);
        var ops = JsonSerializer.Serialize(cube, OperJsonContext.Default.AddPrimitive);
        int id = _vkView.handleCommand(ops);
        if (id == -1)
            return;
        _mdls.AddModel(id, name);
    }

    public void SelectModel(uint id)
    {
        Dispatcher.UIThread.Post(() => {
            var data = (ModelListViewModel)_mdls.DataContext;
            data.SelectNode(id);
        });
    }
}