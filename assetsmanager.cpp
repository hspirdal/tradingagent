#include "assetsmanager.h"
#include "constants.h"
#include "util.h"

// TODO: Having to save per call anytime we set config values is error prone.

AssetsManager::AssetsManager(std::shared_ptr<Config> config, std::shared_ptr<TransactionLogger> log)
  : config_(config), log_(log)
{
  loadOrders();
}

unsigned int AssetsManager::nextOrderNumber()
{
  unsigned int next = config_->assetsConfig().orderNumber() + 1;
  config_->assetsConfig().setOrderNumber(next);
  config_->save();
  return next;
}

void AssetsManager::saveOrders()
{
  QString content;
  for(Order order : remainingOrders_)
    content.append(order.toString());

  // All orders should be in memory; overwrite.
  Util::writeFile("orders.bak", content.toStdString(), true);
}

void AssetsManager::loadOrders()
{
  remainingOrders_= Util::loadOrderFile("orders.bak");
}

bool AssetsManager::setupBuyEnergyOrder(double amount, double predictedPrice)
{
  Order order(amount, 0.0, predictedPrice, realSystemPrice(),  nextOrderNumber());
  remainingOrders_.push_back(order);
  log_->logBuyEnergyOrder(order);
  saveOrders();

  return true;
}

bool AssetsManager::setupSellEnergy(double amount, double predictedPrice)
{
  // The system might allow for the agent to get negative money because number of assets will be bought the next day no matter how the price changes.
  // The rules allow for this, but it might be unfortunate as the rules dicate that we would have to sell all energy on the following turn or lose.

  Order order(0.0, amount, predictedPrice, realSystemPrice(), nextOrderNumber());
  remainingOrders_.push_back((order));
  log_->logSellEnergyOrder(order);
  saveOrders();

  return true;
}

void AssetsManager::setupAvoidBuyingEnergy(double predictedPrice)
{
  log_->logAvoidedBuyingEnergy(predictedPrice, energy(), money());
}

void AssetsManager::setupAvoidSellingEnergy(double predictedPrice)
{
  log_->logAvoidedSellingEnergy(predictedPrice, energy(), money());
}

void AssetsManager::setRealSystemPrice(double sysPriceReal)
{
  config_->assetsConfig().setLastSysPrice(sysPriceReal);
  config_->save();
}

void AssetsManager::setMoney(double money)
{
  config_->assetsConfig().setMoney(money);
  config_->save();
}

void AssetsManager::setEnergy(double energy)
{
  config_->assetsConfig().setEnergy(energy);
  config_->save();
}

bool AssetsManager::rule_moneyHighEnergyLow()
{
  double percentMoneyLeft = money() / StartingMoney;
  double percentEnergyLeft = energy() / StartingEnergy;
  //double ratio = percentMoneyLeft / percentEnergyLeft;
  return percentEnergyLeft < 0.25 && percentMoneyLeft > 0.25;
}

bool AssetsManager::rule_moneyLowEnergyHigh()
{
  double percentMoneyLeft = money() / StartingMoney;
  double percentEnergyLeft = energy() / StartingEnergy;
  //double ratio = percentEnergyLeft / percentMoneyLeft;
  return percentMoneyLeft < 0.25 && percentEnergyLeft > 0.25;
}

void AssetsManager::completeRemainingTransactions()
{
  if(remainingOrders_.size() > 1)
    Logger::get().append("AssetManager.CompleteRemainingTransactions: number of orders was more than 1, which is unexpected.");
  while(remainingOrders_.size() > 0)
  {
    auto order = remainingOrders_.front();
    // Bought energy, thus we couldn't have sold energy.
    if(order.boughtAmountEnergy_ > Constants::ApproxZeroDouble)
    {
      const double total = order.boughtAmountEnergy_ * realSystemPrice();
      this->withdrawFunds(total);
      this->appendEnergy(order.boughtAmountEnergy_);
      log_->logTransferBoughtEnergy(order, energy(), money(), realSystemPrice());
    }
    else if(order.soldAmountEnergy_)
    {
      const double moneyBack = order.soldAmountEnergy_ * realSystemPrice();
      this->withdrawEnergy(order.soldAmountEnergy_);
      this->appendFunds(moneyBack);
      log_->logTransferSoldEnergy(order, energy(), money(), realSystemPrice());
    }
    remainingOrders_.pop_front();
//    config_->assetsConfig().setMoney(money_);
//    config_->assetsConfig().setValue("energy" , energy_);
//    config_->assetsConfig().setValue("lastSysPrice" , sysPriceReal_);
    saveOrders();
  }
}


