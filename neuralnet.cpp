#include "neuralnet.h"
#include <QDebug>
#include "logger.h"

NeuralNet::NeuralNet(const NeuralConfig& config)
:
  config_(config),
  Nameset_(config.Nameset_), MaxPriceExpected_(config.MaxPriceExpected_), DayAheadShort_(config.DayAheadShort_),
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



void NeuralNet::createTrainSet(const QString& trainSetName, const QMap<QDateTime, double>& spotprices)
{  
  trainingSets_.append(new DataTrainSet(trainSetName, DayAheadLong_, NumOutputs_));

  unsigned int rownum = 0;
  std::vector<double> dayprices;

  std::deque<double> priceWindow;
  std::deque<double> aheadPriceWindow;
  assert(spotprices.size() > static_cast<int>(DayAheadLong_) && "period less than window");
  auto ahead_itr = spotprices.begin() + DayAheadLong_;
  for(auto itr = spotprices.begin(); itr != spotprices.end(); ++itr)
  {
    // Problem: at the end of the dataset will probably have overflow < numDayPeriod.
    // Currently we skip them, but they should really get handled. The dataset expects
    // numdayperiod values, and we cannot just add blank either, because it skews the avg.
    // TODO: Consider a solution.
//    const double ptprice = itr.value() / MaxPriceExpected_;
//    assert(Util::lessEqualAbs(ptprice, 1.0));
//    dayprices.push_back(ptprice);
//    if(++rownum >= numDayPeriod_)
//    {
//      trainingSets_.last()->appendRow(dayprices);
//      dayprices.clear();
//      rownum = 0;
//    }

      ++rownum;
      window(itr, priceWindow);

      if(static_cast<int>(rownum + DayAheadLong_) >= spotprices.size())
      {
        qDebug() << "lookahead at end. breaking out.";
        break;
      }
      window(++ahead_itr, aheadPriceWindow);


      if(priceWindow.size() >= DayAheadLong_)
      {
        assert(priceWindow.size() == DayAheadLong_ && "size 'when full' should be exactly like numDayPeriod.");
        assert(aheadPriceWindow.size() == priceWindow.size() && "unexpected size difference.");

        trainingSets_.last()->appendRow(priceWindow, aheadPriceWindow);
      }
  }
  const QString fullname = fullDataTrainFileName(*trainingSets_.last());
  trainingSets_.last()->saveToFile(fullname);
}

void NeuralNet::window(QMap<QDateTime, double>::const_iterator itr, std::deque<double>& priceWindow)
{
  const double ptprice = itr.value() / MaxPriceExpected_;
  assert(Util::lessEqualAbs(ptprice, 1.0));
  priceWindow.push_back(ptprice);
  if(priceWindow.size() > DayAheadLong_)
    priceWindow.pop_front();
}

void NeuralNet::train()
{
  for(DataTrainSet* set : trainingSets_)
    trainSet(*set);
}

void NeuralNet::trainSet(DataTrainSet &set)
{
  const unsigned int num_input = DayAheadLong_;
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
  if(!nn.save(fullNeuralFileName().toStdString()))
  {
    // errror saving file.
    assert(false);
  }
  nn.destroy();

  qDebug() << "we're done training.";
}

void NeuralNet::estimateNextPeriod(const QMap<QDateTime, double>& spotprices, QDateTime startdate, unsigned int numDaysPeriod, unsigned int numDaysAhead)
{
  std::deque<double> priceWindow;
  std::deque<double> aheadPriceWindow;
  slidingWindowPrices(spotprices, startdate, numDaysPeriod, numDaysAhead, priceWindow, aheadPriceWindow);
  FANN::neural_net nn;
  nn.create_from_file(fullNeuralFileName().toStdString());

  fann_type freq[priceWindow.size()];
  double avgCurrPeriod = 0.0;
  for(unsigned int i = 0; i < priceWindow.size(); i++)
  {
    freq[i] = priceWindow[i];
    avgCurrPeriod += priceWindow[i];
  }
  avgCurrPeriod /= priceWindow.size();
  avgCurrPeriod *= MaxPriceExpected_;

  const double actualAvgNextPeriod = (std::accumulate(aheadPriceWindow.begin(), aheadPriceWindow.end(), 0.0) / aheadPriceWindow.size())*MaxPriceExpected_;

  fann_type* output = nn.run(freq);
  double predAvgNextPeriod = output[0]*MaxPriceExpected_;
  qDebug() << "Predicted price avg next cycle: "  << predAvgNextPeriod;
  qDebug() << "Actual price avg next cycle: " << actualAvgNextPeriod;
  qDebug() << "Avg current cycle:" << avgCurrPeriod;
  qDebug() << "Percent error: " << predAvgNextPeriod / actualAvgNextPeriod;
}

void NeuralNet::slidingWindowPrices(const QMap<QDateTime, double>& spotprices, QDateTime start, unsigned int numDaysPeriod, unsigned int numDaysAhead,
    std::deque<double>& priceWindow, std::deque<double>& aheadPriceWindow)
{
  QDateTime aheadDate_start = start.addDays(numDaysAhead);
  assert(spotprices.size() > static_cast<int>(numDaysPeriod) && "period less than window");
  auto ahead_itr = spotprices.begin() + numDaysAhead;
  unsigned int numRows = 0;
  for(auto itr = spotprices.begin(); itr != spotprices.end(); ++itr)
  {
    numRows++;
    // Make sure the window ahead isn't accessing out of bounds, as well as checking if the windows has filled up.
    if(numDaysPeriod - aheadPriceWindow.size() >= spotprices.size() - numRows || priceWindow.size() >= numDaysPeriod)
    {
      qDebug() << "window filled up or at end. Breaking out.";
      break;
    }
    windowPriceEstimation(itr, priceWindow, start, numDaysPeriod);
    windowPriceEstimation(++ahead_itr, aheadPriceWindow, aheadDate_start, numDaysPeriod);
  }

  assert(priceWindow.size() == DayAheadLong_ && aheadPriceWindow.size() == DayAheadLong_ && "unexpected number of values.");
}

void NeuralNet::windowPriceEstimation(QMap<QDateTime, double>::const_iterator itr, std::deque<double> &priceWindow, QDateTime start, unsigned int numDaysPeriod)
{
  QDateTime endPeriod = start.addDays(numDaysPeriod);
  if(itr.key() >= start && itr.key() < endPeriod)
    window(itr, priceWindow);
}

QString NeuralNet::fullDataTrainFileName(const DataTrainSet &set) const
{
  return this->Nameset_ + "_" + set.name() + ".train";
}

QString NeuralNet::fullNeuralFileName() const
{
  return this->Nameset_ + ".net";
}
