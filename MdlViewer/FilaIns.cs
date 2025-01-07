public class FilaIns
{
    private static FilaIns instance = new FilaIns();

    private FilaIns() 
    {
        _select = this.SelectModel;
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

    global::IWin.Delegates.Action_uint _select;
    public void SelectModel(uint id) 
    {
    }
}