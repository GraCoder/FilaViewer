#pragma once

class TWin;

namespace FilaView {

public ref class FilaViewControl {
public:

  FilaViewControl();

  ~FilaViewControl();

  System::IntPtr handle();

  void resize_control(System::IntPtr width, System::IntPtr height);

private:

  TWin *_win = nullptr;

};

} // namespace FilaView
