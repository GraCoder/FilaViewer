using System.Threading.Tasks;
using System.Windows.Input;
using ReactiveUI;
using ReactiveUI.Primitives;

namespace MdlViewer.ViewModels
{
    public class MainWindowViewModel : ViewModelBase
    {

        private string? _file;

        public ICommand OpenFileCommand { get; }

        private readonly Interaction<string?, string[]?> _select_file_interaction;
        public Interaction<string?, string[]?> SelectFileInteraction => this._select_file_interaction;

        public MainWindowViewModel()
        {
            _select_file_interaction = new Interaction<string?, string[]?>();
            OpenFileCommand = ReactiveCommand.CreateFromTask(SelectFile);
        }


        public string? SelectedFile
        {
            get { return _file; }
            set { this.RaiseAndSetIfChanged(ref _file, value); }
        }

        private async Task SelectFile()
        {
            var files = await _select_file_interaction.Handle("").ToTask();
            if (files is { Length: > 0 })
                SelectedFile = files[0];
        }
    }
}
