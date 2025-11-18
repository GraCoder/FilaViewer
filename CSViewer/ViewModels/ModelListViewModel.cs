using ReactiveUI;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive;
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
            //FilaIns.Instance.View?.ShowModel(Id, _visible);
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
    }

    public void AddNode(int id, string name)
    {
        Nodes.Add(new ModelNode(id, name));
    }

    ModelNode? _sel_node = null;
    public ModelNode? SelectedNode { 
        get { return _sel_node; } 
        set { 
            this.RaiseAndSetIfChanged(ref _sel_node, value);
        } 
    }

    public void SelectNode(uint id)
    {
        for (int i = 0; i < Nodes.Count; i++) 
        {
            if (Nodes[i].Id == id) {
                SelectedNode = Nodes[i];
                return;
            }
        }
        SelectedNode = null;
    }
}
