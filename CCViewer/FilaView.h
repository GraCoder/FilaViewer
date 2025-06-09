#pragma once

#include <QWidget>

class TWin;
class FilaView : public QWidget {
  Q_OBJECT
public:
  FilaView();

protected:
  void showEvent(QShowEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void closeEvent(QCloseEvent *event) override; 
  
public slots:
  void sltOpenFile();
 
private:
  void moveWindow();
  
private:
  TWin *_win = nullptr;
};