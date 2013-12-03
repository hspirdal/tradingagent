#ifndef TRANSACTIONLOGGER_H
#define TRANSACTIONLOGGER_H

#include "logger.h"
#include <memory>
#include "3rdparty/SmtpClient-for-Qt/src/SmtpMime"
#include "Config.h"
#include "Order.h"

class TransactionLogger : public Logger
{
public:
  TransactionLogger(QString logfile, const AgentInfoConfig& config);

//  void insufficientFundsBuying(unsigned int amount, double unitPrice, double currentFunds);
//  void insufficientEnergySelling(unsigned int amount, double unitPrice, double currentEnergyStored);
//  void appendBuyEnergy(unsigned int amount, double unitPrice, double currentFundsTotal, double currentEnergyTotal);
//  void appendSellEnergy(unsigned int amount, double unitPrice, double currentFundsTotal, double currentEnergyTotal);
  void logBuyEnergyOrder(Order order);
  void logSellEnergyOrder(Order order);
  void logTransferBoughtEnergy(Order order, double currAmountEnergy, double currAmountMoney, double currSystemPrice);
  void logTransferSoldEnergy(Order order, double currAmountEnergy, double currAmountMoney, double currSystemPrice);

private:
  AgentInfoConfig config_;
  bool sendEmail_;
  QString email_;
  std::unique_ptr<SmtpClient> smtp_;



  void sendMail(QString email, QString subject, QString message);
};

#endif // TRANSACTIONLOGGER_H
