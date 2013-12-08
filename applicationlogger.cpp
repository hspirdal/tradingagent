#include "applicationlogger.h"

ApplicationLogger::ApplicationLogger(QString logfile, const QString& clientEmailAddr, const QString& clientSenderName,
const QString& clientGmailPassw, QList<QString> recipientsEmailAddr)
: Logger(logfile), clientSenderName_(clientSenderName)
{
  smtp_ = std::unique_ptr<SmtpClient>(new SmtpClient(SmtpAddr_, SmtpPort_, SmtpClient::SslConnection));
  smtp_->setUser(clientEmailAddr);
  smtp_->setPassword(clientGmailPassw);

  clientEmailAddr_ = std::make_shared<EmailAddress>(clientEmailAddr, clientSenderName);
  for(QString addr : recipientsEmailAddr)
    recipientsEmailAddr_.append(std::make_shared<EmailAddress>(addr, ""));
}

ApplicationLogger::~ApplicationLogger() { }

void ApplicationLogger::append(QString message, bool writeDebug)
{
  Logger::append(message, logfile_, writeDebug);
  emit valueChanged(message);
}

void ApplicationLogger::appendMail(QString title, QString message, bool writeDebug)
{
  appendMail(title, message, logfile_, writeDebug);
}

void ApplicationLogger::appendMail(QString title, QString message, QString logfile, bool writeDebug)
{
  Logger::append(message, logfile, writeDebug);
  emit valueChanged(message);
  this->sendMail(title, message);
}

void ApplicationLogger::sendMail(QString subject, QString message)
{
  MimeMessage mimeMessage;
  mimeMessage.setSender(clientEmailAddr_.get());
  for(std::shared_ptr<EmailAddress> addr : recipientsEmailAddr_)
    mimeMessage.addRecipient(addr.get());
  mimeMessage.setSubject(subject);
  MimeText text;
  text.setText(message);
  mimeMessage.addPart(&text);
  smtp_->connectToHost();
  smtp_->login();
  smtp_->sendMail(mimeMessage);
  smtp_->quit();
}
