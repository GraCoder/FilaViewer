#pragma once

#include <cstdint>

#include <QWidget>

namespace fv {
class TWin;
}

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
  fv::TWin *_win = nullptr;
  uint64_t _nativeWin = 0;
};
