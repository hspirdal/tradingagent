#ifndef CONFIG_H
#define CONFIG_H

#include <QMap>
#include <QString>
#include <logger.h>
//#include "NeuralConfig.h"
#include <constants.h>
#include <QTime>
#include <assert.h>
#include "util.h"
#include "constants.h"
#include <memory>

#include "datasections.h"


class Config
{
public:
  Config()
  {
    load();
  }

  void load()
  {
    const QMap<QString, QMap<QString, QString> > configMap = Util::loadIniFile(ConfigFileName_);
    neuralConfig_.setValues(configMap["ann"]);
    assetsConfig_.setValues(configMap["assets"]);
    miscConfig_.setValues(configMap["misc"]);
    agentInfoConfig_.setValues(configMap["agentinfo"]);
    parseConfig_.setValues(configMap["parse"]);
  }

  void save()
  {
    QString configData;
    configData.append(neuralConfig_.toString());
    configData.append(assetsConfig_.toString());
    configData.append(miscConfig_.toString());
    configData.append(agentInfoConfig_.toString());
    configData.append(parseConfig_.toString());
    Util::writeFile(ConfigFileName_.toStdString(), configData.toStdString(), true);
  }

  DataNeuralSection& neuralConfig() { return neuralConfig_; }
  DataAssetSection& assetsConfig() { return assetsConfig_; }
  DataMiscSection& miscConfig() { return miscConfig_; }
  DataAgentSection& agentInfoConfig() { return agentInfoConfig_; }
  DataParseSection& parseConfig() { return parseConfig_; }

private:
  DataNeuralSection neuralConfig_;
  DataAssetSection assetsConfig_;
  DataMiscSection miscConfig_;
  DataAgentSection agentInfoConfig_;
  DataParseSection parseConfig_;
  const QString ConfigFileName_ = "config.ini";
};

#endif // CONFIG_H
