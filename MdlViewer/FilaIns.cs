using MdlViewer.Views;

public class FilaIns
{
    private static FilaIns instance = new FilaIns();

    global::IWin.Delegates.Action_uint _select;

    private FilaIns() 
    {
    }

    public static FilaIns Instance {
        get { return instance; }
    }

    IWin.IWin ?_win = null;
    public IWin.IWin? Win { 
        get { return _win; } 
        set {
            _win = value; 
            _win!.RegistSelect(_select);
        } 
    }
    public IWin.IView? View { get; set; }

    MainWindow? _main = null;
    public MainWindow? MainWin { 
        get { return _main; }
        set {
            _main = value;
            _select = _main.SelectModel;
        } 
    }
}