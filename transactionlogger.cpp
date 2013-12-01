#include "transactionlogger.h"

TransactionLogger::TransactionLogger(QString logfile, QString email)
  : Logger(logfile), email_(email), sendEmail_(false), smtp_(new SmtpClient("smtp.gmail.com", 465, SmtpClient::SslConnection))
{

  smtp_->setUser("john.baugen@gmail.com");
  smtp_->setPassword("jobaJOBA");
}


void TransactionLogger::insufficientFundsBuying(unsigned int amount, double unitPrice, double currentFunds)
{
  this->append(QString("Insuffcient funds to complete transaction of %1 energy at price %2. Current funds: %3.").arg(QString::number(amount),
  QString::number(unitPrice), QString::number(currentFunds)));
}

void TransactionLogger::insufficientEnergySelling(unsigned int amount, double unitPrice, double currentEnergyStored)
{
  this->append(QString("Insuffcient amount of energy to complete transaction of %1 energy at price %2. Current amount stored: %3.").arg(QString::number(amount),
  QString::number(unitPrice), QString::number(currentEnergyStored)));
}

void TransactionLogger::appendBuyEnergy(unsigned int amount, double unitPrice, double currentFundsTotal, double currentEnergyTotal)
{
  QString msg = QString("Bought %1 amount of energy at %2 NOK. Current funds: %3. Current amount of energy: %4").arg(QString::number(amount),
   QString::number(unitPrice), QString::number(currentFundsTotal), QString::number(currentEnergyTotal));
  this->append(msg);


  sendMail("nunyah@gmail.com", "Bought energy", msg);
}

void TransactionLogger::appendSellEnergy(unsigned int amount, double unitPrice, double currentFundsTotal, double currentEnergyTotal)
{
  this->append(QString("Sold %1 amount of energy at %2 NOK. Current funds: %3. Current amount of energy: %4").arg(QString::number(amount),
   QString::number(unitPrice), QString::number(currentFundsTotal), QString::number(currentEnergyTotal)));
}

void TransactionLogger::sendMail(QString email, QString subject, QString message)
{
  if(!sendEmail_)
    return;

  MimeMessage mimeMessage;
  mimeMessage.setSender(new EmailAddress("john.baugen@gmail.com", "John Baugen"));
  mimeMessage.addRecipient(new EmailAddress(email, ""));
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
