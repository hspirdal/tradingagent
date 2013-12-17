#ifndef DATASECTIONS_H
#define DATASECTIONS_H

#include "basedatasection.h"
#include "constants.h"
#include <QFile>
#include <QFileInfo>
#include "util.h"

class DataMiscSection : public BaseDataSection
{
public:
  DataMiscSection(QString sectionName, QMap<QString, QString>& dataMap) : BaseDataSection(sectionName, dataMap) { }
  virtual ~DataMiscSection()
  {
    for(auto itr = DatasetFiles_.begin(); itr != DatasetFiles_.end(); ++itr)
      if(itr.value() != nullptr) delete itr.value();

    DatasetFiles_.clear();
  }

  virtual void load()
  {
    TimerInterval_ = value("timer_interval").toUInt(); assert(TimerInterval_ > 0);
    Time_Complete_Transaction_ = QTime::fromString(value("time_complete_transaction"), Constants::TimeFormat);
    Time_Predict_Price_ = QTime::fromString(value("time_predict_price"), Constants::TimeFormat);
    QList<QString> dataSetpaths = value("dataSetFiles").trimmed().split(Constants::Separator);
    for(QString filepath : dataSetpaths)
      addDataset(filepath);
    DefaultTrainSetName_ = value("defaultTrainSetName");
  }

  virtual void save()
  {
    setValue("timer_interval", QString::number(TimerInterval_));
    setValue("time_complete_transaction", Time_Complete_Transaction_.toString(Constants::TimeFormat));
    setValue("time_predict_price", Time_Predict_Price_.toString(Constants::TimeFormat));
    for(auto itr = DatasetFiles_.begin(); itr != DatasetFiles_.end(); ++itr)
      setValue("dataSetFiles", itr.value()->fileName() + Constants::Separator);
    setValue("defaultTrainSetName", DefaultTrainSetName_);
  }

  void addDataset(const QString& filename)
  {
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
  DataAssetSection(QString sectionName, QMap<QString, QString>& dataMap) : BaseDataSection(sectionName, dataMap) { }

  virtual void load()
  {
    Money_ = value("money").toDouble(); assert(Money_ >= 0.0);
    Energy_ = value("energy").toDouble(); assert(Energy_ >= 0.0);
    LastSysPrice_ = value("lastSysPrice").toDouble(); assert(LastSysPrice_ > 1.0);
    OrderNumber_ = value("currentOrderNumber").toUInt();
  }

  virtual void save()
  {
    setValue("money", Util::numberFormat(Money_));
    setValue("energy", Util::numberFormat(Energy_));
    setValue("lastSysPrice", Util::numberFormat(LastSysPrice_));
    setValue("currentOrderNumber", QString::number(OrderNumber_));
  }

  double Money_;
  double Energy_;
  double LastSysPrice_;
  unsigned int OrderNumber_;
};

class DataParseSection : public BaseDataSection
{
public:
  DataParseSection(QString sectionName, QMap<QString, QString>& dataMap) : BaseDataSection(sectionName, dataMap) { }

  virtual void load()
  {
    UrlSpot_ = value("url_spot");
    UrlPrices2013Daily_ = value("url_prices2013daily");
  }

  virtual void save()
  {
    setValue("url_spot", UrlSpot_);
    setValue("url_prices2013daily", UrlPrices2013Daily_);
  }

  QString UrlSpot_;
  QString UrlPrices2013Daily_;
};

class DataAgentSection : public BaseDataSection
{
public:
  DataAgentSection(QString sectionName, QMap<QString, QString>& dataMap) : BaseDataSection(sectionName, dataMap) { }

  virtual void load()
  {
    ClientEmailAddr_ = value("clientEmailAddr");
    ClientName_ = value("clientName");
    SmtpPassw_ = value("smtppassw");
    SmtpClient_ = value("smtpclient");
    SmtpPort_ = value("smtpport").toUInt();
    AgentId_ = value("agent_id").toUInt();
    SendEmail_ = value("sendEmail").toUInt() > 0;
    receiverEmail_ = value("receiverEmail");
    receiverEmail2_ = value("receiverEmail2");
    MaxMoneySpend_ = value("maxMoneySpend").toDouble();
    MaxEnergySell_ = value("maxEnergySell").toDouble();
    CurrentDay_ = value("currentDay").toUInt();
    HasMadeOrder_ = value("hasMadeOrder").toUInt() > 0 ? true : false;
    HasCompletedTransaction_ = value("hasCompletedTransaction").toUInt() > 0 ? true : false;
    IsAgentSleeping_ = value("isAgentSleeping").toUInt() > 0 ? true : false;
    ResetFlagsOnStartup_ = value("resetFlagsOnStartup").toUInt() > 0 ? true : false;
  }

  virtual void save()
  {
    setValue("clientEmailAddr", ClientEmailAddr_);
    setValue("clientName", ClientName_);
    setValue("smtppassw", SmtpPassw_);
    setValue("smtpclient", SmtpClient_);
    setValue("smtpport", QString::number(SmtpPort_));
    setValue("agent_id", QString::number(AgentId_));
    setValue("sendEmail", QString::number(SendEmail_ > 0 ? 1 : 0));
    setValue("receiverEmail", receiverEmail_);
    setValue("receiverEmail2", receiverEmail2_);
    setValue("maxMoneySpend", Util::numberFormat(MaxMoneySpend_));
    setValue("maxEnergySell", Util::numberFormat(MaxEnergySell_));
    setValue("currentDay", QString::number(CurrentDay_));
    setValue("hasMadeOrder", QString::number(HasMadeOrder_ > 0 ? 1 : 0));
    setValue("hasCompletedTransaction", QString::number(HasCompletedTransaction_ > 0 ? 1 : 0));
    setValue("isAgentSleeping", QString::number(IsAgentSleeping_ > 0 ? 1 : 0));
    setValue("resetFlagsOnStartup", QString::number(ResetFlagsOnStartup_ > 0 ? 1 : 0));
  }

  QString ClientEmailAddr_;
  QString ClientName_;
  QString receiverEmail_;
  QString receiverEmail2_;

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
  DataNeuralSection(QString sectionName, QMap<QString, QString>& dataMap) : BaseDataSection(sectionName, dataMap) { }

  virtual void load()
  {
    Nameset_ = value("setname"); assert(!Nameset_.isEmpty());
    MaxPriceExpected_ = value( "maxPriceExpected").toDouble(); assert(MaxPriceExpected_ > 0.0);
    DayAheadShort_ = value("dayAheadShort").toUInt(); assert(DayAheadShort_ > 0);
    DayAheadLong_ = value("dayAheadLong").toUInt(); assert(DayAheadLong_ > 0);
    DayPeriod_ = value("dayPeriod").toUInt(); assert(DayPeriod_ > 0);
    NumLayers_ = value("numLayers").toUInt(); assert(NumLayers_ > 0);
    NumNeuronsHidden_ = value("numNeuronsHidden").toUInt(); assert(NumNeuronsHidden_ > 0);
    NumOutputs_ = value("numOutputs").toUInt(); assert(NumOutputs_ > 0);
    DesiredError_ = value("DesiredError").toDouble(); assert(DesiredError_ > 10e-8);
    MaxEpochs_ = value("maxEpochs").toUInt(); assert(MaxEpochs_ > 0);
    EpochsBetweenReports_ = value("epochsBetweenReports").toUInt(); assert(EpochsBetweenReports_ > 0);
  }

  virtual void save()
  {
    setValue("setname", Nameset_);
    setValue("maxPriceExpected", Util::numberFormat(MaxPriceExpected_));
    setValue("dayAheadShort", QString::number(DayAheadShort_));
    setValue("dayAheadLong", QString::number(DayAheadLong_));
    setValue("dayPeriod", QString::number(DayPeriod_));
    setValue("numLayers", QString::number(NumLayers_));
    setValue("numNeuronsHidden", QString::number(NumNeuronsHidden_));
    setValue("numOutputs", QString::number(NumOutputs_));
    setValue("DesiredError", Util::numberFormat(DesiredError_));
    setValue("maxEpochs", QString::number(MaxEpochs_));
    setValue("epochsBetweenReports", QString::number(EpochsBetweenReports_));
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
