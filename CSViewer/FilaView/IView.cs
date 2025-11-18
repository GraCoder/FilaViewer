using System.Runtime.CompilerServices;

namespace fv;

public unsafe partial struct IView : IView.Interface
{
    public void** lpVtbl;

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(0)]
    public void showEntity(int id, bool show)
    {
        ((delegate* unmanaged[Thiscall]<IView*, int, bool, void>)(lpVtbl[0]))((IView*)Unsafe.AsPointer(ref this), id, show);
    }

    public interface Interface
    {
        [VtblIndex(0)]
        void showEntity(int id, bool show);
    }

    public partial struct Vtbl<TSelf>
        where TSelf : unmanaged, Interface
    {
        [NativeTypeName("void (int, bool)")]
        public delegate* unmanaged[Thiscall]<TSelf*, int, bool, void> showEntity;
    }
}
