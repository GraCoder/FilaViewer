#include "FilaViewControl.h"

#include "FilaView/TWin.h"

#include <Windows.h>
#include <string>

using namespace System;
using namespace FilaView;

FilaViewControl::FilaViewControl() { 
  _win = TWin::create(nullptr, false);
  _win->exec(true);
}

FilaViewControl::~FilaViewControl() {
  if (_win)
    delete _win;
}

System::IntPtr FilaView::FilaViewControl::handle()
{
  return System::IntPtr((long long)_win->handle());
}

void FilaView::FilaViewControl::resize_control(System::IntPtr width, System::IntPtr height)
{
  //MoveWindow((HWND)_win->handle(), 0, 0, width.ToInt32(), height.ToInt32(), true);
  //width.ToInt32();
  //LPARAM p = height.ToInt32();
  //p = (p << 32) | width.ToInt32();
  //SendMessage((HWND)_win->handle(), WM_SIZE, 0, p);

  //SetWindowPos((HWND)_win->handle(), NULL, 0, 0, width.ToInt32(), height.ToInt32(), 0);

  //char chtmp[512];
  //sprintf(chtmp, "%d, %d\n", width.ToInt32(), height.ToInt32());
  //MessageBox(NULL, chtmp, "", MB_OK);
}

void FilaView::FilaViewControl::load_file(System::String ^file)
{
  using namespace Runtime::InteropServices;
  auto s = (const char *)(Marshal::StringToHGlobalAnsi(file)).ToPointer();
  std::string sf = s;
  Marshal::FreeHGlobal(IntPtr((void*)s));
  _win->load_model(sf.c_str());
}
