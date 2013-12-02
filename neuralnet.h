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
#include "NeuralConfig.h"

class NeuralNet
{
public:
  NeuralNet(const NeuralConfig& config);
  ~NeuralNet();

  void createTrainSet(const QString& trainSetName, const QMap<QDateTime, double>& spotprices);
  void train();
  void trainSet(DataTrainSet& set);
  void estimateNextPeriod(const QMap<QDateTime, double>& spotprices, QDateTime startdate, unsigned int numDaysPeriod, unsigned int numDaysAhead);

private:
  const NeuralConfig& config_;
  const QString Nameset_;
  const double MaxPriceExpected_;
  QList<DataTrainSet*> trainingSets_;
  const unsigned int DayAheadShort_;
  const unsigned int DayAheadLong_;

  /* for training */
  const unsigned int NumLayers_;
  const unsigned int NumNeuronsHidden_;
  const unsigned int NumOutputs_;  
  const float DesiredError_;
  const unsigned int MaxEpochs_;
  const unsigned int EpochsBetweenReports_;


  void generateFrequencies();
  void slidingWindowPrices(const QMap<QDateTime, double>& spotprices, QDateTime start, unsigned int numDaysPeriod, unsigned int numDaysAhead,
    std::deque<double>& priceWindow, std::deque<double>& aheadPriceWindow);
  QString fullDataTrainFileName(const DataTrainSet& set) const;
  QString fullNeuralFileName() const;
  void window(QMap<QDateTime, double>::const_iterator itr, std::deque<double>& priceWindow);
  void windowPriceEstimation(QMap<QDateTime, double>::const_iterator itr, std::deque<double>& priceWindow, QDateTime start, unsigned int numDaysPeriod);
};

#endif // NEURALNET_H
