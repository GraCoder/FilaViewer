using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace fv;

public unsafe partial struct IWin : IWin.Interface
{
    public void** lpVtbl;

    [DllImport("FilaView", CallingConvention = CallingConvention.Cdecl, EntryPoint = "?create@IWin@@SAPEAV1@PEAV1@_N@Z", ExactSpelling = true)]
    public static extern IWin* create(IWin* win = null, bool with_border = true);

    [DllImport("FilaView", CallingConvention = CallingConvention.Cdecl, EntryPoint = "?destroy@IWin@@SAXPEAV1@@Z", ExactSpelling = true)]
    public static extern void destroy(IWin* win);

    [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?loadModel@IWin@@QEAAHPEBDM@Z", ExactSpelling = true)]
    public static extern int loadModel(IWin* pThis, [NativeTypeName("const char *")] sbyte* file, float sz = 1);

    [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?showModel@IWin@@QEAAXH_N@Z", ExactSpelling = true)]
    public static extern void showModel(IWin* pThis, int param0, bool param1);

    [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?createOperators@IWin@@QEAAXXZ", ExactSpelling = true)]
    public static extern void createOperators(IWin* pThis);

    [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?registPick@IWin@@QEAAXP6AXI@Z@Z", ExactSpelling = true)]
    public static extern void registPick(IWin* pThis, [NativeTypeName("void (*)(unsigned int)")] delegate* unmanaged[Cdecl]<uint, void> fun);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(0)]
    [return: NativeTypeName("uint64_t")]
    public ulong winId()
    {
        return ((delegate* unmanaged[Thiscall]<IWin*, ulong>)(lpVtbl[0]))((IWin*)Unsafe.AsPointer(ref this));
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(1)]
    public void exec(bool thread)
    {
        ((delegate* unmanaged[Thiscall]<IWin*, bool, void>)(lpVtbl[1]))((IWin*)Unsafe.AsPointer(ref this), thread);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(2)]
    public void setupGui()
    {
        ((delegate* unmanaged[Thiscall]<IWin*, void>)(lpVtbl[2]))((IWin*)Unsafe.AsPointer(ref this));
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(3)]
    public IView* view(int id = 0)
    {
        return ((delegate* unmanaged[Thiscall]<IWin*, int, IView*>)(lpVtbl[3]))((IWin*)Unsafe.AsPointer(ref this), id);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(4)]
    public int handleCommand([NativeTypeName("const char *")] sbyte* ops, int len)
    {
        return ((delegate* unmanaged[Thiscall]<IWin*, sbyte*, int, int>)(lpVtbl[4]))((IWin*)Unsafe.AsPointer(ref this), ops, len);
    }

    public interface Interface
    {
        [VtblIndex(0)]
        [return: NativeTypeName("uint64_t")]
        ulong winId();

        [VtblIndex(1)]
        void exec(bool thread);

        [VtblIndex(2)]
        void setupGui();

        [VtblIndex(3)]
        IView* view(int id = 0);

        [VtblIndex(4)]
        int handleCommand([NativeTypeName("const char *")] sbyte* ops, int len);
    }

    public partial struct Vtbl<TSelf>
        where TSelf : unmanaged, Interface
    {
        [NativeTypeName("uint64_t ()")]
        public delegate* unmanaged[Thiscall]<TSelf*, ulong> winId;

        [NativeTypeName("void (bool)")]
        public delegate* unmanaged[Thiscall]<TSelf*, bool, void> exec;

        [NativeTypeName("void ()")]
        public delegate* unmanaged[Thiscall]<TSelf*, void> setupGui;

        [NativeTypeName("IView *(int)")]
        public delegate* unmanaged[Thiscall]<TSelf*, int, IView*> view;

        [NativeTypeName("int (const char *, int)")]
        public delegate* unmanaged[Thiscall]<TSelf*, sbyte*, int, int> handleCommand;
    }
}
