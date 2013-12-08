#include "transactionlogger.h"

TransactionLogger::TransactionLogger(QString logfile, const AgentInfoConfig& config)
  : Logger(logfile), config_(config), sendEmail_(config.SendEmail_), smtp_(new SmtpClient(config.SmtpClient_, config.SmtpPort_, SmtpClient::SslConnection))
{

  auto s = config.ClientEmailAddr_;
  auto p = config.SmtpPassw_;
  smtp_->setUser(config.ClientEmailAddr_);
  smtp_->setPassword(config.SmtpPassw_);
}


//void TransactionLogger::insufficientFundsBuying(unsigned int amount, double unitPrice, double currentFunds)
//{
//  this->append(QString("Insuffcient funds to complete transaction of %1 energy at price %2. Current funds: %3.").arg(QString::number(amount),
//  QString::number(unitPrice), QString::number(currentFunds)));
//}

//void TransactionLogger::insufficientEnergySelling(unsigned int amount, double unitPrice, double currentEnergyStored)
//{
//  this->append(QString("Insuffcient amount of energy to complete transaction of %1 energy at price %2. Current amount stored: %3.").arg(QString::number(amount),
//  QString::number(unitPrice), QString::number(currentEnergyStored)));
//}

//void TransactionLogger::appendBuyEnergy(unsigned int amount, double unitPrice, double currentFundsTotal, double currentEnergyTotal)
//{
//  QString msg = QString("Bought %1 amount of energy at %2 NOK. Current funds: %3. Current amount of energy: %4").arg(QString::number(amount),
//   QString::number(unitPrice), QString::number(currentFundsTotal), QString::number(currentEnergyTotal));
//  this->append(msg);
//}


//void TransactionLogger::appendSellEnergy(unsigned int amount, double unitPrice, double currentFundsTotal, double currentEnergyTotal)
//{
//  this->append(QString("Sold %1 amount of energy at %2 NOK. Current funds: %3. Current amount of energy: %4").arg(QString::number(amount),
//   QString::number(unitPrice), QString::number(currentFundsTotal), QString::number(currentEnergyTotal)));
//}

void TransactionLogger::logBuyEnergyOrder(Order order)
{
  QString msg = QString("AgentID: %1, Halvor Spirdal. Time of record: %2. Target prediction date: %3. Estimated Price: %4. OrderID: %5. Purchase of %6 Mwh").arg(
    QString::number(config_.AgentId_), order.dateTime_.toString(), order.targetPredictionDate().toString(), QString::number(order.predictedUnitPrice_),
    QString::number(order.orderNumber_), QString::number(order.boughtAmountEnergy_));

  this->append(msg, true);
  //sendMail(config_.receiverEmail_, "Order: Buy Energy - Halvor Spirdal", msg);
  sendMail(config_.receiverEmail_, "Order: Buy Energy - Halvor Spirdal", msg, config_.receiverEmail2_);

}

void TransactionLogger::logSellEnergyOrder(Order order)
{
  QString msg = QString("AgentID: %1, Halvor Spirdal. Time of record: %2. Target prediction date: %3. Estimated Price: %4. OrderID: %5. Selling of %6 Mwh").arg(
    QString::number(config_.AgentId_), order.dateTime_.toString(), order.targetPredictionDate().toString(), QString::number(order.predictedUnitPrice_),
    QString::number(order.orderNumber_), QString::number(order.soldAmountEnergy_));

  this->append(msg, true);
  //sendMail(config_.receiverEmail_, "Order: Sell Energy - Halvor Spirdal", msg);
  sendMail(config_.receiverEmail_, "Order: Sell Energy - Halvor Spirdal", msg, config_.receiverEmail2_);
}

void TransactionLogger::logTransferBoughtEnergy(Order order, double currAmountEnergy, double currAmountMoney, double currSystemPrice)
{
  QString msg = QString("AgentID: %1, Halvor Spirdal. Time of record: %2. Transferred over %3 Mwh energy (UnitPrice: %4). OrderID: %5. Total energy accumulated: %6 Mwh. Total amount of money: %7").arg(
    QString::number(config_.AgentId_), QDateTime::currentDateTime().toString(), QString::number(order.boughtAmountEnergy_),
    QString::number(currSystemPrice), QString::number(order.orderNumber_), QString::number(currAmountEnergy), QString::number(currAmountMoney));

  this->append(msg, true);
  //sendMail(config_.receiverEmail_, "Transferred over energy - Halvor Spirdal", msg);
  sendMail(config_.receiverEmail_, "Transferred over energy - Halvor Spirdal", msg, config_.receiverEmail2_);
}

void TransactionLogger::logTransferSoldEnergy(Order order, double currAmountEnergy, double currAmountMoney, double currSystemPrice)
{
  QString msg = QString("AgentID: %1, Halvor Spirdal. Time of record: %2. Transferred off %3 Mwh energy (UnitPrice: %4). OrderID: %5. Total energy accumulated: %6 Mwh. Total amount of money: %7").arg(
    QString::number(config_.AgentId_), QDateTime::currentDateTime().toString(), QString::number(order.soldAmountEnergy_),
    QString::number(currSystemPrice), QString::number(order.orderNumber_), QString::number(currAmountEnergy), QString::number(currAmountMoney));

  this->append(msg, true);
  //sendMail(config_.receiverEmail_, "Transferred away energy - Halvor Spirdal", msg);
  sendMail(config_.receiverEmail_, "Transferred away energy - Halvor Spirdal", msg, config_.receiverEmail2_);
}

void TransactionLogger::logAvoidedSellingEnergy(double predictedPrice, double currAmountEnergy, double currAmountMoney)
{
  QString msg = QString("AgentID: %1, Halvor Spirdal. Time of record: %2. Target prediction date: %3. Estimated Price: %4. Avoided selling energy because of rule MoneyHighEnergyLow. Current amount of energy: %5. Current funds: %6").arg(
  QString::number(config_.AgentId_), QDateTime::currentDateTime().toString(), QDateTime::currentDateTime().addDays(1).toString(), QString::number(predictedPrice), QString::number(currAmountEnergy), QString::number(currAmountMoney));

  this->append(msg, true);
  sendMail(config_.receiverEmail_, "Avoided selling energy - Halvor Spirdal", msg, config_.receiverEmail2_);
}

void TransactionLogger::logAvoidedBuyingEnergy(double predictedPrice, double currAmountEnergy, double currAmountMoney)
{
  QString msg = QString("AgentID: %1, Halvor Spirdal. Time of record: %2. Target prediction date: %3. Estimated Price: %4. Avoided buying energy because of rule MoneyLowEnergyHigh. Current amount of energy: %5. Current funds: %6").arg(
  QString::number(config_.AgentId_), QDateTime::currentDateTime().toString(), QDateTime::currentDateTime().addDays(1).toString(), QString::number(predictedPrice), QString::number(currAmountEnergy), QString::number(currAmountMoney));

  this->append(msg, true);
  sendMail(config_.receiverEmail_, "Avoided buying energy - Halvor Spirdal", msg, config_.receiverEmail2_);
}


void TransactionLogger::sendMail(QString email, QString subject, QString message, QString secondEmail)
{
  if(!sendEmail_)
    return;

  MimeMessage mimeMessage;
  mimeMessage.setSender(new EmailAddress(config_.ClientEmailAddr_, config_.ClientName_));
  mimeMessage.addRecipient(new EmailAddress(email, ""));
  if(!secondEmail.isEmpty())
    mimeMessage.addRecipient(new EmailAddress(secondEmail, ""));
  mimeMessage.setSubject(subject);
  MimeText text;
  text.setText(message);
  mimeMessage.addPart(&text);
  smtp_->connectToHost();
  smtp_->login();
  smtp_->sendMail(mimeMessage);
  smtp_->quit();

  qDebug() << "sent email.";
}
