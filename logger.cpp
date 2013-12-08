#include "logger.h"

#include <fstream>
#include <QDebug>
#include <QDateTime>

Logger::Logger(QString logfile)
  : logfile_(logfile)
{
}

void Logger::append(QString message, bool writeDebug)
{
  append(message, logfile_, writeDebug);
}

void Logger::append(QString message, QString logfile, bool writeDebug)
{
  std::ifstream in(logfile_.toStdString());
  std::string prevContents = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  in.close();
  std::ofstream out(logfile_.toStdString());
  const QString record = QDateTime::currentDateTime().toString() + ": " + message;
  out << prevContents << message.toStdString() << std::endl;
  out.close();

  emit recordAdded(record);

  if(writeDebug)
    qDebug() << "Log:" << logfile << "-" << message;
}
