using System.Runtime.InteropServices;

namespace FilaView;

public unsafe partial struct TWin
{
    [NativeTypeName("uint32_t")]
    public uint _width;

    [NativeTypeName("uint32_t")]
    public uint _height;

    [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "??_DTWin@@QEAAXXZ", ExactSpelling = true)]
    public static extern void Dispose(TWin* pThis);

    [DllImport("FilaView", CallingConvention = CallingConvention.Cdecl, EntryPoint = "?create@TWin@@SAPEAV1@PEAV1@_N@Z", ExactSpelling = true)]
    public static extern TWin* create(TWin* win = null, [NativeTypeName("bool")] byte with_border = 0);

    [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?handle@TWin@@QEAA_KXZ", ExactSpelling = true)]
    [return: NativeTypeName("uint64_t")]
    public static extern ulong handle(TWin* pThis);

    [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?resize@TWin@@QEAAXHH@Z", ExactSpelling = true)]
    public static extern void resize(TWin* pThis, int w, int h);

    [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?exec@TWin@@QEAAX_N@Z", ExactSpelling = true)]
    public static extern void exec(TWin* pThis, [NativeTypeName("bool")] byte thead = 1);

    [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?view@TWin@@QEAAPEAVTView@@XZ", ExactSpelling = true)]
    public static extern TView* view(TWin* pThis);

    [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?load_model@TWin@@QEAAXPEBD@Z", ExactSpelling = true)]
    public static extern void load_model(TWin* pThis, [NativeTypeName("const char *")] sbyte* file);
}
