#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  auto ft = QApplication::font();
  ft.setFamily("Noto Sans SC");
  ft.setPointSize(12);
  QApplication::setFont(ft);

  MainWindow window;
  window.show();

  return app.exec();
}