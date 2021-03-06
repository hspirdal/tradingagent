#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QFileInfo>
#include "util.h"
#include "constants.h"
#include "constants.h"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), ui(new Ui::MainWindow), network_(new QNetworkAccessManager(this)), timer_(new QTimer(this)), agentRunning_(false)
{
  ui->setupUi(this);

  config_ = std::make_shared<Config>();
  //QString logfile, const QString& clientEmailAddr, const QString& clientSenderName, const QString& clientGmailPassw, QList<QString> recipients
  log_ = std::make_shared<ApplicationLogger>("log.log", config_->agentInfoConfig().ClientEmailAddr_, config_->agentInfoConfig().ClientName_,
            config_->agentInfoConfig().SmtpPassw_, config_->agentInfoConfig().receiversEmail_);


  transLog_ = std::make_shared<TransactionLogger>("transaction.log", config_->agentInfoConfig().ClientEmailAddr_,
  config_->agentInfoConfig().ClientName_,config_->agentInfoConfig().SmtpPassw_, config_->agentInfoConfig().receiversEmail_, config_);



  assets_ = std::make_shared<AssetsManager>(config_, transLog_);
  neurnet_ = std::make_shared<NeuralNet>(config_, log_, ui);

  assets_->setMoney(config_->assetsConfig().money());
  assets_->setEnergy(config_->assetsConfig().energy());
  assets_->setRealSystemPrice(config_->assetsConfig().lastSysPrice());
  agentController_ = std::unique_ptr<AgentController>(new AgentController(config_, log_, neurnet_, assets_));


  QObject::connect(network_.get(), SIGNAL(finished(QNetworkReply*)), this, SLOT(onReply(QNetworkReply*)));
  QObject::connect(timer_.get(), SIGNAL(timeout()), this, SLOT(onTimerUpdate()));
  QObject::connect(log_.get(), SIGNAL(valueChanged(QString)), this, SLOT(appendWindowLog(QString)));
  QObject::connect(transLog_.get(), SIGNAL(valueChanged(QString)), this, SLOT(appendWindowLog(QString)));

  ui->txtbxOverviewLog->setReadOnly(true);
  ui->pbTrainingProgress->setMinimum(0);
  ui->pbTrainingProgress->setMaximum(config_->neuralConfig().MaxEpochs_);
  ui->pbTrainingProgress->hide();
  ui->txtTrainingStatus->setReadOnly(true);

  ui->cmbDatasets->clear();
  ui->txtTrainerFileName->setText(config_->miscConfig().DefaultTrainSetName_);

  reloadDatasetFiles();


  fetchLatestSpotPrice();
  fetchFreshPrices();
  updateAll();

  // TODO: add mails on own line in config.
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::updateAll()
{
  ui->lblCurrMoney->setText(QString::number(assets_->money(), 'G', 8));
  ui->lblCurrEnergy->setText(QString::number(assets_->energy(), 'G', 8));

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

void MainWindow::fetchFreshPricesFromDisk()
{

  //QString content = Util::readFile("Data/Elspot Prices_2013_Daily_NOK.xls");
  //auto a = Util::parseXLS_daily(content);
  auto raw = Util::loadCSVFile("Data/Elspot Prices_2013_Daily_NOK.csv", ',');
  auto a = Util::extractSystemPriceDaily(raw);
  agentController_.get()->setLatestDailyPrices(a);
  Logger::get().append("fetched fresh prices from disk", true);
}



void MainWindow::onReply(QNetworkReply *reply)
{
  if(reply->error() == QNetworkReply::NoError)
  {
    if(reply->url().toString().compare(config_.get()->parseConfig().UrlSpot_) == 0)
    {
      // We're parsing the latest spotprice in NOK from the Nordpool site.
      double spotprice = Util::parseNordpoolSpotpriceNOK(QString(reply->readAll()));
      if(spotprice <= Constants::ApproxZeroDouble)
        return;
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
    // Error downloading http://www.nordpoolspot.com/PageFiles/9383/Elspot Prices_2013_Daily_NOK.xls - server replied: File not found
    const QString err = "Error downloading  " + config_->parseConfig().UrlPrices2013Daily_;
    if(reply->errorString().compare(err) == 0)
    {
      // I've detected that the file is down for a small timeperiod at around 07:30, but otherwise seems to stay up.
      // We have plenty of time to try and parse it during the night, so lets just have it try again until it gets time-critical.
      if(QDateTime::currentDateTime().time().hour() >= config_->miscConfig().Time_Predict_Price_.hour())
        fetchFreshPricesFromDisk();
      Logger::get().append("MainWindow.OnReply: Error downloading 2013 prices from Nordpool.", true);
    }
    else
    {
      qDebug() << reply->errorString();
      Logger::get().append(reply->errorString(), true);
    }
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
  if(!agentController_->hasMadeOrder())
  {
    if(!agentController_->isFreshSystemPrice())
      fetchLatestSpotPrice();
    if(!agentController_->isFreshPriceData())
      fetchFreshPrices();

    // If preliminary stuff is done, start the process of predicting.
    if(agentController_->isFreshSystemPrice() && agentController_->isFreshPriceData() && now.time().hour() >= config_->miscConfig().Time_Predict_Price_.hour())
      agentController_->predictPriceAhead();


  }
  else if(agentController_->hasMadeOrder() && now.time().hour() >= config_.get()->miscConfig().Time_Complete_Transaction_.hour())
  {
    // The time should be well past the updated daily price by now. It is time to complete the "frozen" transaction.
    agentController_->completeRemainingTransactions();
  }
  updateAll();
}


void MainWindow::on_btnTrainData_clicked()
{
  if(config_->miscConfig().DatasetFiles_.count() <= 0)
  {
    log_->append("MainWindow.TrainData: No dataset loaded.", true);
    return;
  }
  //QList<QString> files = {"Data/Elspot Prices_2011_Daily_NOK.csv", "Data/Elspot Prices_2012_Daily_NOK.csv", "Data/Elspot Prices_2013_Daily_NOK.csv"};
  //QList<QStringList> dataMatrix = Util::loadCSVFile("Data/Elspot Prices_2013_Daily_NOK.csv", ',');
  QList<QStringList> dataMatrix = Util::loadCSVFiles(config_->miscConfig().DatasetFiles_.values(), ',');
  auto prices = Util::extractSystemPriceDaily(dataMatrix);

  agentController_.get()->createAndTrainSet(ui->txtTrainerFileName->text(), prices);

}

void MainWindow::appendWindowLog(const QString& log)
{
  ui->txtbxOverviewLog->append(QDateTime::currentDateTime().toString() + ": " + log);
}

void MainWindow::on_btnStartAgent_clicked()
{
    agentRunning_ = !agentRunning_;
    if(agentRunning_)
    {
      timer_->start(config_.get()->miscConfig().TimerInterval_);
      ui->btnStartAgent->setText("Stop Agent");
      ui->lblStatus->setText("Running");
      ui->lblStatus->setStyleSheet("QLabel { color : green }");
      appendWindowLog("Started running the agent");
    }
    else
    {
      timer_->stop();
      ui->btnStartAgent->setText("Start Agent");
      ui->lblStatus->setText("Not Running");
      ui->lblStatus->setStyleSheet("QLabel { color : red }");
      appendWindowLog("Stopped running the agent.");
    }
}

void MainWindow::on_btnRefreshSpot_clicked()
{
    fetchLatestSpotPrice();
}

void MainWindow::on_btnAddDataset_clicked()
{
  const QString path = QFileDialog::getOpenFileName(this, tr("Open CSV File"), QDir::currentPath(), tr("CSV Files (*.csv)"));
  config_->miscConfig().addDataset(path);
  config_->save();

  // Check if something was added or not.
  if(config_->miscConfig().DatasetFiles_.count() == ui->cmbDatasets->count())
    return;

  reloadDatasetFiles();
}

void MainWindow::on_btnRemoveDataset_clicked()
{
  const QString filename = ui->cmbDatasets->currentText();
  config_->miscConfig().removeDataset(filename);
  config_->save();

  // Check if something was deleted or not.
  if( config_->miscConfig().DatasetFiles_.count() == ui->cmbDatasets->count())
    return;

  reloadDatasetFiles();
}

void MainWindow::reloadDatasetFiles()
{
  const int currIndex = ui->cmbDatasets->currentIndex() >= 0 ? ui->cmbDatasets->currentIndex() : 0;
  ui->cmbDatasets->clear();
  auto files = config_->miscConfig().DatasetFiles_.values();
  for(int i = 0; i < files.count(); i++)
    ui->cmbDatasets->insertItem(i, Util::actualFileName(*files[i]));

  ui->cmbDatasets->setCurrentIndex(currIndex);
}
