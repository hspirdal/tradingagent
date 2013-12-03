#ifndef NEURALNET_H
#define NEURALNET_H

#include "floatfann.h"
//#include "doublefann.h"
#include "fann_cpp.h"
#include <QString>
#include <QMap>
#include <QList>
#include <QDateTime>
#include "TrainingSet.h"
#include <memory>
#include <deque>
#include "NeuralConfig.h"
#include "constants.h"

class NeuralNet
{
public:
  NeuralNet(const NeuralConfig& config);
  ~NeuralNet();

  void createTrainSetAverage(const QString& trainSetName, const QMap<QDateTime, double>& spotprices, const unsigned int DayPeriod);
  void createTrainSetDayAhead(const QString &trainSetName, const QMap<QDateTime, double>& spotprices, const unsigned int DayPeriod);

  //void createTrainSet(const QString& trainSetName, const QMap<QDateTime, double>& spotprices);
  void train();
  void trainSet(const QString& setname);
  void trainSet(DataTrainSet& set);
  void estimateNextPeriod(const QMap<QDateTime, double>& spotprices, QDateTime startdate, unsigned int numDaysPeriod, unsigned int numDaysAhead);

  /* estimations */
  double estimateAveragePriceNextPeriod(const std::vector<double>& priceWindow);
  double estimateDayAheadPrice(const std::vector<double>& priceWindow);

private:
  const NeuralConfig& config_;
  const QString Nameset_;
  const double MaxPriceExpected_;
  QList<DataTrainSet*> trainingSets_;
  const unsigned int DayAheadLong_;
  /* for training */
  const unsigned int NumLayers_;
  const unsigned int NumNeuronsHidden_;
  const unsigned int NumOutputs_;  
  const float DesiredError_;
  const unsigned int MaxEpochs_;
  const unsigned int EpochsBetweenReports_;



  QString fullDataTrainFileName(const DataTrainSet& set) const;
  QString fullNeuralFileName(QString setname) const;
  void movingPriceWindow(QMap<QDateTime, double>::const_iterator itr, std::deque<double>& priceWindow, const unsigned int DayPeriod);



  void windowPriceEstimation(QMap<QDateTime, double>::const_iterator itr, std::deque<double>& priceWindow, QDateTime start, unsigned int numDaysPeriod);
    void generateFrequencies();
  void slidingWindowPrices(const QMap<QDateTime, double>& spotprices, QDateTime start, unsigned int numDaysPeriod, unsigned int numDaysAhead,
    std::deque<double>& priceWindow, std::deque<double>& aheadPriceWindow);

  /* Helpers */
  void createTrainSet(const QString &trainSetName, const QMap<QDateTime, double> &spotprices, const unsigned int DayPeriod, Constants::TrainSetFormat format);
  double calculateOutputAverage(const std::deque<double>& aheadPriceWindow);
  double calculateOutputDayAhead(const std::deque<double>& aheadPriceWindow);
};

#endif // NEURALNET_H
