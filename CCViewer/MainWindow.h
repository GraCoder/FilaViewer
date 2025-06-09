#pragma once

#include <memory>
#include <QtGui/qframeless.h>
#include <QtGui/qsystemtrayicon.h>

class TPanel;
class FilaView;
class MainWindow : public QFrameless {
  Q_OBJECT
  typedef QFrameless Base;
public:
  MainWindow();

  ~MainWindow();

private:

  QSize sizeHint() const override;

  void create_widget();

private slots:
  //void slt_quit();
  //void slt_proxy(bool);
  //void slt_force(bool);
  //void slt_host(bool);
  //void activate(QSystemTrayIcon::ActivationReason);

private:
  void closeEvent(QCloseEvent *event);

private:
  FilaView *_view = nullptr;
};