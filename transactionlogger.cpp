#include "transactionlogger.h"

TransactionLogger::TransactionLogger(QString logfile, const QString& clientEmailAddr, const QString& clientSenderName, const QString& clientGmailPassw, QList<QString> recipients, std::shared_ptr<Config> config)
  : ApplicationLogger(logfile, clientEmailAddr, clientSenderName, clientGmailPassw, recipients), config_(config)
{
}

void TransactionLogger::logBuyEnergyOrder(Order order)
{
  QString msg = QString("AgentID: %1, Halvor Spirdal. Time of record: %2. Target prediction date: %3. Estimated Price: %4. OrderID: %5. Purchase of %6 Mwh").arg(
    QString::number(config_->agentInfoConfig().AgentId_), order.dateTime_.toString(), order.targetPredictionDate().toString(), QString::number(order.predictedUnitPrice_),
    QString::number(order.orderNumber_), QString::number(order.boughtAmountEnergy_));

  this->append(msg, true);
  sendMail("Order: Buy Energy - Halvor Spirdal", msg);

}

void TransactionLogger::logSellEnergyOrder(Order order)
{
  QString msg = QString("AgentID: %1, Halvor Spirdal. Time of record: %2. Target prediction date: %3. Estimated Price: %4. OrderID: %5. Selling of %6 Mwh").arg(
    QString::number(config_->agentInfoConfig().AgentId_), order.dateTime_.toString(), order.targetPredictionDate().toString(), QString::number(order.predictedUnitPrice_),
    QString::number(order.orderNumber_), QString::number(order.soldAmountEnergy_));

  this->append(msg, true);
  sendMail("Order: Sell Energy - Halvor Spirdal", msg);
}

void TransactionLogger::logTransferBoughtEnergy(Order order, double currAmountEnergy, double currAmountMoney, double currSystemPrice)
{
  QString msg = QString("AgentID: %1, Halvor Spirdal. Time of record: %2. Transferred over %3 Mwh energy (UnitPrice: %4). OrderID: %5. Total energy accumulated: %6 Mwh. Total amount of money: %7").arg(
    QString::number(config_->agentInfoConfig().AgentId_), QDateTime::currentDateTime().toString(), QString::number(order.boughtAmountEnergy_),
    QString::number(currSystemPrice), QString::number(order.orderNumber_), QString::number(currAmountEnergy), QString::number(currAmountMoney));

  this->append(msg, true);
  sendMail("Transferred over energy - Halvor Spirdal", msg);
}

void TransactionLogger::logTransferSoldEnergy(Order order, double currAmountEnergy, double currAmountMoney, double currSystemPrice)
{
  QString msg = QString("AgentID: %1, Halvor Spirdal. Time of record: %2. Transferred off %3 Mwh energy (UnitPrice: %4). OrderID: %5. Total energy accumulated: %6 Mwh. Total amount of money: %7").arg(
    QString::number(config_->agentInfoConfig().AgentId_), QDateTime::currentDateTime().toString(), QString::number(order.soldAmountEnergy_),
    QString::number(currSystemPrice), QString::number(order.orderNumber_), QString::number(currAmountEnergy), QString::number(currAmountMoney));

  this->append(msg, true);
  sendMail("Transferred away energy - Halvor Spirdal", msg);
}

void TransactionLogger::logAvoidedSellingEnergy(double predictedPrice, double currAmountEnergy, double currAmountMoney)
{
  QString msg = QString("AgentID: %1, Halvor Spirdal. Time of record: %2. Target prediction date: %3. Estimated Price: %4. Avoided selling energy because of rule MoneyHighEnergyLow. Current amount of energy: %5. Current funds: %6").arg(
  QString::number(config_->agentInfoConfig().AgentId_), QDateTime::currentDateTime().toString(), QDateTime::currentDateTime().addDays(1).toString(), QString::number(predictedPrice), QString::number(currAmountEnergy), QString::number(currAmountMoney));

  this->append(msg, true);
  sendMail("Avoided selling energy - Halvor Spirdal", msg);
}

void TransactionLogger::logAvoidedBuyingEnergy(double predictedPrice, double currAmountEnergy, double currAmountMoney)
{
  QString msg = QString("AgentID: %1, Halvor Spirdal. Time of record: %2. Target prediction date: %3. Estimated Price: %4. Avoided buying energy because of rule MoneyLowEnergyHigh. Current amount of energy: %5. Current funds: %6").arg(
  QString::number(config_->agentInfoConfig().AgentId_), QDateTime::currentDateTime().toString(), QDateTime::currentDateTime().addDays(1).toString(), QString::number(predictedPrice), QString::number(currAmountEnergy), QString::number(currAmountMoney));

  this->append(msg, true);
  sendMail("Avoided buying energy - Halvor Spirdal", msg);
}
