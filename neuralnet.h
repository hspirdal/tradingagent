#ifndef NEURALNET_H
#define NEURALNET_H

#include "3rdparty/FANN/floatfann.h"
#include "3rdparty/FANN/fann_cpp.h"
#include <QString>
#include <QMap>
#include <QList>
#include <QDateTime>
#include "TrainingSet.h"
#include <memory>
#include <deque>
#include "Config.h"
#include "constants.h"
#include "applicationlogger.h"
#include "ui_mainwindow.h"

class NeuralNet
{
public:
  NeuralNet(std::shared_ptr<Config> config, std::shared_ptr<ApplicationLogger> log, Ui::MainWindow* window);
  ~NeuralNet();

  void createTrainSetAverage(const QString& trainSetName, const QMap<QDateTime, double>& spotprices, const unsigned int DayPeriod);
  void createTrainSetDayAhead(const QString &trainSetName, const QMap<QDateTime, double>& spotprices, const unsigned int DayPeriod);


  //void createTrainSet(const QString& trainSetName, const QMap<QDateTime, double>& spotprices);
  void train();
  void trainSet(const QString& setname);
  void trainSet(DataTrainSet& set);
  void trainSetCallback(DataTrainSet& set);
  void estimateNextPeriod(const QMap<QDateTime, double>& spotprices, QDateTime startdate, unsigned int numDaysPeriod, unsigned int numDaysAhead);

  /* estimations */
  double estimateAveragePriceNextPeriod(const std::vector<double>& priceWindow);
  double estimateDayAheadPrice(const std::vector<double>& priceWindow);

  // Callback to FANN which is used for progress info while training.
  static int print_callback(FANN::neural_net &net, FANN::training_data &train, unsigned int max_epochs, unsigned int epochs_between_reports,
              float desired_error, unsigned int epochs, void* neuralNet);


private:
  std::shared_ptr<Config> config_;
  std::shared_ptr<DataNeuralSection> neurConfig_;
  std::shared_ptr<ApplicationLogger> log_;
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

  std::unique_ptr<FANN::neural_net> neurnet_;
  Ui::MainWindow* window_;


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
