using System.Linq;
using System.Reactive.Linq;
using System.Threading.Tasks;
using System.Windows.Input;
using Avalonia.Controls;
using ReactiveUI;

namespace MdlViewer.ViewModels
{
    public class MainWindowViewModel : ViewModelBase
    {

        private string? _file;

        public ICommand OpenFileCommand { get; }

        private readonly Interaction<string?, string[]?> _select_file_interaction;

        public MainWindowViewModel()
        {
            _select_file_interaction = new Interaction<string?, string[]?>();
            OpenFileCommand = ReactiveCommand.CreateFromTask(SelectFile);
        }


        public string ? SelectedFile
        {
            get { return _file; }
            set { this.RaiseAndSetIfChanged(ref _file, value); }
        }

        private async Task SelectFile()
        {
            var files = await _select_file_interaction.Handle("");
            if(files.Count() > 0)
                _file = files.First();
        }

        public Interaction<string?, string[]?> SelectFileInteraction => this._select_file_interaction;
    }
}
