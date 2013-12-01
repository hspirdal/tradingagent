#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QMap>

class Util
{
public:
  static QList<QStringList>  loadCSVFile(const QString& filename, QChar separator);
  static QMap<QDateTime, double> extractSystemPriceDaily(const QList<QStringList>& dataMatrix);
  static QList<QStringList> loadCSVFiles(const QList<QString>& filenames, QChar separator);
  static bool writeFile(const std::string& filename, const std::string& content, bool overwrite = false);

  static bool lessEqualAbs(double a, double b, double epsilon = 10e-8)
  {
    // Lacks proper testing,  but should be Ok for lesser stuff.
    return areSame(a, b, epsilon) || a < b;
  }

  static bool areSame(double a, double b, double epsilon = 10e-8)
  {
    return std::abs(a - b) < epsilon;
  }

private:
  Util();
};

#endif // UTIL_H
