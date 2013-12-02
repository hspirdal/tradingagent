#include "util.h"
#include <QFile>
#include <QDebug>
#include <assert.h>
#include "constants.h"
#include <fstream>
#include <assert.h>
#include "logger.h"

Util::Util()
{
}

QList<QStringList> Util::loadCSVFile(const QString &filename, QChar separator)
{
  qDebug() << "parsing..";
  QFile file(filename);
  QList<QStringList> dataMatrix;

  if(file.open(QFile::ReadOnly))
  {
    QTextStream in(&file);
    in.setCodec("UTF-8");
    while(!in.atEnd())
    {
      QString line = in.readLine();
      QStringList fields = line.split(separator);
      dataMatrix.append(fields);
    }
    file.close();
  }
  else
    qDebug() << QString("Could not get read-access to file %1").arg(filename);

  return dataMatrix;
}

QList<QStringList> Util::loadCSVFiles(const QList<QString> &filenames, QChar separator)
{
  QList<QStringList> dataMatrix;
  for(QString filename : filenames)
    dataMatrix.append(loadCSVFile(filename, separator));

  return dataMatrix;
}

QMap<QString, QMap<QString, QString> > Util::loadIniFile(const QString &filename)
{
  QFile file(filename);
  QMap<QString, QMap<QString, QString> > mainMap;
  QMap<QString, QString> keyvalMap;
  QString currentSection = "";
  if(file.open(QFile::ReadOnly))
  {
    QTextStream in(&file);
    in.setCodec("UTF-8");
    while(!in.atEnd())
    {
      QString line = in.readLine().trimmed();

      // Check if we're at a new section.
      if(line.startsWith('['))
      {
        // first remove [ and ]
        line = line.remove(0, 1);
        line = line.remove(line.length()-1, 1);
        currentSection = line;
        keyvalMap = QMap<QString, QString>();
        mainMap.insert(currentSection, keyvalMap);
        continue;
      }

      // Ignore empty lines or lines starting with ;comment
      if(line.isEmpty() || line.startsWith(';'))
        continue;

      if(!line.contains('=')) Logger::get().append("Util.LoadIniFile: unexpected line format.", "global.log", true);
      if(currentSection.isEmpty()) Logger::get().append("Util.LoadIniFile: section must be set before keyval lines.", "global.log", true);

      // key=val format
      QStringList keyval = line.split('=');
      assert(keyval.count() == 2);
      QMap<QString, QString>& keyvalMap = mainMap[currentSection];
      keyvalMap.insert(keyval[0], keyval[1]);
    }
  }
  return mainMap;
}

QMap<QDateTime, double> Util::extractSystemPriceDaily(const QList<QStringList> &dataMatrix)
{
  unsigned int lineno = 0;
  QMap<QDateTime, double> prices;
  foreach(QStringList list, dataMatrix)
  {
//    // Skip the first 3 lines of the CSV as they contain header data.
    if(++lineno < 4)
      continue;

    QDateTime date = QDateTime::fromString(list.at(0), Constants::DateTimeFormat);

    // Quick way to ignore any header rows. Current year has one more row of headerdata.
    if(!date.isValid())
      continue;

    assert(lineno >= 4);

    // Decimal is unfortunately comma-separated in the CSV file, and is thus split. Merging back to complete double.
    QString sysprice = list.at(1) + "." + list.at(2);

    // The number columns are wrapped in "xx,xx". Remove those.
    double sys = sysprice.remove('"').toDouble();
    assert(sys > Constants::ApproxZeroDouble && "Price was zero or less, which is unexpected and probably a bug.");
    //qDebug() << date.toString() << QString::number(sys);
    prices.insert(date, sys);
  }
  return prices;
}

double Util::parseNordpoolSpotpriceNOK(const QString html)
{
    static const QString Official = "<td class=\"price-official\" title=\"Official\">";
    static const QString Preliminary = "<td class=\"preliminary\" title=\"Preliminary\">";
    static const QString EndTDtag = "</td>";

    int start_NOKpriceIndex = 0;
    int end_NOKpriceIndex = 0;
    if(html.contains(Official))
    {
      int first = html.indexOf(Official) +  Official.length();
      start_NOKpriceIndex = html.indexOf(Official, first) + Official.length();
    }
    else if(html.contains(Preliminary))
    {
      int first = html.indexOf(Preliminary) +  Preliminary.length();
      start_NOKpriceIndex = html.indexOf(Preliminary, first) + Preliminary.length();
    }
    end_NOKpriceIndex = html.indexOf(EndTDtag, start_NOKpriceIndex);
    QString pricestring = html.mid(start_NOKpriceIndex, end_NOKpriceIndex-start_NOKpriceIndex);
    double spotprice = pricestring.replace(',', '.').toDouble();

    if(spotprice <= 0.0)
      Logger::get().append("ParseNordpool: spotprice was not parsed correctly! Pricestring:" + pricestring , true);

    return spotprice;
}

bool Util::writeFile(const std::string& filename, const std::string& content, bool overwrite)
{
  // TODO: handle errors.
  std::ofstream out(filename);
  if(!overwrite)
  {
    std::ifstream in(filename);
    out << std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  }
  out << content;
  out.close();
  return true;
}


