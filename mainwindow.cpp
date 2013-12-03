#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QFileInfo>
#include "util.h"
#include "constants.h"
#include "NeuralConfig.h"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), ui(new Ui::MainWindow), NumDaysAhead_(1), network_(new QNetworkAccessManager(this)), timer_(new QTimer(this))
{
  ui->setupUi(this);

  config_ = std::make_shared<Config>(Util::loadIniFile("config.ini"));
  assets_ = std::make_shared<AssetsManager>(config_);
  neurnet_ = std::unique_ptr<NeuralNet>(new NeuralNet(config_.get()->neuralConfig()));

  assets_->setMoney(config_.get()->assetsConfig().Money_);
  assets_->setEnergy(config_.get()->assetsConfig().Energy_);
  assets_->setRealSystemPrice(config_.get()->assetsConfig().LastSysPrice_);




  agentController_ = std::unique_ptr<AgentController>(new AgentController(config_, neurnet_, assets_));


  QObject::connect(network_.get(), SIGNAL(finished(QNetworkReply*)), this, SLOT(onReply(QNetworkReply*)));
  QObject::connect(timer_.get(), SIGNAL(timeout()), this, SLOT(onTimerUpdate()));
  timer_.get()->start(config_.get()->miscConfig().TimerInterval_);


  updateAll();


}

MainWindow::~MainWindow()
{
  delete ui;
}

//void MainWindow::on_btnAddSet_clicked()
//{
//  const QString path = QFileDialog::getOpenFileName(this, tr("Open CSV File"), QDir::currentPath(), tr("CSV Files (*.csv)"));
//  QFile* raw = new QFile(path);
//  QFileInfo fileInfo(raw->fileName());
//  if(!rawDataFiles_.contains(fileInfo.fileName()))
//  {
//    rawDataFiles_.insert(raw->fileName(), raw);
//    //ui->cmbDatasetRaw->insertItem(MainWindow::NextId_++, fileInfo.fileName());
//  }

//  qDebug() << path;
//}

void MainWindow::updateAll()
{
  ui->lblCurrMoney->setText(QString::number(assets_->money()));
  ui->lblCurrEnergy->setText(QString::number(assets_->energy()));

  QString filenames = "Elspot Prices_2011_Daily_NOK.csv\nElspot Prices_2012_Daily_NOK.csv\nElspot Prices_2013_Daily_NOK.csv\n";
  ui->lblDatasetFile->setText(filenames);

  fetchLatestSpotPrice();
}

void MainWindow::fetchLatestSpotPrice()
{
  network_.get()->get(QNetworkRequest(QUrl(config_.get()->parseConfig().UrlSpot_)));
}

void MainWindow::fetchFreshPrices()
{
  network_.get()->get(QNetworkRequest(QUrl(config_.get()->parseConfig().UrlPrices2013Daily_)));
}

//void MainWindow::on_btnBuyAmount_clicked()
//{
//  //assets_->buyEnergy(ui->txtBuyAmount->text().toDouble());
//  updateAll();
//}

void MainWindow::on_btnPredict_clicked()
{
//  QList<QStringList> dataMatrix = Util::loadCSVFile("Data/Elspot Prices_2013_Daily_NOK.csv", ',');
//  auto prices = Util::extractSystemPriceDaily(dataMatrix);

//  QDateTime fromdate;
//  fromdate.setDate(QDate(2013, 10 , 1));
//  neurnet_->estimateNextPeriod(prices, fromdate, config_.get()->neuralConfig().DayAheadLong_, NumDaysAhead_);
}

void MainWindow::onReply(QNetworkReply *reply)
{
  if(reply->error() == QNetworkReply::NoError)
  {
    if(reply->url().toString().compare(config_.get()->parseConfig().UrlSpot_) == 0)
    {
      // We're parsing the latest spotprice in NOK from the Nordpool site.
      double spotprice = Util::parseNordpoolSpotpriceNOK(QString(reply->readAll()));
      agentController_->setSystemPrice(spotprice);
      ui->lblSpotPrice->setText(QString::number(assets_.get()->realSystemPrice()));
    }
    else if(reply->url().toString().compare(config_.get()->parseConfig().UrlPrices2013Daily_) == 0)
    {
      // We're grabbing the latest price data for this year, parsing it to a readable format.
      auto a = Util::parseXLS_daily(QString(reply->readAll()));
      agentController_.get()->setLatestDailyPrices(a);
    }
  }
  else
  {
    qDebug() << reply->errorString();
    Logger::get().append(reply->errorString(), true);
  }
  reply->deleteLater();
}

void MainWindow::onTimerUpdate()
{
  // If agent has performed its duties for the day, it will go to sleep, halting any further operation.
  // We keep polling him until it is a new calendarday. The agent should 'wake up' by then and clear previous flags.
  if(agentController_->agentSleepingUntilNextDay())
    if(!agentController_->tryToWake())
      return;

  QDateTime now = QDateTime::currentDateTime();
  // Once the window for any of these opens, there's a number of flags that needs to be set to true, before we can go to the next step.
  // This is to ensure that everything happens in order, because we cannot trust an internet connection.
  if(!agentController_->hasPredictedAndMadeOrder() && now.time().hour() >= config_.get()->miscConfig().Time_Predict_Price_.hour())
  {
    fetchLatestSpotPrice();
    fetchFreshPrices();

    // If preliminary stuff is done, start the process of predicting.
    if(agentController_->isFreshSystemPrice() && agentController_->isFreshPriceData())
      agentController_->predictPriceAhead();


  }
  else if(agentController_->hasPredictedAndMadeOrder() && now.time().hour() >= config_.get()->miscConfig().Time_Complete_Transaction_.hour())
  {
    // The time should be well past the updated daily price by now. It is time to complete the "frozen" transaction.
    agentController_->completeRemainingTransactions();
  }
}








void MainWindow::on_btnTrainData_clicked()
{
  QList<QString> files = {"Data/Elspot Prices_2011_Daily_NOK.csv", "Data/Elspot Prices_2012_Daily_NOK.csv", "Data/Elspot Prices_2013_Daily_NOK.csv"};
  QList<QStringList> dataMatrix = Util::loadCSVFiles(files, ',');
  auto prices = Util::extractSystemPriceDaily(dataMatrix);
  agentController_.get()->createAndTrainSet("2011_2013_daily_NOK", prices);

}

void MainWindow::on_btnStartAgent_clicked()
{
    network_.get()->get(QNetworkRequest(QUrl(config_.get()->parseConfig().UrlPrices2013Daily_)));
}
