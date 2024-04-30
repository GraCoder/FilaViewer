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
using FilaView;

namespace FilaMat.Views
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

            _view = this.FindNameScope()?.Find<global::FilaMat.Views.EmbedView>("_view");
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

        FilaViewControl fila_control()
        {
            return ((VulkanWin)_view.Implementation).ViewCtl;
        }

        public void load_file(IStorageFile f)
        {
            var exts = new string[] {
                ".gltf", ".glb", ".obj", ".fbx"
            };

            string p = f.Path.ToString();
            var ext = Path.GetExtension(p);
            if (exts.Contains(ext))
            {
                fila_control().load_file(p);
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
        }
    }

    public interface INativeControl
    {
        IPlatformHandle CreateControl(IPlatformHandle parent, Func<IPlatformHandle> createDefault);
    }


    public class VulkanWin : INativeControl
    {
        FilaView.FilaViewControl _view_ctl;

        public FilaView.FilaViewControl ViewCtl
        {
            get { return _view_ctl; }
        }
        
        public IPlatformHandle CreateControl(IPlatformHandle parent, Func<IPlatformHandle> createDefault)
        {
            _view_ctl = new FilaView.FilaViewControl();
            return new Win32WindowControlHandle(_view_ctl.handle(), "HWND");
        }
    }




    //    public class EmbedSampleWin : INativeControl
    //    {
    //        private const string RichText =
    //            @"{\rtf1\ansi\ansicpg1251\deff0\nouicompat\deflang1049{\fonttbl{\f0\fnil\fcharset0 Calibri;}}
    //{\colortbl ;\red255\green0\blue0;\red0\green77\blue187;\red0\green176\blue80;\red155\green0\blue211;\red247\green150\blue70;\red75\green172\blue198;}
    //{\*\generator Riched20 6.3.9600}\viewkind4\uc1 
    //\pard\sa200\sl276\slmult1\f0\fs22\lang9 <PREFIX>I \i am\i0  a \cf1\b Rich Text \cf0\b0\fs24 control\cf2\fs28 !\cf3\fs32 !\cf4\fs36 !\cf1\fs40 !\cf5\fs44 !\cf6\fs48 !\cf0\fs44\par
    //}";

    //        public IPlatformHandle CreateControl(IPlatformHandle parent, Func<IPlatformHandle> createDefault)
    //        {
    //            WinApi.LoadLibrary("Msftedit.dll");
    //            var handle = WinApi.CreateWindowEx(0, "RICHEDIT50W",
    //                @"Rich Edit",
    //                0x800000 | 0x10000000 | 0x40000000 | 0x800000 | 0x10000 | 0x0004, 0, 0, 1, 1, parent.Handle,
    //                IntPtr.Zero, WinApi.GetModuleHandle(null), IntPtr.Zero);
    //            var st = new WinApi.SETTEXTEX { Codepage = 65001, Flags = 0x00000008 };
    //            var text = RichText.Replace("<PREFIX>", "");
    //            var bytes = Encoding.UTF8.GetBytes(text);
    //            WinApi.SendMessage(handle, 0x0400 + 97, ref st, bytes);
    //            return new Win32WindowControlHandle(handle, "HWND");
    //        }
    //    }

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
