#ifndef TRANSACTIONLOGGER_H
#define TRANSACTIONLOGGER_H

#include "applicationlogger.h"
#include <memory>
#include "3rdparty/SmtpClient-for-Qt/src/SmtpMime"
#include "Config.h"
#include "Order.h"

class TransactionLogger : public ApplicationLogger
{
public:
  TransactionLogger(QString logfile, const QString& clientEmailAddr, const QString& clientSenderName, const QString& clientGmailPassw, QList<QString> recipients, std::shared_ptr<Config> config);

  void logBuyEnergyOrder(Order order);
  void logSellEnergyOrder(Order order);
  void logTransferBoughtEnergy(Order order, double currAmountEnergy, double currAmountMoney, double currSystemPrice);
  void logTransferSoldEnergy(Order order, double currAmountEnergy, double currAmountMoney, double currSystemPrice);
  void logAvoidedSellingEnergy(double predictedPrice, double currAmountEnergy, double currAmountMoney);
  void logAvoidedBuyingEnergy(double predictedPrice, double currAmountEnergy, double currAmountMoney);

private:
  std::shared_ptr<Config> config_;
};

#endif // TRANSACTIONLOGGER_H
