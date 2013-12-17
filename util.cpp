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

QList<QStringList> Util::loadCSVFiles(const QList<QFile*> &files, QChar seperator)
{
  QList<QStringList> dataMatrix;
  for(QFile* file : files)
    dataMatrix.append(loadCSVFile(file->fileName(), seperator));

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

    // Quick way to ignore any header rows. Current year has one more row of headerdata. Any row evaluating this to false
    // means it isn't a row with actual price data, so we can ignore it.
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

/* very specialized method to extract data from a specific XLS sheet online at
 * http://www.nordpoolspot.com/PageFiles/9383/Elspot%20Prices_2013_Daily_NOK.xls
 */
QMap<QDateTime, double> Util::parseXLS_daily(const QString& content)
{
  static QString day_start =  "<td style=\"text-align:left;\">";
  static QString endTd = "</td>";
  static QString price_html_start = "<td style=\"text-align:right;\">";
  static QString price_html_preliminary = "<td style=\"color:purple;text-align:right;\">";
  QMap<QDateTime, double> priceMap;

  // The last few days in the document will be marked with a purple tag if its been weekend. Thus the extra work.
  // We're basically extracting the dato and the first price instance per line. Current version works great, but
  // could of course break anytime during the future if the format changes in the file.
  QStringList lines = content.split('\n');
  for(QString line : lines)
  {
    if(line.contains(day_start))
    {
      int indexDateStart = line.indexOf(day_start) + day_start.length();
      int indexDateEnd = line.indexOf(endTd, indexDateStart);
      QString datestring = line.mid(indexDateStart, indexDateEnd-indexDateStart);
      QDateTime date = QDateTime::fromString(datestring, Constants::DateTimeFormat);

      int indexPriceStart = 0;
      if(line.contains(price_html_preliminary))
        indexPriceStart = line.indexOf(price_html_preliminary, indexDateEnd) + price_html_preliminary.length();
      else
        indexPriceStart = line.indexOf(price_html_start, indexDateEnd) + price_html_start.length();

      int indexPriceEnd = line.indexOf(endTd, indexPriceStart);
      QString pricestring = line.mid(indexPriceStart, indexPriceEnd-indexPriceStart).replace(',', '.');
      double price = pricestring.toDouble();

      priceMap.insert(date, price);
    }
  }

  if(priceMap.count() <= 0) Logger::get().append("ParseXLSDaily: Expected more than 0 items in the pricelist.", true);
  return priceMap;
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
      // On weekends, non-euro valuta is only preliminary, and is put in a td of class preliminary, so check for that.
      if(html.contains(Preliminary))
      {
        start_NOKpriceIndex = html.indexOf(Preliminary) +  Preliminary.length();
        //start_NOKpriceIndex = html.indexOf(Preliminary, first) + Preliminary.length();
      }
      else
      {
      int first = html.indexOf(Official) +  Official.length();
      start_NOKpriceIndex = html.indexOf(Official, first) + Official.length();
      }
    }
    end_NOKpriceIndex = html.indexOf(EndTDtag, start_NOKpriceIndex);
    QString pricestring = html.mid(start_NOKpriceIndex, end_NOKpriceIndex-start_NOKpriceIndex);
    double spotprice = pricestring.replace(',', '.').toDouble();

    if(spotprice <= 0.0)
      Logger::get().append("ParseNordpool: spotprice was not parsed correctly! Pricestring:" + pricestring , true);

    return spotprice;
}

QMap<QDateTime, double> Util::extractPricesWithinDate(const QMap<QDateTime, double>& spotprices, QDateTime from, QDateTime to)
{
  QMap<QDateTime, double> spotpriceSelection;
  for(auto itr = spotprices.begin(); itr != spotprices.end(); ++itr)
  {
    if(itr.key() >= from && itr.key() <= to)
      spotpriceSelection.insert(itr.key(), itr.value());
  }
  if(spotpriceSelection.count() <= 0) Logger::get().append("Util.ExtractPricesWithinDate: Price selection is zero.", true);
  return spotpriceSelection;
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

QString Util::readFile(const QString filename)
{
  QString content = "";
  QFile file(filename);
  if(file.open(QFile::ReadOnly))
  {
    QTextStream in(&file);
    in.setCodec("UTF-8");
    while(!in.atEnd())
      content.append(in.readLine());
  }
  return content;
}

std::deque<Order> Util::loadOrderFile(const QString& filename)
{
  std::deque<Order> orders;
  // Not exactly CSV file, but close. :)
  QList<QStringList> rawparts = Util::loadCSVFile(filename, '|');
  for(QStringList line : rawparts)
  {
    Order order;
    order.orderNumber_ = line[0].toUInt();
    order.dateTime_ = QDateTime::fromString(line[1], Constants::DateTimeFormat);
    order.predictedUnitPrice_ = line[2].toDouble();
    order.systemPriceAtTime_ = line[3].toDouble();
    order.boughtAmountEnergy_ = line[4].toDouble();
    order.soldAmountEnergy_ = line[5].toDouble();
    orders.push_back(order);
  }
  return orders;
}

QList<QString> Util::absoluteFilePath(QList<QFile *> files)
{
  QList<QString> list;
  for(QFile* file : files)
    list.append(file->fileName());

  return list;
}



