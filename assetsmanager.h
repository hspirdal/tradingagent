#ifndef ASSETSMANAGER_H
#define ASSETSMANAGER_H

#include "transactionlogger.h"
#include <memory>
#include <queue>
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

  double money() const { return money_; }
  double energy() const { return energy_; }
  double realSystemPrice() const { return sysPriceReal_; }
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
  double money_;
  double energy_;
  double sysPriceReal_;
  std::queue<Order> remainingOrders_;



  const double StartingMoney = 1000000.0;
  const double StartingEnergy = 500;
  const QString SectionName = "assets";

  // Temp methods that should later hold more rigorous checking typical for any transaction apps.
  void withdrawFunds(double amount) { money_ -= amount; }
  void withdrawEnergy(double amount) { energy_ -= amount; }
  void appendFunds(double amount) { money_ += amount; }
  void appendEnergy(double amount) { energy_ += amount; }

  unsigned int nextOrderNumber();
};

#endif // ASSETSMANAGER_H
