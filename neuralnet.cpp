#include "neuralnet.h"
#include <QDebug>
#include "logger.h"

NeuralNet::NeuralNet(const NeuralConfig& config)
:
  config_(config),
  Nameset_(config.Nameset_), MaxPriceExpected_(config.MaxPriceExpected_),
  DayAheadLong_(config.DayAheadLong_), NumLayers_(config.NumLayers_), NumNeuronsHidden_(config.NumNeuronsHidden_),
  NumOutputs_(config.NumOutputs_), DesiredError_(config.DesiredError_), MaxEpochs_(config.MaxEpochs_), EpochsBetweenReports_(config.EpochsBetweenReports_)
{
}

NeuralNet::~NeuralNet()
{
  for(DataTrainSet* set : trainingSets_)
    if(set != NULL)
      delete set;

  trainingSets_.clear();
}



//void NeuralNet::createTrainSet(const QString& trainSetName, const QMap<QDateTime, double>& spotprices)
//{
//  trainingSets_.append(new DataTrainSet(trainSetName, DayAheadLong_, NumOutputs_));

//  unsigned int rownum = 0;
//  std::vector<double> dayprices;

//  std::deque<double> priceWindow;
//  std::deque<double> aheadPriceWindow;
//  assert(spotprices.size() > static_cast<int>(DayAheadLong_) && "period less than window");
//  auto ahead_itr = spotprices.begin() + DayAheadLong_;
//  for(auto itr = spotprices.begin(); itr != spotprices.end(); ++itr)
//  {
//      ++rownum;
//      window(itr, priceWindow);

//      if(static_cast<int>(rownum + DayAheadLong_) >= spotprices.size())
//      {
//        qDebug() << "lookahead at end. breaking out.";
//        break;
//      }
//      window(++ahead_itr, aheadPriceWindow);


//      if(priceWindow.size() >= DayAheadLong_)
//      {
//        assert(priceWindow.size() == DayAheadLong_ && "size 'when full' should be exactly like numDayPeriod.");
//        assert(aheadPriceWindow.size() == priceWindow.size() && "unexpected size difference.");

//        trainingSets_.last()->appendRow(priceWindow, aheadPriceWindow);
//      }
//  }
//  const QString fullname = fullDataTrainFileName(*trainingSets_.last());
//  trainingSets_.last()->saveToFile(fullname);
//}

/* Creates a dataset where output is the average of the next period to current. Its purpose is to look further than one day into the future. */
/* Example: Using day 01-15 as input, and then use day 16-30 average as output ('solution') to that set. */
void NeuralNet::createTrainSetAverage(const QString &trainSetName, const QMap<QDateTime, double> &spotprices, const unsigned int DayPeriod)
{
  createTrainSet(trainSetName, spotprices, DayPeriod, Constants::TrainSetFormat::PeriodAverage);
}

/* Creates a dataset where output is the "day-ahead" in a series of days. Its purpose is to be used when predicting day-ahead price. */
/* Example: Day 01-15 as input. Day 16 as output for said set. */
void NeuralNet::createTrainSetDayAhead(const QString &trainSetName, const QMap<QDateTime, double>& spotprices, const unsigned int DayPeriod)
{
  createTrainSet(trainSetName, spotprices, DayPeriod, Constants::TrainSetFormat::PeriodDayAhead);
}

double NeuralNet::calculateOutputAverage(const std::deque<double>& aheadPriceWindow)
{
    double d =  std::accumulate(aheadPriceWindow.begin(), aheadPriceWindow.end(), 0.0) / aheadPriceWindow.size();
    return d;
}

double NeuralNet::calculateOutputDayAhead(const std::deque<double>& aheadPriceWindow)
{
  // Keeping this in its own method in case of later changes. Basically return the last item.
  double last = aheadPriceWindow.at(aheadPriceWindow.size()-1);
  return last;
}

void NeuralNet::createTrainSet(const QString &trainSetName, const QMap<QDateTime, double> &spotprices, const unsigned int DayPeriod, Constants::TrainSetFormat format)
{
  trainingSets_.append(new DataTrainSet(trainSetName, DayPeriod, NumOutputs_));
  unsigned int rownum = 0;
  std::deque<double> priceWindow;
  std::deque<double> aheadPriceWindow;
  const unsigned int MinPriceRows = format == Constants::TrainSetFormat::PeriodDayAhead ? DayPeriod+1 : DayPeriod*2;
  assert(spotprices.size() > static_cast<int>(MinPriceRows) && "periods less than window");
    // Depending on which format we're creating, we determine the iterator offset the one ahead needs to be.
  const unsigned int MinDistAheadItr = format == Constants::TrainSetFormat::PeriodDayAhead ? 1 : DayPeriod;
  auto ahead_itr = spotprices.begin() + MinDistAheadItr;
  for(auto itr = spotprices.begin(); itr != spotprices.end(); ++itr)
  {
    ++rownum;
    movingPriceWindow(itr, priceWindow, DayPeriod);
    // Check if the one ahead has reached the end, and if so, break to loop.
    if(static_cast<int>(rownum + MinPriceRows) >= spotprices.size()) { break; }

    movingPriceWindow(++ahead_itr, aheadPriceWindow, DayPeriod);
    if(priceWindow.size() >= DayPeriod)
    {
      assert(priceWindow.size() == DayPeriod && "size 'when full' should be exactly like numDayPeriod.");
      assert(aheadPriceWindow.size() == priceWindow.size() && "unexpected size difference.");

      // A training set takes [DayPeriod] inputs and one output.
      if(format == Constants::TrainSetFormat::PeriodDayAhead)
        trainingSets_.last()->appendRow(priceWindow, calculateOutputDayAhead(aheadPriceWindow));
      else if(format == Constants::TrainSetFormat::PeriodAverage)
        trainingSets_.last()->appendRow(priceWindow, calculateOutputAverage(aheadPriceWindow));
    }
  }
  const QString fullname = fullDataTrainFileName(*trainingSets_.last());
  trainingSets_.last()->saveToFile(fullname);
}

void NeuralNet::movingPriceWindow(QMap<QDateTime, double>::const_iterator itr, std::deque<double>& priceWindow, unsigned int DayPeriod)
{
  // This will (unless value is wrong) keep the prices between [0, 1] which is important for the functions I use.
  const double ptprice = itr.value() / MaxPriceExpected_;
  assert(Util::lessEqualAbs(ptprice, 1.0));
  priceWindow.push_back(ptprice);
  if(priceWindow.size() > DayPeriod)
    priceWindow.pop_front();
}

void NeuralNet::train()
{
  for(DataTrainSet* set : trainingSets_)
    trainSet(*set);
}

void NeuralNet::trainSet(const QString& setname)
{
  for(DataTrainSet* set : trainingSets_)
    if(set->name().compare(setname)==0) trainSet(*set);
}

void NeuralNet::trainSet(DataTrainSet &set)
{
  const unsigned int num_input = config_.DayPeriod_;
  const unsigned int num_output = NumOutputs_;
  const unsigned int num_layers = NumLayers_;
  const unsigned int num_neurons_hidden = NumNeuronsHidden_;
  const float desired_error = DesiredError_;
  const unsigned int max_epochs = MaxEpochs_;
  const unsigned int epochs_between_reports = EpochsBetweenReports_;


  FANN::neural_net nn;
  if(!nn.create_standard(num_layers, num_input, num_neurons_hidden, num_output))
  {
    // something bad happened.
    assert(false);
  }

  nn.set_activation_function_hidden(FANN::SIGMOID_SYMMETRIC);
  nn.set_activation_function_output(FANN::SIGMOID_SYMMETRIC);

  nn.train_on_file(fullDataTrainFileName(set).toStdString(), max_epochs, epochs_between_reports, desired_error);
  if(!nn.save(fullNeuralFileName(set.name()).toStdString()))
  {
    Logger::get().append("NeuralNet::TrainSet: Error saving .net file after training.", true);
  }
  nn.destroy();
}

double NeuralNet::estimateAveragePriceNextPeriod(const std::vector<double>& priceWindow)
{
  FANN::neural_net nn;
  if(!nn.create_from_file("testset_avg_2011_2013_daily_NOK.net")) Logger::get().append("NN.EstimateAvgPriceNextPeriod: could probably not find .net file.");

  fann_type freq[priceWindow.size()];
  for(unsigned int i = 0; i < priceWindow.size(); i++)
    freq[i] = priceWindow[i];
  fann_type* output = nn.run(freq);
  // Output shouldn't hold more than one value, but in case we'd change to more later, make sure it won't break. Oh and scale it back up.
  double out =  output[0]*MaxPriceExpected_;
  return out;
}

double NeuralNet::estimateDayAheadPrice(const std::vector<double> &priceWindow)
{
  FANN::neural_net nn;
  if(!nn.create_from_file("testset_dayahead_2011_2013_daily_NOK.net")) Logger::get().append("NN.EstimateAvgPriceNextPeriod: could probably not find .net file.");

  fann_type freq[priceWindow.size()];
  for(unsigned int i = 0; i < priceWindow.size(); i++)
    freq[i] = priceWindow[i];
  fann_type* output = nn.run(freq);
  // Output shouldn't hold more than one value, but in case we'd change to more later, make sure it won't break. Oh and scale it back up.
  double out =  output[0]*MaxPriceExpected_;
  return out;
}

//void NeuralNet::estimateNextPeriod(const QMap<QDateTime, double>& spotprices, QDateTime startdate, unsigned int numDaysPeriod, unsigned int numDaysAhead)
//{
//  std::deque<double> priceWindow;
//  std::deque<double> aheadPriceWindow;
//  slidingWindowPrices(spotprices, startdate, numDaysPeriod, numDaysAhead, priceWindow, aheadPriceWindow);
//  FANN::neural_net nn;
//  nn.create_from_file(fullNeuralFileName().toStdString());

//  fann_type freq[priceWindow.size()];
//  double avgCurrPeriod = 0.0;
//  for(unsigned int i = 0; i < priceWindow.size(); i++)
//  {
//    freq[i] = priceWindow[i];
//    avgCurrPeriod += priceWindow[i];
//  }
//  avgCurrPeriod /= priceWindow.size();
//  avgCurrPeriod *= MaxPriceExpected_;

//  const double actualAvgNextPeriod = (std::accumulate(aheadPriceWindow.begin(), aheadPriceWindow.end(), 0.0) / aheadPriceWindow.size())*MaxPriceExpected_;

//  fann_type* output = nn.run(freq);
//  double predAvgNextPeriod = output[0]*MaxPriceExpected_;
//  qDebug() << "Predicted price avg next cycle: "  << predAvgNextPeriod;
//  qDebug() << "Actual price avg next cycle: " << actualAvgNextPeriod;
//  qDebug() << "Avg current cycle:" << avgCurrPeriod;
//  qDebug() << "Percent error: " << predAvgNextPeriod / actualAvgNextPeriod;
//}

//void NeuralNet::slidingWindowPrices(const QMap<QDateTime, double>& spotprices, QDateTime start, unsigned int numDaysPeriod, unsigned int numDaysAhead,
//    std::deque<double>& priceWindow, std::deque<double>& aheadPriceWindow)
//{
//  QDateTime aheadDate_start = start.addDays(numDaysAhead);
//  assert(spotprices.size() > static_cast<int>(numDaysPeriod) && "period less than window");
//  auto ahead_itr = spotprices.begin() + numDaysAhead;
//  unsigned int numRows = 0;
//  for(auto itr = spotprices.begin(); itr != spotprices.end(); ++itr)
//  {
//    numRows++;
//    // Make sure the window ahead isn't accessing out of bounds, as well as checking if the windows has filled up.
//    if(numDaysPeriod - aheadPriceWindow.size() >= spotprices.size() - numRows || priceWindow.size() >= numDaysPeriod)
//    {
//      qDebug() << "window filled up or at end. Breaking out.";
//      break;
//    }
//    windowPriceEstimation(itr, priceWindow, start, numDaysPeriod);
//    windowPriceEstimation(++ahead_itr, aheadPriceWindow, aheadDate_start, numDaysPeriod);
//  }

//  assert(priceWindow.size() == DayAheadLong_ && aheadPriceWindow.size() == DayAheadLong_ && "unexpected number of values.");
//}

//void NeuralNet::windowPriceEstimation(QMap<QDateTime, double>::const_iterator itr, std::deque<double> &priceWindow, QDateTime start, unsigned int numDaysPeriod)
//{
//  QDateTime endPeriod = start.addDays(numDaysPeriod);
//  if(itr.key() >= start && itr.key() < endPeriod)
//    window(itr, priceWindow);
//}

QString NeuralNet::fullDataTrainFileName(const DataTrainSet &set) const
{
  return this->Nameset_ + "_" + set.name() + ".train";
}

QString NeuralNet::fullNeuralFileName(QString setname) const
{
  return this->Nameset_ + "_" + setname + ".net";
}
