#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QFile>

#include "transactionlogger.h"
#include "assetsmanager.h"
#include <memory>
#include "neuralnet.h"
#include "Config.h"
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QTimer>
#include "agentcontroller.h"
#include "applicationlogger.h"
#include "transactionlogger.h"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

public slots:
  void appendWindowLog(const QString& log);

private slots:
  void onReply(QNetworkReply* reply);
  void onTimerUpdate();
  void on_btnTrainData_clicked();
  void on_btnStartAgent_clicked();
  void on_btnRefreshSpot_clicked();

private:
  Ui::MainWindow *ui;
  QMap<QString, QFile*> rawDataFiles_;
  unsigned int NextId_;
  std::shared_ptr<AssetsManager> assets_;
  std::shared_ptr<NeuralNet> neurnet_;
  std::shared_ptr<Config> config_;
  std::shared_ptr<ApplicationLogger> log_;
  std::shared_ptr<TransactionLogger> transLog_;
  std::unique_ptr<QNetworkAccessManager> network_;
  std::unique_ptr<QTimer> timer_;
  std::unique_ptr<AgentController> agentController_;
  bool agentRunning_;

  void updateAll();
  void fetchLatestSpotPrice();
  void fetchFreshPrices();
  void fetchFreshPricesFromDisk();

};

#endif // MAINWINDOW_H
