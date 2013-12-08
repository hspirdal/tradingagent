#ifndef APPLICATIONLOGGER_H
#define APPLICATIONLOGGER_H

#include <QObject>
#include <QString>
#include "logger.h"
#include <memory>
#include "3rdparty/SmtpClient-for-Qt/src/SmtpMime"

class ApplicationLogger : public QObject, public Logger
{
  Q_OBJECT
public:
  ApplicationLogger(QString logfile, const QString& clientEmailAddr, const QString& clientSenderName, const QString& clientGmailPassw, QList<QString> recipients);
  virtual ~ApplicationLogger();

  virtual void append(QString message, bool writeDebug);
  void appendMail(QString title, QString message, QString logfile, bool writeDebug = false);
  void appendMail(QString title, QString message, bool writeDebug = false);

signals:
  void valueChanged(QString message);

protected:
  QString clientSenderName_;
  std::unique_ptr<SmtpClient> smtp_;
  const int SmtpPort_ = 465;
  const QString SmtpAddr_ = "smtp.gmail.com";
  std::shared_ptr<EmailAddress> clientEmailAddr_;
  QList<std::shared_ptr<EmailAddress> > recipientsEmailAddr_;

  void sendMail(QString subject, QString message);

};

#endif // APPLICATIONLOGGER_H
