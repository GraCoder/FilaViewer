using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace fv
{
    public unsafe partial struct IWin
    {
        public void** lpVtbl;

        [DllImport("FilaView", CallingConvention = CallingConvention.Cdecl, EntryPoint = "?create@IWin@@SAPEAV1@PEAV1@_N@Z", ExactSpelling = true)]
        public static extern IWin* create(IWin* win = null, [NativeTypeName("bool")] byte with_border = true);

        [DllImport("FilaView", CallingConvention = CallingConvention.Cdecl, EntryPoint = "?destroy@IWin@@SAXPEAV1@@Z", ExactSpelling = true)]
        public static extern void destroy(IWin* win);

        [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?loadModel@IWin@@QEAAHPEBDM@Z", ExactSpelling = true)]
        public static extern int loadModel(IWin* pThis, [NativeTypeName("const char *")] sbyte* file, float sz = 1);

        [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?exeOperator@IWin@@QEAAHPEBDH@Z", ExactSpelling = true)]
        public static extern int exeOperator(IWin* pThis, [NativeTypeName("const char *")] sbyte* ops, int len);

        [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?createOperators@IWin@@QEAAXXZ", ExactSpelling = true)]
        public static extern void createOperators(IWin* pThis);

        [DllImport("FilaView", CallingConvention = CallingConvention.ThisCall, EntryPoint = "?registPick@IWin@@QEAAXP6AXI@Z@Z", ExactSpelling = true)]
        public static extern void registPick(IWin* pThis, [NativeTypeName("void (*)(unsigned int)")] delegate* unmanaged[Cdecl]<uint, void> fun);

        [return: NativeTypeName("uint64_t")]
        public ulong handle()
        {
            return ((delegate* unmanaged[Thiscall]<IWin*, ulong>)(lpVtbl[0]))((IWin*)Unsafe.AsPointer(ref this));
        }

        public void exec([NativeTypeName("bool")] byte thread)
        {
            ((delegate* unmanaged[Thiscall]<IWin*, byte, void>)(lpVtbl[1]))((IWin*)Unsafe.AsPointer(ref this), thread);
        }

        public void setupGui()
        {
            ((delegate* unmanaged[Thiscall]<IWin*, void>)(lpVtbl[2]))((IWin*)Unsafe.AsPointer(ref this));
        }

        //public IView* view(int id = 0)
        //{
        //    return ((delegate* unmanaged[Thiscall]<IWin*, int, IView*>)(lpVtbl[3]))((IWin*)Unsafe.AsPointer(ref this), id);
        //}
    }
}
