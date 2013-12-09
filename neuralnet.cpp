#include "neuralnet.h"
#include <QDebug>

NeuralNet::NeuralNet(const NeuralConfig& config, std::shared_ptr<ApplicationLogger> log, Ui::MainWindow* window)
:
  config_(config), log_(log),
  Nameset_(config.Nameset_), MaxPriceExpected_(config.MaxPriceExpected_),
  DayAheadLong_(config.DayAheadLong_), NumLayers_(config.NumLayers_), NumNeuronsHidden_(config.NumNeuronsHidden_),
  NumOutputs_(config.NumOutputs_), DesiredError_(config.DesiredError_), MaxEpochs_(config.MaxEpochs_), EpochsBetweenReports_(config.EpochsBetweenReports_)
  {
    neurnet_ = std::unique_ptr<FANN::neural_net>(new FANN::neural_net());
    window_ = window;
  }

NeuralNet::~NeuralNet()
{
  neurnet_->destroy();

  for(DataTrainSet* set : trainingSets_)
    if(set != NULL)
      delete set;

  trainingSets_.clear();
}

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

void NeuralNet::createTrainSet(const QString &trainSetName, const QMap<QDateTime, double> &spotprices, const unsigned int DayPeriod, Constants::TrainSetFormat format)
{
  trainingSets_.append(new DataTrainSet(trainSetName, DayPeriod, NumOutputs_));
  unsigned int rownum = 0;
  std::deque<double> priceWindow;
  std::deque<double> aheadPriceWindow;
  const unsigned int MinPriceRows = DayPeriod + format == Constants::TrainSetFormat::PeriodDayAhead ? 1 : DayPeriod;
  assert(spotprices.size() > static_cast<int>(MinPriceRows) && "periods less than window");
    // Depending on which format we're creating, we determine the iterator offset the one ahead needs to be.
  const unsigned int MinDistAheadItr = format == Constants::TrainSetFormat::PeriodDayAhead ? 0 : DayPeriod;
  auto ahead_itr = spotprices.begin() + MinDistAheadItr;
  for(auto itr = spotprices.begin(); itr != spotprices.end(); ++itr)
  {
    ++rownum;
    movingPriceWindow(itr, priceWindow, DayPeriod);
    // Check if the one ahead has reached the end, and if so, break to loop.
    if(static_cast<int>(rownum + MinPriceRows) >= spotprices.size()) break;

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

double NeuralNet::calculateOutputAverage(const std::deque<double>& aheadPriceWindow)
{
    return std::accumulate(aheadPriceWindow.begin(), aheadPriceWindow.end(), 0.0) / aheadPriceWindow.size();
}

double NeuralNet::calculateOutputDayAhead(const std::deque<double>& aheadPriceWindow)
{
  // Keeping this in its own method in case of later changes. Basically return the last item.
  //double last = aheadPriceWindow.at(aheadPriceWindow.size()-1);
  return aheadPriceWindow.front();
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

  if(!neurnet_->create_standard(num_layers, num_input, num_neurons_hidden, num_output))
  {
    log_->append("NeuralNet.Trainset: " + QString::fromStdString(neurnet_->get_errstr()), true);
    return;
  }

  neurnet_->set_activation_function_hidden(FANN::SIGMOID_SYMMETRIC);
  neurnet_->set_activation_function_output(FANN::SIGMOID_SYMMETRIC);


  neurnet_->set_callback(&NeuralNet::print_callback, (void*)(window_));
  window_->pbTrainingProgress->show();
  neurnet_->train_on_file(fullDataTrainFileName(set).toStdString(), max_epochs, epochs_between_reports, desired_error);
  window_->pbTrainingProgress->hide();
  window_->txtTrainingStatus->append("Done training.");
  if(!neurnet_->save(fullNeuralFileName(set.name()).toStdString()))
  {
    log_->append("NeuralNet::TrainSet: Error saving .net file after training.", true);
  }
}

int NeuralNet::print_callback(FANN::neural_net &net, FANN::training_data &/*train*/, unsigned int /*max_epochs*/, unsigned int /*epochs_between_reports*/, float /*desired_error*/, unsigned int epochs, void* user_data)
{
  Ui::MainWindow* nn = (Ui::MainWindow*)user_data;
  nn->txtTrainingStatus->append(QString("Epoch: %1. Current error: %2.").arg(QString::number(epochs), QString::number(net.get_MSE(), 'G', 8)));
  nn->pbTrainingProgress->setValue(epochs);
  return 0;
}

double NeuralNet::estimateAveragePriceNextPeriod(const std::vector<double>& priceWindow)
{
  if(!neurnet_->create_from_file("testset_avg_2011_2013_daily_NOK.net")) log_->append("NN.EstimateAvgPriceNextPeriod: could probably not find .net file.", true);

  fann_type freq[priceWindow.size()];
  for(unsigned int i = 0; i < priceWindow.size(); i++)
    freq[i] = priceWindow[i] / MaxPriceExpected_;
  fann_type* output = neurnet_->run(freq);
  // Output shouldn't hold more than one value, but in case we'd change to more later, make sure it won't break. Oh and scale it back up.
  double out =  output[0]*MaxPriceExpected_;
  return out;
}

double NeuralNet::estimateDayAheadPrice(const std::vector<double> &priceWindow)
{
  if(!neurnet_->create_from_file("testset_dayahead_2011_2013_daily_NOK.net")) log_->append("NN.EstimateDayAheadPrice: could probably not find .net file.", true);

  fann_type freq[priceWindow.size()];
  for(unsigned int i = 0; i < priceWindow.size(); i++)
    freq[i] = priceWindow[i] / MaxPriceExpected_;
  fann_type* output = neurnet_->run(freq);
  // Output shouldn't hold more than one value, but in case we'd change to more later, make sure it won't break. Oh and scale it back up.
  double out =  output[0]*MaxPriceExpected_;
  return out;
}

QString NeuralNet::fullDataTrainFileName(const DataTrainSet &set) const
{
  return this->Nameset_ + "_" + set.name() + ".train";
}

QString NeuralNet::fullNeuralFileName(QString setname) const
{
  return this->Nameset_ + "_" + setname + ".net";
}
