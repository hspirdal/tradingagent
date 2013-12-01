#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QFileInfo>
#include "util.h"
#include "constants.h"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), ui(new Ui::MainWindow), NumDaysAhead_(1), assets_(new AssetsManager()), neurnet_(new NeuralNet("testset", NumDaysAhead_))
{
  ui->setupUi(this);

  assets_->setMoney(1000000);
  assets_->setEnergy(1000000);
  assets_->setRealSystemPrice(113.0);
  updateAll();

  QList<QString> files = {"Data/Elspot Prices_2011_Daily_NOK.csv", "Data/Elspot Prices_2012_Daily_NOK.csv", "Data/Elspot Prices_2013_Daily_NOK.csv"};
  QList<QStringList> dataMatrix = Util::loadCSVFiles(files, ',');
  auto prices = Util::extractSystemPriceDaily(dataMatrix);
  neurnet_->createTrainSet("2011_2013_daily_NOK", prices);
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
  neurnet_->estimateNextPeriod(prices, fromdate, NumDaysAhead_);
}







