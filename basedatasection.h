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
  BaseDataSection(QString sectionName, QMap<QString, QString>& dataMap) : sectionName_(sectionName), dataMap_(dataMap) { }
  virtual ~BaseDataSection() { }


  QString value(const QString& key)
  {
    if(!isValid(key)) return Constants::EmptyString;
    return dataMap_[key];
  }

  void setValue(const QString& key, const QString& value)
  {
    if(!isValid(key))
      return;
    dataMap_[key] = value;
  }
  void setValue(const QString& key, double value)
  {
    if(!isValid(key))
      return;
    dataMap_[key] = Util::numberFormat(value);
  }

  void setValue(const QString& key, QList<QString> values)
  {
    if(!isValid(key)) return;
    dataMap_[key] = "";
    for(QString value : values)
      dataMap_[key].append(value + Constants::Separator);
  }

  void setValue(const QString& key, unsigned int value)
  {
    if(!isValid(key)) return;
    dataMap_[key] = QString::number(value);
  }

  bool isValid(const QString& key)
  {
    if(!dataMap_.contains(key)) { Logger::get().append(QString("DataSection.IsValid: key not found. Section: %1. Key: %2.").arg(sectionName_, key), true); return false; }
    return true;
  }

  virtual QString toString() const
  {
    QString contents = "";
    // Write out header
    contents.append(QString("[%1]\n").arg(sectionName_));
    // Write out valuedata
    for(auto itr = dataMap_.begin(); itr != dataMap_.end(); ++itr)
      contents.append(itr.key() + '=' + itr.value() + "\n");
    return contents;
  }

  void saveAndRefresh()
  {
    // Persistent data is stored in the map. The subclass variables
    // are just bridges with types between. So save the peristant data
    // then reload the bridge values.
//    save();
//    load();
  }

  virtual void save() = 0;
  virtual void load() = 0;

protected:
  QString sectionName_;
  QMap<QString, QString>& dataMap_;
};

#endif // BASEDATASECTION_H

#ifndef MISCDATASECTION_H
#define MISCDATASECTION_H


#endif // MISCDATASECTION_H
