using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MdlViewer.ViewModels;

internal class ModelNode
{
    private bool _visible = true;

    public int Id { get; }
    public string Name { get; }
    public bool Visible { 
        get { return _visible; }
        set {
            _visible = value;
            FilaIns.Instance.View?.ShowModel(Id, _visible);
        }
    }

    public ObservableCollection<ModelNode>? SubNodes { get; }

    public ModelNode(int id, string title)
    { 
        this.Id = id;
        this.Name = title; 
    }
}

internal class ModelListViewModel : ViewModelBase
{
    public ObservableCollection<ModelNode> Nodes { get; }

    public ModelListViewModel()
    {
        Nodes = new ObservableCollection<ModelNode>();

        Nodes.Add(new ModelNode(1, "1"));
        Nodes.Add(new ModelNode(2, "2"));
        Nodes.Add(new ModelNode(3, "3"));
        Nodes.Add(new ModelNode(4, "4"));
        Nodes.Add(new ModelNode(5, "5"));
        Nodes.Add(new ModelNode(6, "6"));
    }

    public void AddNode(int id, string name)
    {
        Nodes.Add(new ModelNode(id, name));
    }
}
