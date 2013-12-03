#ifndef ORDER_H
#define ORDER_H
#include <QDateTime>

struct Order
{
  Order() : dateTime_(QDateTime::currentDateTime()), boughtAmountEnergy_(0.0),
  soldAmountEnergy_(0.0), predictedUnitPrice_(0.0), systemPriceAtTime_(0.0), orderNumber_(0) { }

  Order(double boughtAmountEnergy, double soldAmountEnergy, double predictedUnitPrice, double systemPriceAtTime, unsigned int ordernr)
  : dateTime_(QDateTime::currentDateTime()), boughtAmountEnergy_(boughtAmountEnergy),  soldAmountEnergy_(soldAmountEnergy),
    predictedUnitPrice_(predictedUnitPrice), systemPriceAtTime_(systemPriceAtTime), orderNumber_(ordernr) { }

  QDateTime targetPredictionDate() const { return dateTime_.addDays(1); }

  QDateTime dateTime_;
  double boughtAmountEnergy_;
  double soldAmountEnergy_;
  double predictedUnitPrice_;
  double systemPriceAtTime_;
  unsigned int orderNumber_;
};

#endif // ORDER_H
