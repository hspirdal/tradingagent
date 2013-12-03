#include "assetsmanager.h"
#include "constants.h"

AssetsManager::AssetsManager(std::shared_ptr<Config> config)
  : config_(config), money_(0.0), energy_(0), sysPriceReal_(0.0),
  log_(new TransactionLogger("transaction.log", config_->agentInfoConfig()))
{
}

bool AssetsManager::setupBuyEnergyOrder(double amount, double predictedPrice)
{
  // TODO LOG

  Order order(amount, 0.0, predictedPrice, sysPriceReal_,  config_->nextOrderNumber());
  remainingOrders_.push(order);

  return true;
}

bool AssetsManager::setupSellEnergy(double amount, double predictedPrice)
{
  // if energy happen to be more than we have, we'll just sell as much as we can (per the rules), but this should not happen.

  // TODO LOG

  Order order(0.0, amount, predictedPrice, sysPriceReal_, config_->nextOrderNumber());
  remainingOrders_.push((order));

  return true;
}

void AssetsManager::setMoney(double money)
{
  money_ = money;
}

void AssetsManager::setEnergy(double energy)
{
  energy_ = energy;
}

bool AssetsManager::rule_moneyHighEnergyLow()
{
  double percentMoneyLeft = money() / StartingMoney;
  double percentEnergyLeft = energy() / StartingEnergy;
  double ratio = percentMoneyLeft / percentEnergyLeft;
  return ratio > 0.25;
}

bool AssetsManager::rule_moneyLowEnergyHigh()
{
  double percentMoneyLeft = money() / StartingMoney;
  double percentEnergyLeft = energy() / StartingEnergy;
  double ratio = percentEnergyLeft / percentMoneyLeft;
  return ratio > 0.25;
}

void AssetsManager::completeRemainingTransactions()
{
  // TODO LOG.
  while(remainingOrders_.size() > 0)
  {
    auto order = remainingOrders_.front();
    // Bought energy, thus we couldn't have sold energy.
    if(order.boughtAmountEnergy_ > Constants::ApproxZeroDouble)
    {
      const double total = order.boughtAmountEnergy_ * realSystemPrice();
      this->withdrawFunds(total);
      this->appendEnergy(order.boughtAmountEnergy_);
    }
    else if(order.soldAmountEnergy_)
    {
      const double moneyBack = order.soldAmountEnergy_ * realSystemPrice();
      this->withdrawEnergy(order.soldAmountEnergy_);
      this->appendFunds(moneyBack);
    }
    remainingOrders_.pop();
  }
}


