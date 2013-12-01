#ifndef ASSETSMANAGER_H
#define ASSETSMANAGER_H

#include "transactionlogger.h"
#include <memory>

class AssetsManager
{
public:
  AssetsManager();

  bool buyEnergy(unsigned int amount);
  bool sellEnergy(unsigned int amount);

  double money() const { return money_; }
  double energy() const { return energy_; }
  double realSystemPrice() const { return sysPriceReal_; }
  void setMoney(double money);
  void setEnergy(double energy);
  void setRealSystemPrice(double sysPriceReal) { sysPriceReal_ = sysPriceReal; }

private:
  double money_;
  double energy_;
  std::unique_ptr<TransactionLogger> log_;
  double sysPriceReal_;

  // Temp methods that should later hold more rigorous checking typical for any transaction apps.
  void withdrawFunds(double amount) { money_ -= amount; }
  void withdrawEnergy(double amount) { energy_ -= amount; }
  void appendFunds(double amount) { money_ += amount; }
  void appendEnergy(double amount) { energy_ += amount; }
};

#endif // ASSETSMANAGER_H
