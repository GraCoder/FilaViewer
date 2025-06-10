#include <C:\Users\t\dev\FilaViewer\FilaView\IWin.h>
#include <C:\Users\t\dev\FilaViewer\FilaView\node\Node.h>
#include <new>

class IWin& (IWin::*_0)(class IWin&&) = &IWin::operator=;
class IWin_IWinIWin : public IWin { public: IWin_IWinIWin(): IWin() {}; unsigned long long handle() { return {}; } void exec(bool) {} void setup_gui() {} IView* view(int) { return {}; }  };
extern "C" __declspec(dllexport) void IWin_IWin(void* __instance) { ::new (__instance) IWin_IWinIWin(); }
class IWin_IWin___1__S_IWinIWin : public IWin { public: IWin_IWin___1__S_IWinIWin(const IWin& _0): IWin(_0) {}; unsigned long long handle() { return {}; } void exec(bool) {} void setup_gui() {} IView* view(int) { return {}; }  };
extern "C" __declspec(dllexport) void IWin_IWin___1__S_IWin(void* __instance, const IWin& _0) { ::new (__instance) IWin_IWin___1__S_IWinIWin(_0); }
class IView& (IView::*_1)(class IView&&) = &IView::operator=;
class IView_IViewIView : public IView { public: IView_IViewIView(): IView() {}; void show_model(int, bool) {}  };
extern "C" __declspec(dllexport) void IView_IView(void* __instance) { ::new (__instance) IView_IViewIView(); }
class IView_IView___1__S_IViewIView : public IView { public: IView_IView___1__S_IViewIView(const IView& _0): IView(_0) {}; void show_model(int, bool) {}  };
extern "C" __declspec(dllexport) void IView_IView___1__S_IView(void* __instance, const IView& _0) { ::new (__instance) IView_IView___1__S_IViewIView(_0); }
unsigned int (Node::*_2)() = &Node::id;
const tg::Tvec3<float>& (Node::*_3)() = &Node::translation;
void (Node::*_4)(const tg::Tvec3<float>&) = &Node::set_translation;
const tg::Tvec3<float>& (Node::*_5)() = &Node::rotation;
void (Node::*_6)(const tg::Tvec3<float>&) = &Node::set_rotation;
const tg::Tvec3<float>& (Node::*_7)() = &Node::scale;
void (Node::*_8)(const tg::Tvec3<float>&) = &Node::set_scale;
struct RDNode* (Node::*_9)() = &Node::rdNode;
class Node& (Node::*_10)(const class Node&) = &Node::operator=;
