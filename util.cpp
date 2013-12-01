#include "util.h"
#include <QFile>
#include <QDebug>
#include <assert.h>
#include "constants.h"
#include <fstream>

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
    qDebug() << date.toString() << QString::number(sys);
    prices.insert(date, sys);
  }
  return prices;
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


