#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QFile>
#include "assetsmanager.h"
#include <memory>
#include "neuralnet.h"
#include "Config.h"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void on_btnAddSet_clicked();

  void on_btnBuyAmount_clicked();

  void on_btnTrain_clicked();

  void on_btnPredict_clicked();

private:
  Ui::MainWindow *ui;
  const unsigned int NumDaysAhead_;
  QMap<QString, QFile*> rawDataFiles_;
  unsigned int NextId_;
  std::unique_ptr<AssetsManager> assets_;
  std::unique_ptr<NeuralNet> neurnet_;
  std::shared_ptr<Config> config_;


  void updateAll();

};

#endif // MAINWINDOW_H
