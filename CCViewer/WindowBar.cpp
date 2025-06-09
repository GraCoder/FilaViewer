#include <cmath>
#include <algorithm>

#include <QtGui/QPainter>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMenuBar>

#include "WindowBar.h"

#include <Windows.h>

WindowBtn::WindowBtn(BtnType btnType)
  : _type(btnType)
{
  setMinimumWidth(45);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void WindowBtn::paintEvent(QPaintEvent *ev)
{
  QPainter painter(this);
  if (underMouse()) {
    if (_type == BT_Close) {
      if (_pressing)
        painter.fillRect(rect(), QColor(0xf1707a));
      else
        painter.fillRect(rect(), QColor(0xe81123));
    } else {
      if (_pressing)
        painter.fillRect(rect(), QColor(0, 0, 0, 60));
      else
        painter.fillRect(rect(), QColor(0, 0, 0, 25));
    }
  }

  QPen pen = painter.pen();
  pen.setColor(Qt::black);

  if (_type == BT_Close) {
    if (underMouse()) {
      pen.setColor(Qt::white);
    }
    pen.setWidthF(1.25);
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing);
    float sz = std::min<int>(width(), height()) * 0.33;
    float xoft = (width() - sz) / 2.0;
    float yoft = (height() - sz) / 2.0;
    painter.drawLine(QPoint(qRound(xoft), qRound(yoft)), QPoint(qRound(xoft + sz), qRound(yoft + sz)));
    painter.drawLine(QPoint(qRound(xoft), qRound(yoft + sz)), QPointF(qRound(xoft + sz), qRound(yoft)));
    return;
  } else if (_type == BT_Max) {
    pen.setWidthF(1);
    painter.setPen(pen);
    float sz = std::min<int>(width(), height()) * 0.34;
    int xoft = round((width() - sz) / 2.0);
    int yoft = round((height() - sz) / 2.0);
    if (topLevelWidget()->isMaximized()) {
      constexpr float f1 = 0.2083333;
      constexpr float f2 = 0.75;
      painter.drawRect(xoft, yoft + sz * f1, f2 * sz, f2 * sz);
      const float f3 = xoft + f1 * sz;
      painter.drawLine(f3, yoft + sz * f1, f3, yoft);
      painter.drawLine(f3, yoft - 1, xoft + sz, yoft - 1);
      painter.drawLine(xoft + sz, yoft, xoft + sz, yoft + sz * f2);
      painter.drawLine(xoft + sz, yoft + sz * f2, xoft + f2 * sz, yoft + sz * f2);
    } else {
      painter.drawRect(xoft, yoft, sz, sz);
    }
    return;
  } else if (_type == BT_Min) {
    pen.setWidthF(1);
    painter.setPen(pen);
    int y = height() / 2.0;
    float sz = std::min<int>(width(), height()) * 0.33;
    int xoft = round((width() - sz) / 2.0);
    painter.drawLine(xoft, y, xoft + sz, y);
    return;
  }

  QPushButton::paintEvent(ev);
}

void WindowBtn::enterEvent(QEvent *ev)
{
  Base::enterEvent(ev);
  update();
}

void WindowBtn::leaveEvent(QEvent *ev)
{
  Base::leaveEvent(ev);
  update();
}

void WindowBtn::mousePressEvent(QMouseEvent *ev)
{
  Base::mousePressEvent(ev);
  _pressing = true;
  update();
}

void WindowBtn::mouseReleaseEvent(QMouseEvent *ev)
{
  Base::mouseReleaseEvent(ev);
  _pressing = false;

  if (topLevelWidget()->isHidden() || topLevelWidget()->isMinimized()) {
    setAttribute(Qt::WA_UnderMouse, false);
  } else {
    auto pt = mapFromGlobal(QCursor::pos());
    setAttribute(Qt::WA_UnderMouse, hitButton(pt));
  }

  update();
}

//------------------------------------------------------------------------------

WindowBar::WindowBar()
{
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 1, 0, 0);
  layout->addStretch();

  layout->setSpacing(0);
  _min = new WindowBtn(WindowBtn::BT_Min);
  layout->addWidget(_min);

  _max = new WindowBtn(WindowBtn::BT_Max);
  layout->addWidget(_max);

  _close = new WindowBtn(WindowBtn::BT_Close);
  layout->addWidget(_close);
}

void WindowBar::init()
{
  auto wgt = topLevelWidget();
  connect(_min, SIGNAL(clicked(bool)), wgt, SLOT(showMinimized()));
  connect(_close, SIGNAL(clicked(bool)), wgt, SLOT(close()));
  connect(_max, SIGNAL(clicked(bool)), this, SLOT(sltMax()));
}

void WindowBar::setMenuBar(QWidget *menuBar)
{
  _menuBar = menuBar;
  _menuBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
  static_cast<QHBoxLayout *>(layout())->insertWidget(0, _menuBar);
}

void WindowBar::sltMax()
{
  auto wgt = topLevelWidget();
  if (wgt->isMaximized())
    wgt->showNormal();
  else
    wgt->showMaximized();
}

bool WindowBar::inButtons(const QPoint &pt)
{
  bool ret = false;
  auto cpt = mapFromParent(pt);
  if (_close)
    ret |= _close->geometry().contains(cpt);
  if (_min)
    ret |= _min->geometry().contains(cpt);
  if (_max)
    ret |= _max->geometry().contains(cpt);
  return ret;
}

int WindowBar::titlePart(const QPoint &pt)
{
  auto cpt = mapFromParent(pt);
  if (_close && _close->geometry().contains(cpt))
    return HTCLOSE;
  if (_min && _min->geometry().contains(cpt))
    return HTMINBUTTON;
  if (_max && _max->geometry().contains(cpt))
    return HTMAXBUTTON;

  if (_menuBar && _menuBar->geometry().contains(cpt))
    return HTCLIENT;

  return HTCAPTION;
}

void WindowBar::paintEvent(QPaintEvent *ev)
{
  Base::paintEvent(ev);

  // QPainter painter(this);
  // painter.fillRect(rect(), Qt::red);
}

void WindowBar::mouseMoveEvent(QMouseEvent *ev)
{
  update();
}
