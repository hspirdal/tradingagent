#include "assetsmanager.h"

AssetsManager::AssetsManager()
  : money_(0.0), energy_(0), sysPriceReal_(0.0), log_(new TransactionLogger("transaction.log", "nunyah@gmail.com"))
{
}

bool AssetsManager::buyEnergy(unsigned int amount)
{

  double priceTotal = sysPriceReal_ * amount;
  if(priceTotal > money_)
  {
    log_->insufficientFundsBuying(amount, sysPriceReal_, money());
    return false;
  }
  withdrawFunds(priceTotal);
  appendEnergy(amount);
  log_->appendBuyEnergy(amount, sysPriceReal_, money(), energy());
  return true;
}

bool AssetsManager::sellEnergy(unsigned int amount)
{
  if(amount > energy())
  {
    log_->insufficientEnergySelling(amount, sysPriceReal_, energy());
    return false;
  }
  withdrawEnergy(amount);
  appendFunds(amount * sysPriceReal_);
  log_->appendSellEnergy(amount, sysPriceReal_, money(), energy());
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


