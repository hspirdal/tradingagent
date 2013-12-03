#ifndef TRAININGSET_H
#define TRAININGSET_H

#include <QList>
#include <assert.h>
#include <QString>
#include "util.h"
#include <deque>

struct TrainRow
{
public:
  TrainRow(std::deque<double> inputs, double output)
  {
    inputs_ = inputs;
    output_ = output;
  }


  QString toString() const
  {
    QString out("");
    for(double input : inputs_)
      out.append(QString::number(input) + " ");

    out.append(QString("\n\n%1\n").arg(QString::number(output_)));
    return out;
  }

private:
  std::deque<double> inputs_;
  double output_;
};

class DataTrainSet
{
public:
  DataTrainSet(QString setname, unsigned int numInputValues, unsigned int numOutputValues)
  :  setName_(setname), numInputPerRow_(numInputValues), numOutputPerRow_(numOutputValues)
  {}

  void appendRow(const std::deque<double>& inputs, const double output)
  {
    assert(inputs.size() == numInputPerRow_ && "unexpected size difference");
    trainingRows_.append(TrainRow(inputs, output));
  }

  QString toString() const
  {
    QString out = "";
    out.append(QString("%1 %2 %3\n").arg(QString::number(numRows()), QString::number(numInputValuesPerRow()), QString::number(numOutoutValuesPerRow())));
    for(TrainRow row : trainingRows_)
      out.append("\n" + row.toString());

    return out;
  }

  void saveToFile(const QString& filename)
  {
    Util::writeFile(filename.toStdString(), toString().toStdString(), true);
  }


  const QString& name() const { return setName_; }
  unsigned int numRows() const { return trainingRows_.count(); }
  unsigned int numInputValuesPerRow() const { return numInputPerRow_; }
  unsigned int numOutoutValuesPerRow() const { return numOutputPerRow_; }

private:
  const QString setName_;
  QList<TrainRow> trainingRows_;
  const unsigned int numInputPerRow_;
  const unsigned int numOutputPerRow_;

};

#endif // TRAININGSET_H
