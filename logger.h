#ifndef LOGGER_H
#define LOGGER_H

#include <QString>

class Logger
{
public:
  Logger(QString logfile);
  virtual void append(QString message);

protected:
  QString logfile_;
};

#endif // LOGGER_H
