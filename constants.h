#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace Constants
{
  const QString DateTimeFormat = "dd-MM-yyyy";
  const QString TimeFormat = "hh:mm:ss";
  const double ApproxZeroDouble = 10e-6;

  enum RunMode { NotRunning, Running };
  enum TrainSetFormat { PeriodAverage, PeriodDayAhead };

}

#endif // CONSTANTS_H
