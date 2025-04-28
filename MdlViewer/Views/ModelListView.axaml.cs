using Avalonia.Controls;
using MdlViewer.ViewModels;

namespace MdlViewer.Views;

public partial class ModelListView : UserControl
{
    public ModelListView()
    {
        InitializeComponent();

        DataContext = new ModelListViewModel();
    }

    public void AddModel(int id, string name)
    {
        var model = (ModelListViewModel)DataContext;
        model.AddNode(id, name);
    }
}