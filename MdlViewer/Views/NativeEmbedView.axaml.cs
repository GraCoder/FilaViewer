using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Platform;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Platform;
using Avalonia.Platform.Storage;

namespace MdlViewer.Views
{
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

        public int load_file(string p)
        {
            var exts = new string[] {
                ".gltf", ".glb", ".obj", ".fbx"
            };

            var ext = Path.GetExtension(p);
            if (!exts.Contains(ext))
                return -1;

            var vk_win = (VulkanWin)(_view as EmbedView).Implementation;
            return vk_win.load_file(p);
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
        FilaView.IWin _win = null;

        public IPlatformHandle CreateControl(IPlatformHandle parent, Func<IPlatformHandle> createDefault)
        {
#if true 
            _win = FilaView.IWin.Create(null, false);
            _win.CreateOperators();
            _win.Exec(true);

            FilaIns.Instance.Win = _win;
            FilaIns.Instance.View = _win.View(0);

            return new Win32WindowControlHandle((IntPtr)_win.Handle, "HWND");
#else
            return null;
#endif
        }

        public void DestroyControl()
        {
            if (_win != null) {
                FilaView.IWin.Destroy(_win);
            }
        }

        public int load_file(String file)
        {
            if (_win == null)
                return -1;
            
            return _win.LoadModel(file, 10);
        }
    }

    internal class Win32WindowControlHandle : PlatformHandle, INativeControlHostDestroyableControlHandle
    {
        public Win32WindowControlHandle(IntPtr handle, string descriptor) : base(handle, descriptor)
        {
        }

        public void Destroy()
        {
            _ = WinApi.DestroyWindow(Handle);
        }
    }
}
