#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QFileInfo>
#include "util.h"
#include "constants.h"
#include "NeuralConfig.h"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), ui(new Ui::MainWindow), NumDaysAhead_(1), assets_(new AssetsManager()), network_(new QNetworkAccessManager(this))
{
  ui->setupUi(this);

  config_ = std::make_shared<Config>(Util::loadIniFile("config.ini"));
  neurnet_ = std::unique_ptr<NeuralNet>(new NeuralNet(config_.get()->neuralConfig()));

  assets_->setMoney(config_.get()->assetsConfig().Money_);
  assets_->setEnergy(config_.get()->assetsConfig().Energy_);
  assets_->setRealSystemPrice(config_.get()->assetsConfig().LastSysPrice_);
  updateAll();

  QList<QString> files = {"Data/Elspot Prices_2011_Daily_NOK.csv", "Data/Elspot Prices_2012_Daily_NOK.csv", "Data/Elspot Prices_2013_Daily_NOK.csv"};
  QList<QStringList> dataMatrix = Util::loadCSVFiles(files, ',');
  auto prices = Util::extractSystemPriceDaily(dataMatrix);
  neurnet_->createTrainSet("2011_2013_daily_NOK", prices);

  //connect(nam_, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReply(QNetworkReply*)));

  QObject::connect(network_.get(), SIGNAL(finished(QNetworkReply*)), this, SLOT(onReply(QNetworkReply*)));


  network_.get()->get(QNetworkRequest(QUrl("http://www.nordpoolspot.com/")));
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_btnAddSet_clicked()
{
  const QString path = QFileDialog::getOpenFileName(this, tr("Open CSV File"), QDir::currentPath(), tr("CSV Files (*.csv)"));
  QFile* raw = new QFile(path);
  QFileInfo fileInfo(raw->fileName());
  if(!rawDataFiles_.contains(fileInfo.fileName()))
  {
    rawDataFiles_.insert(raw->fileName(), raw);
    ui->cmbDatasetRaw->insertItem(MainWindow::NextId_++, fileInfo.fileName());
  }

  qDebug() << path;
}

void MainWindow::updateAll()
{
  ui->lblCurrMoney->setText(QString::number(assets_->money()));
  ui->lblCurrEnergy->setText(QString::number(assets_->energy()));

}

void MainWindow::on_btnBuyAmount_clicked()
{
  assets_->buyEnergy(ui->txtBuyAmount->text().toDouble());
  updateAll();
}

void MainWindow::on_btnTrain_clicked()
{
  qDebug() << "started training..";
  neurnet_->train();
}

void MainWindow::on_btnPredict_clicked()
{
  QList<QStringList> dataMatrix = Util::loadCSVFile("Data/Elspot Prices_2013_Daily_NOK.csv", ',');
  auto prices = Util::extractSystemPriceDaily(dataMatrix);

  QDateTime fromdate;
  fromdate.setDate(QDate(2013, 10 , 1));
  neurnet_->estimateNextPeriod(prices, fromdate, config_.get()->neuralConfig().DayAheadLong_, NumDaysAhead_);
}

void MainWindow::onReply(QNetworkReply *reply)
{
  if(reply->error() == QNetworkReply::NoError)
  {
    double spotprice = Util::parseNordpoolSpotpriceNOK(QString(reply->readAll()));
    ui->lblSpotPrice->setText(QString::number(spotprice));
  }
  else
  {
    qDebug() << reply->errorString();
    Logger::get().append(reply->errorString(), true);
  }
  reply->deleteLater();
}







