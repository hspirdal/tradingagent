#ifndef BASEDATASECTION_H
#define BASEDATASECTION_H

#include <QString>
#include <QMap>
#include "logger.h"
#include "util.h"
#include "constants.h"

class BaseDataSection
{
public:
  BaseDataSection(QString sectionName) : sectionName_(sectionName) { }
  virtual ~BaseDataSection() { }

  virtual void setValues(const QMap<QString, QString>& sectionMap) = 0;
  virtual QString toString() const = 0;


protected:
  QString sectionName_;

  virtual QString value(const QString& key, const QMap<QString, QString>& sectionMap)
  {
    if(!isValid(key, sectionMap)) return Constants::EmptyString;
    return sectionMap[key];
  }

  virtual bool isValid(const QString& key, const QMap<QString, QString>& sectionMap)
  {
    if(!sectionMap.contains(key)) { Logger::get().append(QString("DataSection.IsValid: key not found. Section: %1. Key: %2.").arg(sectionName_, key), true); return false; }
    return true;
  }

  static void appendConfigLine(QString& sectionData, const QString& key, const QString& value)
  {
    sectionData.append(key + "=" + value + "\n");
  }

};

#endif // BASEDATASECTION_H

#ifndef MISCDATASECTION_H
#define MISCDATASECTION_H


#endif // MISCDATASECTION_H
