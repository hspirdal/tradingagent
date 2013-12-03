#include "agentcontroller.h"


AgentController::AgentController(std::shared_ptr<Config> config, std::shared_ptr<NeuralNet> neurnet, std::shared_ptr<AssetsManager> assets)
: agentInfoConfig_(config.get()->agentInfoConfig()), config_(config), logger_(new Logger("log.log")), neurnet_(neurnet), assets_(assets)
{
  resetFlags();
  // track days gone past, and use it to become more aggressive after a while.
  dateStarted_ = QDateTime::currentDateTime();
}

void AgentController::predictPriceAhead()
{
  double futureavg = neurnet_->estimateAveragePriceNextPeriod(this->latestDailyPrices_);
  double dayAheadprice = neurnet_->estimateDayAheadPrice(this->latestDailyPrices_);
  qDebug() << "future avg: " << futureavg;
  qDebug() << "dayahead:" << dayAheadprice;
  hasPredictedAndMadeOrder_ = true;

  // Check and handle disaster.
  if(assets_->NegativeMoneyFlag())
  {
    logger_->append("AgentController.PredictPriceAhead: NegativeMoneyFlag encountered!", true);
    assets_->setupSellEnergy(assets_->energy(), dayAheadprice);
    return;
  }
  const double ratio = dayAheadprice / assets_->realSystemPrice();
  if(ratio > 1.0)
  {
    // Do not buy if cash reserves are low, if energy amount is still fairly high.
    if(assets_->rule_moneyLowEnergyHigh())
      return;

    // Depending on scale and probability, this could be a good time to buy.
    const double percent = Util::clamp(ratio - 1, Constants::ApproxZeroDouble, 0.25); // never buy more than 25% of total money assets.
    const double moneyToSpend = assets_->money() * percent;
    const double unitsToBuy = moneyToSpend / assets_->realSystemPrice();
    assets_->setupBuyEnergyOrder(unitsToBuy, dayAheadprice);
  }
  else
  {
    // Don't sell off too much energy, unless it's getting further out in the competition.
    if(assets_->rule_moneyHighEnergyLow() || currentDate().date().day() < 15)
      return;

    // like above, it can be profitable to sell if the change is big enough.
    const double percent = Util::clamp(ratio - 1, Constants::ApproxZeroDouble, 0.25); // never buy more than 25% of total money assets.
    const double energyToSell = assets_->energy() * percent;
    assets_->setupSellEnergy(energyToSell, dayAheadprice);
  }
}

void AgentController::createAndTrainSet(const QString &trainSetName, const QMap<QDateTime, double> &spotprices)
{
  const QString avg = "avg_" + trainSetName;
  neurnet_.get()->createTrainSetAverage(avg, spotprices, config_.get()->neuralConfig().DayAheadLong_);
  neurnet_.get()->trainSet(avg);
  logger_.get()->append("CreateAndTrainSet: created and trained file named " + avg, true);

  const QString ahead = "dayahead_" + trainSetName;
  neurnet_.get()->createTrainSetDayAhead(ahead, spotprices, config_.get()->neuralConfig().DayPeriod_);
  neurnet_.get()->trainSet(ahead);
  logger_.get()->append("CreateAndTrainSet: created and trained file named " + ahead, true);
}

void AgentController::setLatestDailyPrices(const QMap<QDateTime, double>& latestDaily)
{
  latestDailyPrices_.push_back(this->assets_.get()->realSystemPrice());
  // Only want the last [num] ones.
  QMapIterator<QDateTime, double> i(latestDaily);
  i.toBack();
  while(i.hasPrevious())
  {
    i.previous();
    qDebug() << i.key() << i.value();
    latestDailyPrices_.push_back(i.value());
    if(latestDailyPrices_.size() >= config_.get()->neuralConfig().DayPeriod_)
      break;
  }
  // Reverse it back i correct order.
  std::reverse(latestDailyPrices_.begin(), latestDailyPrices_.end());
  isFreshPriceData_ = true;
}

void AgentController::resetFlags()
{
  currentTime_ = QDateTime::currentDateTime();
  isFreshPriceData_ = false;
  isFreshSystemPrice_ = false;
  hasPredictedAndMadeOrder_ = false;
  hasUpdatedAssetReserves_ = false;
  agentSleepingUntilNextDay_ = false;

  latestDailyPrices_.clear();
}

void AgentController::completeRemainingTransactions()
{
  // TODO curr date
  hasUpdatedAssetReserves_ = true;
  agentSleepingUntilNextDay_ = true;
  assets_->completeRemainingTransactions();
}

bool AgentController::tryToWake()
{
  if(currentTime_.date().day() <= QDateTime::currentDateTime().date().day())
    return false;

  // It is next day. Time to clear all prev flags.
  resetFlags();
  return true;
}
