#ifndef ASSETSMANAGER_H
#define ASSETSMANAGER_H

#include "transactionlogger.h"
#include <memory>
#include <deque>
#include "constants.h"
#include <Config.h>
#include "Order.h"

class AssetsManager
{
public:
  AssetsManager(std::shared_ptr<Config> config, std::shared_ptr<TransactionLogger> log);

  bool setupBuyEnergyOrder(double amount, double predictedPrice);
  bool setupSellEnergy(double amount, double predictedPrice);
  void setupAvoidBuyingEnergy(double predictedPrice);
  void setupAvoidSellingEnergy(double predictedPrice);

  double money() const { return config_->assetsConfig().money(); }
  double energy() const { return config_->assetsConfig().energy(); }
  double realSystemPrice() const { return config_->assetsConfig().lastSysPrice(); }
  void setMoney(double money);
  void setEnergy(double energy);
  void setRealSystemPrice(double sysPriceReal);
  void completeRemainingTransactions();

  bool NegativeMoneyFlag() const { return money() < Constants::ApproxZeroDouble; }

  bool rule_moneyLowEnergyHigh();
  bool rule_moneyHighEnergyLow();

private:
  std::shared_ptr<Config> config_;
  std::shared_ptr<TransactionLogger> log_;
//  double money_;
//  double energy_;
//  double sysPriceReal_;
  std::deque<Order> remainingOrders_;



  const double StartingMoney = 1000000.0;
  const double StartingEnergy = 500;
  const QString SectionName = "assets";

  // Temp methods that should later hold more rigorous checking typical for any transaction apps.
  void withdrawFunds(double amount) { setMoney(money() - amount); }
  void withdrawEnergy(double amount) { setEnergy(energy() - amount); }
  void appendFunds(double amount) { setMoney(money() + amount); }
  void appendEnergy(double amount) { setEnergy(energy() + amount); }

  unsigned int nextOrderNumber();
  void saveOrders();
  void loadOrders();
};

#endif // ASSETSMANAGER_H
