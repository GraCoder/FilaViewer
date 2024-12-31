public class FilaIns
{
    private static FilaIns instance = new FilaIns();

    private FilaIns() { }

    public static FilaIns Instance {
        get { return instance; }
    }

    public IWin.IWin? Win { get; set; }
    public IWin.IView? View { get; set; }
}