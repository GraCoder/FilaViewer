#include <QCloseEvent>
#include <QHBoxLayout>
#include <QSize>
#include <QMenu>
#include <QMenuBar>

#include "MainWindow.h"
#include "WindowBar.h"

#include "FilaView.h"

MainWindow::MainWindow()
{
  setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
  setMinimumSize(sizeHint());

  create_widget();
}

MainWindow::~MainWindow()
{
}

QSize MainWindow::sizeHint() const
{
  return QSize(800, 600);
}

void MainWindow::create_widget()
{
  auto layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);

  _view = new FilaView;

  {
    auto bar = new WindowBar;
    layout->addWidget(bar);
    setTitleBar(bar);

    auto mbar = new QMenuBar;
    bar->setMenuBar(mbar);
    {
      {
        auto m = mbar->addMenu("file");
        m->addAction("open", _view, SLOT(sltOpenFile()));
        m->addAction("save", _view, SLOT(sltSaveFile()));
      }
      {
        auto m = mbar->addMenu("edit");
      }
    }
  }

  layout->addWidget(_view, 1);

 // layout->addStretch();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  _view->close();
  Base::closeEvent(event);
}