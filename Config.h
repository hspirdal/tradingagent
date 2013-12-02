#ifndef CONFIG_H
#define CONFIG_H

#include <QMap>
#include <QString>
#include <logger.h>
#include "NeuralConfig.h"

struct AssetsConfig
{
public:
  double Money_;
  double Energy_;
  double LastSysPrice_;
};


class Config
{
public:
  Config(QMap<QString, QMap<QString, QString> > configmap) : dataMap_(configmap), Empty_(), neuralConfig_()
  {
    loadNeuralConfig();
    loadAssetsConfig();
  }

  QString value(const QString& section, const QString& key)
  {
    if(!dataMap_.contains(section)) { Logger::get().append("Config.value: section not found.", true); return Empty_; }
    if(!dataMap_[section].contains(key)) { Logger::get().append("Config.value: key not found.", true); return Empty_; }
    return dataMap_[section][key];
  }

  const NeuralConfig& neuralConfig() const { return neuralConfig_; }
  const AssetsConfig& assetsConfig() const { return assetsConfig_; }

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


  void loadNeuralConfig()
  {
    const QString Section = "ann";
    neuralConfig_.Nameset_ = value(Section, "setname"); assert(!neuralConfig_.Nameset_.isEmpty());
    neuralConfig_.MaxPriceExpected_ = value(Section, "maxPriceExpected").toDouble(); assert(neuralConfig_.MaxPriceExpected_ > 0.0);
    neuralConfig_.DayAheadShort_ = value(Section, "dayAheadShort").toUInt(); assert(neuralConfig_.DayAheadShort_ > 0);
    neuralConfig_.DayAheadLong_ = value(Section, "dayAheadLong").toUInt(); assert(neuralConfig_.DayAheadLong_ > 0);
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
  }

};

#endif // CONFIG_H
