#ifndef TRANSACTIONLOGGER_H
#define TRANSACTIONLOGGER_H

#include "logger.h"
#include <memory>
#include "3rdparty/SmtpClient-for-Qt/src/SmtpMime"

class TransactionLogger : public Logger
{
public:
  TransactionLogger(QString logfile, QString email);

  void insufficientFundsBuying(unsigned int amount, double unitPrice, double currentFunds);
  void insufficientEnergySelling(unsigned int amount, double unitPrice, double currentEnergyStored);
  void appendBuyEnergy(unsigned int amount, double unitPrice, double currentFundsTotal, double currentEnergyTotal);
  void appendSellEnergy(unsigned int amount, double unitPrice, double currentFundsTotal, double currentEnergyTotal);

private:
  QString email_;
  std::unique_ptr<SmtpClient> smtp_;
  bool sendEmail_;

  void sendMail(QString email, QString subject, QString message);
};

#endif // TRANSACTIONLOGGER_H
