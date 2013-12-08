#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <memory>

class Logger
{
public:
  Logger(QString logfile);

  virtual void append(QString message, bool writeDebug = false);
  virtual void append(QString message, QString logfile, bool writeDebug = false);

  // Making the logger be able to work as a singleton as well because it's convenient.
  static Logger& get() { static Logger instance_; return instance_; }

protected:
  QString logfile_;
  Logger() : logfile_("global.log") {}

};

#endif // LOGGER_H
