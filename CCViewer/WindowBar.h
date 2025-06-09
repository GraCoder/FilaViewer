#pragma once

#include <QtGui/qframeless.h>
#include <QtGui/qpushbutton.h>

class WindowBtn : public QPushButton {
  Q_OBJECT
  typedef QPushButton Base;

public:
  enum BtnType { BT_Close, BT_Max, BT_Min };

  WindowBtn(BtnType btnType);

protected:
  void paintEvent(QPaintEvent *ev) override;

  void enterEvent(QEvent *) override;
  void leaveEvent(QEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;

private:
  BtnType _type;
  bool _pressing = false;
};

class WindowBar : public QFramelessBar {
  Q_OBJECT
  typedef QFramelessBar Base;

public:
  WindowBar();

  void init();
  void setMenuBar(QWidget *menuBar);

protected slots:

  void sltMax();

protected:

  bool inButtons(const QPoint &pt) override;
  int titlePart(const QPoint &pt) override;

  void paintEvent(QPaintEvent *ev) override;
  void mouseMoveEvent(QMouseEvent *) override;

private:
  QWidget *_menuBar = nullptr;

  WindowBtn *_close = nullptr;
  WindowBtn *_min = nullptr;
  WindowBtn *_max = nullptr;
};