#ifndef CONFIG_H
#define CONFIG_H

#include <QMap>
#include <QString>
#include <logger.h>
#include "NeuralConfig.h"
#include <constants.h>
#include <QTime>
#include <assert.h>
#include "util.h"

struct AssetsConfig
{
  double Money_;
  double Energy_;
  double LastSysPrice_;
  unsigned int OrderNumber_;
};

struct MiscConfig
{
  unsigned int TimerInterval_;
  QTime Time_Complete_Transaction_;
  QTime Time_Predict_Price_;
};

struct AgentInfoConfig
{
  QString ClientEmailAddr_;
  QString ClientName_;
  QString receiverEmail_;
  QString receiverEmail2_;

  QString SmtpPassw_;
  QString SmtpClient_;
  unsigned int SmtpPort_;
  QString SmtpCon_;
  unsigned int AgentId_;
  bool SendEmail_;
  double MaxMoneySpend_;
  double MaxEnergySell_;
  unsigned int CurrentDay_;
  bool HasMadeOrder_;
  bool HasCompletedTransaction_;
  bool IsAgentSleeping_;

};

struct ParseConfig
{
  QString UrlSpot_;
  QString UrlPrices2013Daily_;
};


class Config
{
public:
  Config(QMap<QString, QMap<QString, QString> > configmap) : dataMap_(configmap), Empty_(), neuralConfig_()
  {
    reloadConfigs();
  }

  void reloadConfigs()
  {
    loadNeuralConfig();
    loadAssetsConfig();
    loadMiscConfig();
    loadAgentInfoConfig();
    loadParseConfig();
  }

  void save()
  {
    Util::writeFile("config.ini", this->toString().toStdString(), true);
  }

  QString value(const QString& section, const QString& key)
  {
    if(!dataMap_.contains(section)) { Logger::get().append("Config.value: section not found.", true); return Empty_; }
    if(!dataMap_[section].contains(key)) { Logger::get().append("Config.value: key not found: " + key , true); return Empty_; }
    return dataMap_[section][key];
  }
  void setValue(const QString& section, const QString& key, double value)
  {
    if(!dataMap_.contains(section)) { Logger::get().append("Config.setValue: section not found", true); return; }
    if(!dataMap_[section].contains(key)) { Logger::get().append("Config.value: key not found: " + key, true); return; }
    dataMap_[section][key] = QString::number(value);

    // It's suboptimal, but currently it's just a fix expanding on a little shortsighted future set that was assumed to have values that
    // does not change during runtime. If a value changes at runtime, we need to update it 'locally' as well.
    // the local struct containers are just convenience storage for the same data (in string value) in the mapper.
    // I could only reload the value that changed, but its a small file; it's easier currently to just reload all of it.
    reloadConfigs();
    save();
  }

  void setValue(const QString& section, const QString& key, unsigned int value)
  {
    if(!dataMap_.contains(section)) { Logger::get().append("Config.setValue: section not found", true); return; }
    if(!dataMap_[section].contains(key)) { Logger::get().append("Config.value: key not found.", true); return; }
    dataMap_[section][key] = QString::number(value);
    reloadConfigs();
    save();
  }

  const NeuralConfig& neuralConfig() const { return neuralConfig_; }
  const AssetsConfig& assetsConfig() const { return assetsConfig_; }
  const MiscConfig& miscConfig() const { return miscConfig_; }
  const AgentInfoConfig& agentInfoConfig() const { return agentInfoConfig_; }
  const ParseConfig& parseConfig() const { return parseConfig_; }


  QString toString()
  {
    QString contents = "";
    for(auto itr = dataMap_.begin(); itr != dataMap_.end(); ++itr)
    {
      // Write out header
      contents.append(QString("[%1]\n").arg(itr.key()));
      // Write out subdata
      QMap<QString, QString>& subdata = itr.value();
      for(auto subitr = subdata.begin(); subitr != subdata.end(); ++subitr)
        contents.append(subitr.key() + '=' + subitr.value() + "\n");
    }
    return contents;
  }

private:
  QMap<QString, QMap<QString, QString> > dataMap_;
  const QString Empty_;
  NeuralConfig neuralConfig_;
  AssetsConfig assetsConfig_;
  MiscConfig miscConfig_;
  AgentInfoConfig agentInfoConfig_;
  ParseConfig parseConfig_;


  void loadNeuralConfig()
  {
    const QString Section = "ann";
    neuralConfig_.Nameset_ = value(Section, "setname"); assert(!neuralConfig_.Nameset_.isEmpty());
    neuralConfig_.MaxPriceExpected_ = value(Section, "maxPriceExpected").toDouble(); assert(neuralConfig_.MaxPriceExpected_ > 0.0);
    neuralConfig_.DayAheadShort_ = value(Section, "dayAheadShort").toUInt(); assert(neuralConfig_.DayAheadShort_ > 0);
    neuralConfig_.DayAheadLong_ = value(Section, "dayAheadLong").toUInt(); assert(neuralConfig_.DayAheadLong_ > 0);
    neuralConfig_.DayPeriod_ = value(Section, "dayPeriod").toUInt(); assert(neuralConfig_.DayPeriod_ > 0);
    neuralConfig_.NumLayers_ = value(Section, "numLayers").toUInt(); assert(neuralConfig_.NumLayers_ > 0);
    neuralConfig_.NumNeuronsHidden_ = value(Section, "numNeuronsHidden").toUInt(); assert(neuralConfig_.NumNeuronsHidden_ > 0);
    neuralConfig_.NumOutputs_ = value(Section, "numOutputs").toUInt(); assert(neuralConfig_.NumOutputs_ > 0);
    neuralConfig_.DesiredError_ = value(Section, "DesiredError").toDouble(); assert(neuralConfig_.DesiredError_ > 10e-8);
    neuralConfig_.MaxEpochs_ = value(Section, "maxEpochs").toUInt(); assert(neuralConfig_.MaxEpochs_ > 0);
    neuralConfig_.EpochsBetweenReports_ = value(Section, "epochsBetweenReports").toUInt(); assert(neuralConfig_.EpochsBetweenReports_ > 0);
  }

  void loadAssetsConfig()
  {
    const QString Section = "assets";
    assetsConfig_.Money_ = value(Section, "money").toDouble(); assert(assetsConfig_.Money_ >= 0.0);
    assetsConfig_.Energy_ = value(Section, "energy").toDouble(); assert(assetsConfig_.Energy_ >= 0.0);
    assetsConfig_.LastSysPrice_ = value(Section, "lastSysPrice").toDouble(); assert(assetsConfig_.LastSysPrice_ > 1.0);
    assetsConfig_.OrderNumber_ = value(Section, "currentOrderNumber").toUInt();
  }

  void loadMiscConfig()
  {
    const QString Section = "misc";
    miscConfig_.TimerInterval_ = value(Section, "timer_interval").toUInt(); assert(miscConfig_.TimerInterval_ > 0);
    miscConfig_.Time_Complete_Transaction_ = QTime::fromString(value(Section, "time_complete_transaction"), Constants::TimeFormat);
    miscConfig_.Time_Predict_Price_ = QTime::fromString(value(Section, "time_predict_price"), Constants::TimeFormat);
  }

  void loadAgentInfoConfig()
  {
    const QString Section = "agentinfo";
    agentInfoConfig_.ClientEmailAddr_ = value(Section, "clientEmailAddr");
    agentInfoConfig_.ClientName_ = value(Section, "clientName");
    agentInfoConfig_.SmtpPassw_ = value(Section, "smtppassw");
    agentInfoConfig_.SmtpClient_ = value(Section, "smtpclient");
    agentInfoConfig_.SmtpPort_ = value(Section, "smtpport").toUInt();
    agentInfoConfig_.SmtpCon_ = value(Section, "smtpcon");
    agentInfoConfig_.AgentId_ = value(Section, "agent_id").toUInt();
    agentInfoConfig_.SendEmail_ = value(Section, "sendEmail").toUInt() > 0;
    agentInfoConfig_.receiverEmail_ = value(Section, "receiverEmail");
    agentInfoConfig_.receiverEmail2_ = value(Section, "receiverEmail2");
    agentInfoConfig_.MaxMoneySpend_ = value(Section, "maxMoneySpend").toDouble();
    agentInfoConfig_.MaxEnergySell_ = value(Section, "maxEnergySell").toDouble();
    agentInfoConfig_.CurrentDay_ = value(Section, "currentDay").toUInt();
    agentInfoConfig_.HasMadeOrder_ = value(Section, "hasMadeOrder").toUInt() > 0 ? true : false;
    agentInfoConfig_.HasCompletedTransaction_ = value(Section, "hasCompletedTransaction").toUInt() > 0 ? true : false;
    agentInfoConfig_.IsAgentSleeping_ = value(Section, "isAgentSleeping").toUInt() > 0 ? true : false;
  }

  void loadParseConfig()
  {
    const QString Section = "parse";
    parseConfig_.UrlSpot_ = value(Section, "url_spot");
    parseConfig_.UrlPrices2013Daily_ = value(Section, "url_prices2013daily");
  }

};

#endif // CONFIG_H
