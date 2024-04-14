#include "FilaView.h"

#include "TWin.h"

using namespace FilaView;

FilaViewCtl::FilaViewCtl() { 
  _win = TWin::create(nullptr);
  _win->exec();
}

FilaViewCtl::~FilaViewCtl() {
  if (_win)
    delete _win;
}

System::IntPtr FilaView::FilaViewCtl::handle() { return System::IntPtr((long long)_win->handle()); }
