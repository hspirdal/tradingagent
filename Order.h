#ifndef ORDER_H
#define ORDER_H
#include <QDateTime>
#include "constants.h"

struct Order
{
  Order() : dateTime_(QDateTime::currentDateTime()), boughtAmountEnergy_(0.0),
  soldAmountEnergy_(0.0), predictedUnitPrice_(0.0), systemPriceAtTime_(0.0), orderNumber_(0) { }

  Order(double boughtAmountEnergy, double soldAmountEnergy, double predictedUnitPrice, double systemPriceAtTime, unsigned int ordernr)
  : dateTime_(QDateTime::currentDateTime()), boughtAmountEnergy_(boughtAmountEnergy),  soldAmountEnergy_(soldAmountEnergy),
    predictedUnitPrice_(predictedUnitPrice), systemPriceAtTime_(systemPriceAtTime), orderNumber_(ordernr) { }

  QDateTime targetPredictionDate() const { return dateTime_.addDays(1); }
  QString toString() const
  {
    QString content = "";
    content.append(QString::number(orderNumber_) + " ");
    content.append(dateTime_.toString(Constants::DateTimeFormat) + " ");
    content.append(QString::number(predictedUnitPrice_) + " ");
    content.append(QString::number(systemPriceAtTime_) + " ");
    content.append(QString::number(boughtAmountEnergy_) + " ");
    content.append(QString::number(soldAmountEnergy_) + " \n");
    return content;
  }



  QDateTime dateTime_;
  double boughtAmountEnergy_;
  double soldAmountEnergy_;
  double predictedUnitPrice_;
  double systemPriceAtTime_;
  unsigned int orderNumber_;
};

#endif // ORDER_H
