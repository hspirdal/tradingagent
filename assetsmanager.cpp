#include "assetsmanager.h"
#include "constants.h"
#include "util.h"

AssetsManager::AssetsManager(std::shared_ptr<Config> config)
  : config_(config), money_(0.0), energy_(0), sysPriceReal_(0.0),
  log_(new TransactionLogger("transaction.log", config_->agentInfoConfig()))
{
}

bool AssetsManager::setupBuyEnergyOrder(double amount, double predictedPrice)
{
  Order order(amount, 0.0, predictedPrice, sysPriceReal_,  config_->nextOrderNumber());
  remainingOrders_.push(order);
  log_->logBuyEnergyOrder(order);

  return true;
}

bool AssetsManager::setupSellEnergy(double amount, double predictedPrice)
{
  // The system might allow for the agent to get negative money because number of assets will be bought the next day no matter how the price changes.
  // The rules allow for this, but it might be unfortunate as the rules dicate that we would have to sell all energy on the following turn or lose.

  Order order(0.0, amount, predictedPrice, sysPriceReal_, config_->nextOrderNumber());
  remainingOrders_.push((order));
  log_->logSellEnergyOrder(order);

  return true;
}

void AssetsManager::setRealSystemPrice(double sysPriceReal)
{
  sysPriceReal_ = sysPriceReal;
  config_->setValue("Assets", "lastSysPrice", sysPriceReal_);
  Util::writeFile("config.ini", config_->toString().toStdString(), true);
}

void AssetsManager::setMoney(double money)
{
  money_ = money;
  config_->setValue("Assets", "money" , money_);
  Util::writeFile("config.ini", config_->toString().toStdString(), true);
  qDebug() << "money:" << config_->value("assets", "money");
}

void AssetsManager::setEnergy(double energy)
{
  energy_ = energy;
  config_->setValue("Assets", "energy" , energy_);
  Util::writeFile("config.ini", config_->toString().toStdString(), true);
}

bool AssetsManager::rule_moneyHighEnergyLow()
{
  double percentMoneyLeft = money() / StartingMoney;
  double percentEnergyLeft = energy() / StartingEnergy;
  double ratio = percentMoneyLeft / percentEnergyLeft;
  return ratio < 0.25;
}

bool AssetsManager::rule_moneyLowEnergyHigh()
{
  double percentMoneyLeft = money() / StartingMoney;
  double percentEnergyLeft = energy() / StartingEnergy;
  double ratio = percentEnergyLeft / percentMoneyLeft;
  return ratio < 0.25;
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
      log_->logTransferBoughtEnergy(order, energy(), money(), sysPriceReal_);
    }
    else if(order.soldAmountEnergy_)
    {
      const double moneyBack = order.soldAmountEnergy_ * realSystemPrice();
      this->withdrawEnergy(order.soldAmountEnergy_);
      this->appendFunds(moneyBack);
      log_->logTransferSoldEnergy(order, energy(), money(), sysPriceReal_);
    }
    remainingOrders_.pop();
    config_->setValue("Assets", "money" , money_);
    config_->setValue("Assets", "energy" , energy_);
    config_->setValue("Assets", "lastSysPrice" , sysPriceReal_);
    Util::writeFile("config.ini", config_->toString().toStdString(), true);
  }
}


