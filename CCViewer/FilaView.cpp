#include <QWidget>
#include <QFileDialog>

#include "FilaView/TWin.h"
#include "FilaView.h"

#include <Windows.h>

FilaView::FilaView()
{
  // setAttribute(Qt::WA_OpaquePaintEvent);
  // setAttribute(Qt::WA_NoSystemBackground);
}

void FilaView::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);

  if (!_win) {
    _win = dynamic_cast<TWin *>(IWin::create(nullptr, false));
    _win->exec(true);

    SetParent(HWND(_win->handle()), (HWND)winId());
  }

  moveWindow();
}

void FilaView::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  if (_win) {
    moveWindow();
  }
}

void FilaView::closeEvent(QCloseEvent *event)
{
  QWidget::closeEvent(event);
  if (_win) {
    IWin::destroy(_win);
    _win = nullptr;
  }
}

void FilaView::sltOpenFile() 
{
  auto f = QFileDialog::getOpenFileName();
  if (f.isEmpty())
    return;

  _win->load_model(f.toLocal8Bit().data(), 2.0f);
}

void FilaView::moveWindow()
{
  HWND hwnd = (HWND)_win->handle();
  QPoint pt;
  // auto pt = mapToGlobal(QPoint(0, 0));
  MoveWindow(hwnd, pt.x(), pt.y(), width(), height(), TRUE);
}
