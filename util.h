#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QMap>
#include <assert.h>
#include "Order.h"
#include <deque>

class Util
{
public:
  static QList<QStringList>  loadCSVFile(const QString& filename, QChar separator);
  // Just the very basic ini format: key=val, ignoring lines starting with [ or ; ([section, ;comment).
  static QMap<QString, QMap<QString, QString> > loadIniFile(const QString& filename);
  static QMap<QDateTime, double> extractSystemPriceDaily(const QList<QStringList>& dataMatrix);
  static QList<QStringList> loadCSVFiles(const QList<QString>& filenames, QChar separator);
  static QMap<QDateTime, double> parseXLS_daily(const QString& content);
  static double parseNordpoolSpotpriceNOK(const QString html);
  static QMap<QDateTime, double> extractPricesWithinDate(const QMap<QDateTime, double>& spotprices, QDateTime from, QDateTime to);
  static bool writeFile(const std::string& filename, const std::string& content, bool overwrite = false);
  static QString readFile(const QString filename);
  static std::deque<Order> loadOrderFile(const QString& filename);

  static bool lessEqualAbs(double a, double b, double epsilon = 10e-8)
  {
    // Lacks proper testing,  but should be Ok for lesser stuff.
    return areSame(a, b, epsilon) || a < b;
  }

  static bool areSame(double a, double b, double epsilon = 10e-8)
  {
    return std::abs(a - b) < epsilon;
  }

  static double clamp(double value, double min, double max)
  {
    assert(max >= min && "Min value higher than max value");
    return std::min(std::max(value, min), max);
  }

private:
  Util();
};

#endif // UTIL_H
