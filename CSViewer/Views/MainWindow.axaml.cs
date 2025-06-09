using System;
using System.Linq;
using System.Threading.Tasks;
using System.Text.Json;
using System.Text.Json.Nodes;
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

        FilaIns.Instance.MainWin = this;
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

    public void AddCube()
    {
        var cubeOps = new PrimitiveOperator(PrimitiveOperator.PrimitiveType.Cube);
        var ops = JsonSerializer.Serialize<PrimitiveOperator>(cubeOps, OperatorSerializeContext.Default.PrimitiveOperator);
        int id = FilaIns.Instance.Win.OperatorS(ops, ops.Length);
        if (id == -1)
            return;
        mdllist.AddModel(id, "Cube");
    }

    public void AddSphere()
    {
        var cubeOps = new PrimitiveOperator(PrimitiveOperator.PrimitiveType.Sphere);
        var ops = JsonSerializer.Serialize<PrimitiveOperator>(cubeOps, OperatorSerializeContext.Default.PrimitiveOperator);
        int id = FilaIns.Instance.Win.OperatorS(ops, ops.Length);
        if (id == -1)
            return;
        mdllist.AddModel(id, "Sphere");
    }

    public void SelectModel(uint id)
    {
        Dispatcher.UIThread.Post(() => {
            var data = (ModelListViewModel)mdllist.DataContext;
            data.SelectNode(id);
        });
    }
}