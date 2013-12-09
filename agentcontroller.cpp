#include "agentcontroller.h"


AgentController::AgentController(std::shared_ptr<Config> config, std::shared_ptr<ApplicationLogger> log, std::shared_ptr<NeuralNet> neurnet, std::shared_ptr<AssetsManager> assets)
: config_(config), log_(log), neurnet_(neurnet), assets_(assets)
{
  // Wouldn't want to reset various flags if program was restarted for some reason.
  if(config_->agentInfoConfig().ResetFlagsOnStartup_)
    resetFlags();

  // track days gone past, and use it to become more aggressive after a while.
  dateStarted_ = QDateTime::currentDateTime();
}

void AgentController::predictPriceAhead()
{ 
  if(hasMadeOrder())
  {
    log_->append("AgentController.PredictPriceAhead: Already predicted this day. Should only happen once per day.", true);
    return;
  }

  const double futureavg = neurnet_->estimateAveragePriceNextPeriod(this->latestDailyPrices_);
  const double curravg = std::accumulate(latestDailyPrices_.begin(), latestDailyPrices_.end(), 0.0) / latestDailyPrices_.size();

  double dayAheadprice = neurnet_->estimateDayAheadPrice(this->latestDailyPrices_);
  qDebug() << "future avg: " << futureavg;
  qDebug() << "current avg: " << curravg;
  qDebug() << "dayahead:" << dayAheadprice;
  dayAheadprice = dayAheadprice - (dayAheadprice - futureavg) / 2.0;
  qDebug() << "medianshortlongterm: " << dayAheadprice;
  setHasMadeOrder(true);
  log_->append("Placed order", true);

  // Check and handle disaster.
  if(assets_->NegativeMoneyFlag())
  {
    log_->append("AgentController.PredictPriceAhead: NegativeMoneyFlag encountered!", true);
    assets_->setupSellEnergy(assets_->energy(), dayAheadprice);
    return;
  }
  const double ratio = dayAheadprice / assets_->realSystemPrice();
  // Buy energy only is expected price is less than system
  if(ratio < 1.0)
  {
    // Do not buy if cash reserves are low, if energy amount is still fairly high.
    if(assets_->rule_moneyLowEnergyHigh())
    {
      // Would still need to make a record of it, though.
      assets_->setupAvoidBuyingEnergy(dayAheadprice);
      setHasCompletedTransaction(true);
      return;
    }

    // Depending on scale and probability, this could be a good time to buy.
    // never buy more than [max] of total money assets.
    const double percent = Util::clamp(1-ratio, Constants::ApproxZeroDouble, config_->agentInfoConfig().MaxMoneySpend_);
    const double moneyToSpend = assets_->money() * percent;
    const double unitsToBuy = moneyToSpend / assets_->realSystemPrice();
    assets_->setupBuyEnergyOrder(unitsToBuy, dayAheadprice);
  }
  else
  {
    // Don't sell off too much energy
    if(assets_->rule_moneyHighEnergyLow())
    {
      // Would still need to make a record of it, though.
      assets_->setupAvoidSellingEnergy(dayAheadprice);
      setHasCompletedTransaction(true);
      return;
    }

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
  log_.get()->append("CreateAndTrainSet: created and trained file named " + avg, true);

  const QString ahead = "dayahead_" + trainSetName;
  neurnet_.get()->createTrainSetDayAhead(ahead, spotprices, config_.get()->neuralConfig().DayPeriod_);
  neurnet_.get()->trainSet(ahead);
  log_.get()->append("CreateAndTrainSet: created and trained file named " + ahead, true);
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
    latestDailyPrices_.push_back(i.value());
    if(latestDailyPrices_.size() >= config_.get()->neuralConfig().DayPeriod_)
      break;
  }
  // Reverse it back i correct order.
  std::reverse(latestDailyPrices_.begin(), latestDailyPrices_.end());
  isFreshPriceData_ = true;
  log_->append("Updated fresh system prices for the last " + QString::number(config_->neuralConfig().DayPeriod_) + " days.", true);
}

void AgentController::resetFlags()
{
  //currentTime_ = QDateTime::currentDateTime();
  isFreshPriceData_ = false;
  isFreshSystemPrice_ = false;
  setHasMadeOrder(false);
  setHasCompletedTransaction(false);
  setSleeping(false);
  setCurrentDay(static_cast<unsigned int>(QDateTime::currentDateTime().date().day()));

  latestDailyPrices_.clear();
  log_->append("AgentController flags reset.", true);
}

void AgentController::completeRemainingTransactions()
{
  if(!hasCompletedTransaction() && QDateTime::currentDateTime().time().hour() >= config_->miscConfig().Time_Complete_Transaction_.hour())
  {
    setHasCompletedTransaction(true);
    assets_->completeRemainingTransactions();
  }
  else
    log_->append("AgentController.CompleteRemainingTransactions: Tried to complete a transaction while flagged as already done!", true);
}

bool AgentController::tryToWake()
{
  if(config_->agentInfoConfig().CurrentDay_ < static_cast<unsigned int>(QDateTime::currentDateTime().date().day()))
  {
    // It is next day. Time to clear all prev flags.
    resetFlags();
    return true;
  }
  return false;
}

void AgentController::setSystemPrice(double systemPrice)
{
  assets_->setRealSystemPrice(systemPrice);
  isFreshSystemPrice_ = true;
  log_->append("AgentController - Updated the system price.", true);
}

void AgentController::setHasMadeOrder(bool hasMadeOrder)
{
  config_->setValue("agentinfo", "hasMadeOrder", static_cast<unsigned int>(hasMadeOrder ? 1:0));
}

void AgentController::setHasCompletedTransaction(bool hasCompletedTransaction)
{
  config_->setValue("agentinfo", "hasCompletedTransaction", static_cast<unsigned int>(hasCompletedTransaction ? 1:0));
  setSleeping(hasCompletedTransaction);
}
void AgentController::setSleeping(bool sleeping)
{
  config_->setValue("agentinfo", "isAgentSleeping", static_cast<unsigned int>(sleeping ? 1:0));
  if(sleeping)
    log_->append("Agent going to sleep until next calendar day.", true);
  else
    log_->append("Agent no longer at sleep.", true);
}
void AgentController::setCurrentDay(unsigned int day)
{
  config_->setValue("agentinfo", "currentDay", day);
}
