#include "neuralnet.h"
#include <QDebug>


NeuralNet::NeuralNet(QString nameset, unsigned int numDayPeriod)
: nameset_(nameset), MaxPriceExpected_(1000.0), numDayPeriod_(numDayPeriod), numOutputs_(1.0)
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
  trainingSets_.append(new DataTrainSet(trainSetName, numDayPeriod_, numOutputs_));

  unsigned int rownum = 0;
  std::vector<double> dayprices;

  std::deque<double> priceWindow;
  std::deque<double> aheadPriceWindow;
  assert(spotprices.size() > static_cast<int>(numDayPeriod_) && "period less than window");
  auto ahead_itr = spotprices.begin() + numDayPeriod_;
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

      if(static_cast<int>(rownum + numDayPeriod_) >= spotprices.size())
      {
        qDebug() << "lookahead at end. breaking out.";
        break;
      }
      window(++ahead_itr, aheadPriceWindow);


      if(priceWindow.size() >= numDayPeriod_)
      {
        assert(priceWindow.size() == numDayPeriod_ && "size 'when full' should be exactly like numDayPeriod.");
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
  if(priceWindow.size() > numDayPeriod_)
    priceWindow.pop_front();
}

void NeuralNet::train()
{
  for(DataTrainSet* set : trainingSets_)
    trainSet(*set);
}

void NeuralNet::trainSet(DataTrainSet &set)
{
  const unsigned int num_input = numDayPeriod_;
  const unsigned int num_output = numOutputs_;
  const unsigned int num_layers = 3;
  const unsigned int num_neurons_hidden = 16;
  const float desired_error = (const float) 0.0001;
  const unsigned int max_epochs = 10000;
  const unsigned int epochs_between_reports = 10;


  FANN::neural_net nn;
  if(!nn.create_standard(num_layers, num_input, num_neurons_hidden, num_output))
  {
    // something bad happened.
    assert(false);
  }

  qDebug() << "create_std ran";

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

void NeuralNet::estimateNextPeriod(const QMap<QDateTime, double>& spotprices, QDateTime start, unsigned int numDaysPeriod)
{
  std::deque<double> priceWindow;
  std::deque<double> aheadPriceWindow;
  slidingWindowPrices(spotprices, start, numDaysPeriod, priceWindow, aheadPriceWindow);
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

void NeuralNet::windowPriceEstimation(QMap<QDateTime, double>::const_iterator itr, std::deque<double> &priceWindow, QDateTime start, unsigned int numDaysPeriod)
{
  QDateTime endPeriod = start.addDays(numDaysPeriod);
  if(itr.key() >= start && itr.key() < endPeriod)
    window(itr, priceWindow);
}

void NeuralNet::slidingWindowPrices(const QMap<QDateTime, double>& spotprices, QDateTime start, unsigned int numDaysPeriod,
    std::deque<double>& priceWindow, std::deque<double>& aheadPriceWindow)
{
  QDateTime aheadDate_start = start.addDays(numDaysPeriod);
  assert(spotprices.size() > static_cast<int>(numDaysPeriod) && "period less than window");
  auto ahead_itr = spotprices.begin() + numDaysPeriod;
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

  assert(priceWindow.size() == numDaysPeriod && aheadPriceWindow.size() == numDaysPeriod && "unexpected number of values.");



//  std::vector<double> dayprices;
//  QDateTime endPeriod = start.addDays(numDaysPeriod);
//  for(auto itr = spotprices.begin(); itr != spotprices.end(); ++itr)
//  {
//    if(itr.key() >= start && itr.key() < endPeriod)
//    {
//      const double ptprice = itr.value() / MaxPriceExpected_;
//      assert(Util::lessEqualAbs(ptprice, 1.0));
//      dayprices.push_back(ptprice);
//    }
//  }
//  assert(dayprices.size() == numDaysPeriod && "unexpected number of values.");
//  return dayprices;
}

QString NeuralNet::fullDataTrainFileName(const DataTrainSet &set) const
{
  return this->nameset_ + "_" + set.name() + ".train";
}

QString NeuralNet::fullNeuralFileName() const
{
  return this->nameset_ + ".net";
}
