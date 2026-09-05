using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Platform;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Platform;
using MdlViewer.ViewModels;
using ReactiveUI;
using ReactiveUI.Primitives;

namespace MdlViewer.Views;

public partial class NativeEmbedView : UserControl
{
    public NativeEmbedView()
    {
        InitializeComponent();
    }

    private void InitializeComponent()
    {
        AvaloniaXamlLoader.Load(this);

        _view = this.FindNameScope()?.Find<global::MdlViewer.Views.EmbedView>("_view");

        MessageBus.Current.Listen<ModelVisibleMsg>().Subscribe(x =>
        {
            showModel(x.Id, x.Visible);
        });
    }

    public async void ShowPopupDelay(object sender, RoutedEventArgs args)
    {
        await Task.Delay(3000);
        ShowPopup(sender, args);
    }

    public void ShowPopup(object sender, RoutedEventArgs args)
    {
        new ContextMenu()
        {
            Items =
            {
                new MenuItem() { Header = "Test" }, new MenuItem() { Header = "Test" }
            }
        }.Open((Control)sender);
    }

    protected override void OnPropertyChanged(AvaloniaPropertyChangedEventArgs change)
    {
        base.OnPropertyChanged(change);

        if (change.Property == BoundsProperty)
        {
        }
    }

    public int loadFile(string file)
    {
        var exts = new string[] {
            ".gltf", ".glb", ".obj", ".fbx"
        };

        var ext = Path.GetExtension(file);
        if (!exts.Contains(ext))
            return -1;

        unsafe
        {
            var vkWin = (VulkanWin)(_view as EmbedView).Implementation;
            var bytes = System.Text.Encoding.UTF8.GetBytes(file);
            fixed (byte* p = bytes)
            {
                return fv.IWin.loadModel(vkWin.Win, (sbyte*)p, bytes.Length);
            }
        }
    }

    public int handleCommand(string ops)
    {
        unsafe
        {
            var vkWin = (VulkanWin)(_view as EmbedView).Implementation;
            var bytes = System.Text.Encoding.UTF8.GetBytes(ops);
            fixed (byte* p = bytes)
            {
                return vkWin.Win->handleCommand((sbyte*)p, bytes.Length);
            }
        }
    }

    public void showModel(int id, bool visible)
    {
        unsafe
        {
            var vkWin = (VulkanWin)(_view as EmbedView).Implementation;
            fv.IWin.showModel(vkWin.Win, id, visible);
        }
    }
}

public class EmbedView : NativeControlHost
{
    public INativeControl? Implementation { get; set; }

    public EmbedView()
    {
        Implementation = new VulkanWin();
    }

    protected override IPlatformHandle CreateNativeControlCore(IPlatformHandle parent)
    {
        return Implementation?.CreateControl(parent, () => base.CreateNativeControlCore(parent))
            ?? base.CreateNativeControlCore(parent);
    }

    protected override void DestroyNativeControlCore(IPlatformHandle control)
    {
        base.DestroyNativeControlCore(control);
        ((VulkanWin)Implementation)?.DestroyControl();
    }
}

public interface INativeControl
{
    IPlatformHandle CreateControl(IPlatformHandle parent, Func<IPlatformHandle> createDefault);
}


public unsafe class VulkanWin : INativeControl
{
    public fv.IWin* Win { get; private set; }

    public IPlatformHandle CreateControl(IPlatformHandle parent, Func<IPlatformHandle> createDefault)
    {
#if true 
        Win = fv.IWin.create(null, false);
        var winId = Win->exec(true);
        return new Win32WindowControlHandle((IntPtr)winId, "HWND");
#else
        return null;
#endif
    }

    public void DestroyControl()
    {
        if (Win != null)
            fv.IWin.destroy(Win);
        Win = null;
    }
}

internal class Win32WindowControlHandle : PlatformHandle, INativeControlHostDestroyableControlHandle
{
    public Win32WindowControlHandle(IntPtr handle, string descriptor) : base(handle, descriptor)
    {
    }

    public void Destroy()
    {
        // The SDL execution thread owns and destroys this HWND.
    }
}
