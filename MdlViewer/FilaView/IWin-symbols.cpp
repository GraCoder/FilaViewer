#include <C:\Users\t\dev\FilaViewer\FilaView\IWin.h>
#include <new>

class IWin& (IWin::*_0)(class IWin&&) = &IWin::operator=;
class IWin_IWinIWin : public IWin { public: IWin_IWinIWin(): IWin() {}; unsigned long long handle() { return {}; } void exec(bool) {} IView* view(int) { return {}; } int load_model(const char*, float) { return {}; }  };
extern "C" __declspec(dllexport) void IWin_IWin(void* __instance) { ::new (__instance) IWin_IWinIWin(); }
class IWin_IWin___1__S_IWinIWin : public IWin { public: IWin_IWin___1__S_IWinIWin(const IWin& _0): IWin(_0) {}; unsigned long long handle() { return {}; } void exec(bool) {} IView* view(int) { return {}; } int load_model(const char*, float) { return {}; }  };
extern "C" __declspec(dllexport) void IWin_IWin___1__S_IWin(void* __instance, const IWin& _0) { ::new (__instance) IWin_IWin___1__S_IWinIWin(_0); }
class IView& (IView::*_1)(class IView&&) = &IView::operator=;
class IView_IViewIView : public IView { public: IView_IViewIView(): IView() {}; void show_model(int, bool) {}  };
extern "C" __declspec(dllexport) void IView_IView(void* __instance) { ::new (__instance) IView_IViewIView(); }
class IView_IView___1__S_IViewIView : public IView { public: IView_IView___1__S_IViewIView(const IView& _0): IView(_0) {}; void show_model(int, bool) {}  };
extern "C" __declspec(dllexport) void IView_IView___1__S_IView(void* __instance, const IView& _0) { ::new (__instance) IView_IView___1__S_IViewIView(_0); }
