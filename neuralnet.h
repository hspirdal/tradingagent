#ifndef NEURALNET_H
#define NEURALNET_H

#include "floatfann.h"
#include "fann_cpp.h"
#include <QString>
#include <QMap>
#include <QList>
#include <QDateTime>
#include "TrainingSet.h"
#include <memory>
#include <deque>

class NeuralNet
{
public:
  NeuralNet(QString nameset, unsigned int numDayPeriod);
  ~NeuralNet();

  void createTrainSet(const QString& trainSetName, const QMap<QDateTime, double>& spotprices);
  void train();
  void trainSet(DataTrainSet& set);
  void estimateNextPeriod(const QMap<QDateTime, double>& spotprices, QDateTime start, unsigned int numDaysPeriod);

private:
  QString nameset_;
  const double MaxPriceExpected_;
  const unsigned int numDayPeriod_;
  const unsigned int numOutputs_;
  QList<DataTrainSet*> trainingSets_;

  void generateFrequencies();
  void slidingWindowPrices(const QMap<QDateTime, double>& spotprices, QDateTime start, unsigned int numDaysPeriod,
    std::deque<double>& priceWindow, std::deque<double>& aheadPriceWindow);
  QString fullDataTrainFileName(const DataTrainSet& set) const;
  QString fullNeuralFileName() const;
  void window(QMap<QDateTime, double>::const_iterator itr, std::deque<double>& priceWindow);
  void windowPriceEstimation(QMap<QDateTime, double>::const_iterator itr, std::deque<double>& priceWindow, QDateTime start, unsigned int numDaysPeriod);
};

#endif // NEURALNET_H
