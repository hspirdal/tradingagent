#include "agentcontroller.h"


AgentController::AgentController(std::shared_ptr<Config> config, std::shared_ptr<NeuralNet> neurnet, std::shared_ptr<AssetsManager> assets)
: config_(config), logger_(new Logger("log.log")), neurnet_(neurnet), assets_(assets)
{
  resetFlags();
  // track days gone past, and use it to become more aggressive after a while.
  dateStarted_ = QDateTime::currentDateTime();
}

void AgentController::predictPriceAhead()
{
  const double futureavg = neurnet_->estimateAveragePriceNextPeriod(this->latestDailyPrices_);
  const double curravg = std::accumulate(latestDailyPrices_.begin(), latestDailyPrices_.end(), 0.0) / latestDailyPrices_.size();
  const double increaseRatioLongTerm = futureavg / curravg;
  if(increaseRatioLongTerm > 1.0)
  {
    // do something smart here.
    qDebug() << "avg should rise";
  }
  else { qDebug() << "avg should sink"; }

  double dayAheadprice = neurnet_->estimateDayAheadPrice(this->latestDailyPrices_);
  qDebug() << "future avg: " << futureavg;
  qDebug() << "current avg: " << curravg;
  qDebug() << "dayahead:" << dayAheadprice;
  setHasMadeOrder(true);

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
    // never buy more than [max] of total money assets.
    const double percent = Util::clamp(ratio - 1, Constants::ApproxZeroDouble, config_->agentInfoConfig().MaxMoneySpend_);
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
    // never sell more than [max] of total money assets.
    const double percent = Util::clamp(ratio - 1, Constants::ApproxZeroDouble, config_->agentInfoConfig().MaxEnergySell_);
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
  setHasMadeOrder(false);
  setHasCompletedTransaction(false);
  setSleeping(false);
  setCurrentDay(static_cast<unsigned int>(currentTime_.date().day()));

  latestDailyPrices_.clear();
}

void AgentController::completeRemainingTransactions()
{
  // TODO curr date
  setHasCompletedTransaction(true);
  setSleeping(true);
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
