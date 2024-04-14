#pragma once

class TWin;

namespace FilaView {

public
ref class FilaViewCtl {
public:

  FilaViewCtl();

  ~FilaViewCtl();

  System::IntPtr handle();

private:

  TWin *_win = nullptr;

};

} // namespace FilaView
