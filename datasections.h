#ifndef DATASECTIONS_H
#define DATASECTIONS_H

#include "basedatasection.h"
#include "constants.h"
#include <QFile>
#include <QFileInfo>
#include "util.h"
#include <QDebug>

class DataMiscSection : public BaseDataSection
{
public:
  DataMiscSection() : BaseDataSection("misc") { }
  virtual ~DataMiscSection()
  {
    for(auto itr = DatasetFiles_.begin(); itr != DatasetFiles_.end(); ++itr)
      if(itr.value() != nullptr) delete itr.value();

    DatasetFiles_.clear();
  }

  virtual void setValues(const QMap<QString, QString> &sectionMap)
  {
    TimerInterval_ = value("timer_interval", sectionMap).toUInt(); assert(TimerInterval_ > 0);
    Time_Complete_Transaction_ = QTime::fromString(value("time_complete_transaction", sectionMap), Constants::TimeFormat);
    Time_Predict_Price_ = QTime::fromString(value("time_predict_price", sectionMap), Constants::TimeFormat);
    QList<QString> dataSetpaths = value("dataSetFiles", sectionMap).trimmed().split(Constants::Separator);
    for(QString filepath : dataSetpaths)
      addDataset(filepath);
    DefaultTrainSetName_ = value("defaultTrainSetName", sectionMap);
  }

  virtual QString toString() const
  {
    QString sectionData(QString("[%1]\n").arg(sectionName_));
    appendConfigLine(sectionData, "timer_interval", QString::number(TimerInterval_));
    appendConfigLine(sectionData, "time_complete_transaction", Time_Complete_Transaction_.toString(Constants::TimeFormat));
    appendConfigLine(sectionData, "time_predict_price", Time_Predict_Price_.toString(Constants::TimeFormat));
    QString datasetFileNames;
    for(auto itr = DatasetFiles_.begin(); itr != DatasetFiles_.end(); ++itr)
      datasetFileNames.append(itr.value()->fileName() + Constants::Separator);
    datasetFileNames = datasetFileNames.remove(datasetFileNames.length()-1, 1); // remove last sep
    appendConfigLine(sectionData, "dataSetFiles", datasetFileNames);
    appendConfigLine(sectionData, "defaultTrainSetName", DefaultTrainSetName_);
    return sectionData;
  }


  void addDataset(const QString& filename)
  {
    if(filename.isEmpty())
      return;

    if(!DatasetFiles_.contains(filename))
    {
      QFile* file = new QFile(filename);
      QFileInfo fileInfo(file->fileName());
      DatasetFiles_.insert(fileInfo.fileName(), file);
    }
  }

  void removeDataset(const QString& filename)
  {
    if(filename.isEmpty()) { Logger::get().append("MainWindow.RemoveDataset: filename was not found. Filename: " + filename, true); return; }
    if(!DatasetFiles_.contains(filename)) { Logger::get().append("MainWindow.RemoveDataset: filename was not found. Filename: " + filename, true); return; }
    DatasetFiles_.remove(filename);
  }

  unsigned int TimerInterval_;
  QTime Time_Complete_Transaction_;
  QTime Time_Predict_Price_;
  QMap<QString, QFile*> DatasetFiles_;
  QString DefaultTrainSetName_;

};

class DataAssetSection : public BaseDataSection
{
public:
  DataAssetSection() : BaseDataSection("assets") { }

  virtual void setValues(const QMap<QString, QString> &sectionMap)
  {
    money_ = value("money", sectionMap).toDouble(); assert(money_ >= 0.0);
    energy_ = value("energy", sectionMap).toDouble(); assert(energy_ >= 0.0);
    lastSysPrice_ = value("lastSysPrice", sectionMap).toDouble(); assert(lastSysPrice_ > 1.0);
    orderNumber_ = value("currentOrderNumber", sectionMap).toUInt();
  }

  virtual QString toString() const
  {
    QString sectionData(QString("[%1]\n").arg(sectionName_));
    appendConfigLine(sectionData, "money", Util::numberFormat(money_));
    appendConfigLine(sectionData, "energy", Util::numberFormat(energy_));
    appendConfigLine(sectionData, "lastSysPrice", Util::numberFormat(lastSysPrice_));
    appendConfigLine(sectionData, "currentOrderNumber", QString::number(orderNumber_));
    return sectionData;
  }

  double money() const {return money_; }
  double energy() const { return energy_; }
  double lastSysPrice() const { return lastSysPrice_; }
  unsigned int orderNumber() const { return orderNumber_; }
  void setMoney(double money) { money_ = money; }
  void setEnergy(double energy) { energy_ = energy; }
  void setLastSysPrice(double lastSysPrice) { lastSysPrice_ = lastSysPrice; }
  void setOrderNumber(unsigned int orderNumber) { orderNumber_ = orderNumber; }

private:
  double money_;
  double energy_;
  double lastSysPrice_;
  unsigned int orderNumber_;
};

class DataParseSection : public BaseDataSection
{
public:
  DataParseSection() : BaseDataSection("parse") { }

  virtual void setValues(const QMap<QString, QString> &sectionMap)
  {
    UrlSpot_ = value("url_spot", sectionMap);
    UrlPrices2013Daily_ = value("url_prices2013daily", sectionMap);
  }

  virtual QString toString() const
  {
    QString sectionData(QString("[%1]\n").arg(sectionName_));
    appendConfigLine(sectionData, "url_spot", UrlSpot_);
    appendConfigLine(sectionData, "url_prices2013daily", UrlPrices2013Daily_);
    return sectionData;
  }

  QString UrlSpot_;
  QString UrlPrices2013Daily_;
};

class DataAgentSection : public BaseDataSection
{
public:
  DataAgentSection() : BaseDataSection("agentinfo") { }

  virtual void setValues(const QMap<QString, QString> &sectionMap)
  {
    ClientEmailAddr_ = value("clientEmailAddr", sectionMap);
    ClientName_ = value("clientName", sectionMap);
    SmtpPassw_ = value("smtppassw", sectionMap);
    SmtpClient_ = value("smtpclient", sectionMap);
    SmtpPort_ = value("smtpport",sectionMap).toUInt();
    AgentId_ = value("agent_id", sectionMap).toUInt();
    SendEmail_ = value("sendEmail", sectionMap).toUInt() > 0;
    receiversEmail_= value("receiverEmail", sectionMap).trimmed().split(Constants::Separator);
    MaxMoneySpend_ = value("maxMoneySpend",sectionMap).toDouble();
    MaxEnergySell_ = value("maxEnergySell", sectionMap).toDouble();
    CurrentDay_ = value("currentDay", sectionMap).toUInt();
    HasMadeOrder_ = value("hasMadeOrder", sectionMap).toUInt() > 0 ? true : false;
    HasCompletedTransaction_ = value("hasCompletedTransaction", sectionMap).toUInt() > 0 ? true : false;
    IsAgentSleeping_ = value("isAgentSleeping", sectionMap).toUInt() > 0 ? true : false;
    ResetFlagsOnStartup_ = value("resetFlagsOnStartup", sectionMap).toUInt() > 0 ? true : false;
  }

  virtual QString toString() const
  {
    QString sectionData(QString("[%1]\n").arg(sectionName_));
    appendConfigLine(sectionData, "clientEmailAddr", ClientEmailAddr_);
    appendConfigLine(sectionData, "clientName", ClientName_);
    appendConfigLine(sectionData, "smtppassw", SmtpPassw_);
    appendConfigLine(sectionData, "smtpclient", SmtpClient_);
    appendConfigLine(sectionData, "smtpport", QString::number(SmtpPort_));
    appendConfigLine(sectionData, "agent_id", QString::number(AgentId_));
    appendConfigLine(sectionData, "sendEmail", QString::number(SendEmail_ > 0 ? 1 : 0));
    QString emailReceivers;
    for(QString recAddr : receiversEmail_)
      emailReceivers.append(recAddr + Constants::Separator);
    emailReceivers = emailReceivers.remove(emailReceivers.length()-1, 1); // remove last sep
    appendConfigLine(sectionData, "receiverEmail", emailReceivers);
    appendConfigLine(sectionData, "maxMoneySpend", Util::numberFormat(MaxMoneySpend_));
    appendConfigLine(sectionData, "maxEnergySell", Util::numberFormat(MaxEnergySell_));
    appendConfigLine(sectionData, "currentDay", QString::number(CurrentDay_));
    appendConfigLine(sectionData, "hasMadeOrder", QString::number(HasMadeOrder_ > 0 ? 1 : 0));
    appendConfigLine(sectionData, "hasCompletedTransaction", QString::number(HasCompletedTransaction_ > 0 ? 1 : 0));
    appendConfigLine(sectionData, "isAgentSleeping", QString::number(IsAgentSleeping_ > 0 ? 1 : 0));
    appendConfigLine(sectionData, "resetFlagsOnStartup", QString::number(ResetFlagsOnStartup_ > 0 ? 1 : 0));
    return sectionData;
  }

  QString ClientEmailAddr_;
  QString ClientName_;
  QList<QString> receiversEmail_;

  QString SmtpPassw_;
  QString SmtpClient_;
  unsigned int SmtpPort_;
  unsigned int AgentId_;
  bool SendEmail_;
  double MaxMoneySpend_;
  double MaxEnergySell_;
  unsigned int CurrentDay_;
  bool HasMadeOrder_;
  bool HasCompletedTransaction_;
  bool IsAgentSleeping_;
  bool ResetFlagsOnStartup_;
};

class DataNeuralSection : public BaseDataSection
{
public:
  DataNeuralSection() : BaseDataSection("ann") { }

  virtual void setValues(const QMap<QString, QString> &sectionMap)
  {
    Nameset_ = value("setname", sectionMap); assert(!Nameset_.isEmpty());
    MaxPriceExpected_ = value( "maxPriceExpected", sectionMap).toDouble(); assert(MaxPriceExpected_ > 0.0);
    DayAheadShort_ = value("dayAheadShort", sectionMap).toUInt(); assert(DayAheadShort_ > 0);
    DayAheadLong_ = value("dayAheadLong", sectionMap).toUInt(); assert(DayAheadLong_ > 0);
    DayPeriod_ = value("dayPeriod", sectionMap).toUInt(); assert(DayPeriod_ > 0);
    NumLayers_ = value("numLayers", sectionMap).toUInt(); assert(NumLayers_ > 0);
    NumNeuronsHidden_ = value("numNeuronsHidden", sectionMap).toUInt(); assert(NumNeuronsHidden_ > 0);
    NumOutputs_ = value("numOutputs", sectionMap).toUInt(); assert(NumOutputs_ > 0);
    DesiredError_ = value("DesiredError", sectionMap).toDouble(); assert(DesiredError_ > 10e-8);
    MaxEpochs_ = value("maxEpochs", sectionMap).toUInt(); assert(MaxEpochs_ > 0);
    EpochsBetweenReports_ = value("epochsBetweenReports", sectionMap).toUInt(); assert(EpochsBetweenReports_ > 0);
  }

  virtual QString toString() const
  {
    QString sectionData(QString("[%1]\n").arg(sectionName_));
    appendConfigLine(sectionData, "setname", Nameset_);
    appendConfigLine(sectionData, "maxPriceExpected", Util::numberFormat(MaxPriceExpected_));
    appendConfigLine(sectionData, "dayAheadShort", QString::number(DayAheadShort_));
    appendConfigLine(sectionData, "dayAheadLong", QString::number(DayAheadLong_));
    appendConfigLine(sectionData, "dayPeriod", QString::number(DayPeriod_));
    appendConfigLine(sectionData, "numLayers", QString::number(NumLayers_));
    appendConfigLine(sectionData, "numNeuronsHidden", QString::number(NumNeuronsHidden_));
    appendConfigLine(sectionData, "numOutputs", QString::number(NumOutputs_));
    appendConfigLine(sectionData, "DesiredError", Util::numberFormat(DesiredError_));
    appendConfigLine(sectionData, "maxEpochs", QString::number(MaxEpochs_));
    appendConfigLine(sectionData, "epochsBetweenReports", QString::number(EpochsBetweenReports_));
    return sectionData;
  }

  QString Nameset_;
  double MaxPriceExpected_;
  unsigned int DayAheadShort_;
  unsigned int DayAheadLong_;
  unsigned int DayPeriod_;
  unsigned int NumLayers_;
  unsigned int NumNeuronsHidden_;
  unsigned int NumOutputs_;
  double DesiredError_;
  unsigned int MaxEpochs_;
  unsigned int EpochsBetweenReports_;
};

#endif // DATASECTIONS_H



