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
//  TrainRow(std::vector<double> spotPrices, double avg)
//  {
//    spotPrices_ = spotPrices;
//    avgPrice_ = avg;
//  }

  TrainRow(std::deque<double> priceWindow, double solutionAverage)
  {
    spotPrices_ = priceWindow;
    avgPrice_ = solutionAverage;
  }


  QString toString() const
  {
    QString out("");
    for(double price : spotPrices_)
      out.append(QString::number(price) + " ");

    out.append(QString("\n\n%1\n").arg(QString::number(avgPrice_)));
    return out;
  }

private:
  std::deque<double> spotPrices_;
  double avgPrice_;
};

class DataTrainSet
{
public:
  DataTrainSet(QString setname, unsigned int numInputValues, unsigned int numOutputValues)
  :  setName_(setname), numInputPerRow_(numInputValues), numOutputPerRow_(numOutputValues)
  {}

  // It is assumed (and up to client caller) that the vector has values between [0.0, 1.0].
//  void appendRow(const std::vector<double>& dayPrices)
//  {
//    // Expect the input value to match the criterea set on the set.
//    assert(dayPrices.size() == numInputPerRow_);
//    // Output is here the average of the input values.
//    const double avg = std::accumulate(dayPrices.begin(), dayPrices.end(), 0.0) / dayPrices.size();
//    trainingRows_.append(TrainRow(dayPrices, avg));
//  }

  void appendRow(const std::deque<double>& priceWindow, const std::deque<double>& aheadPriceWindow)
  {
    assert((priceWindow.size() == aheadPriceWindow.size()) && priceWindow.size() == numInputPerRow_ && "unexpected size difference");
    const double solutionAverage = std::accumulate(aheadPriceWindow.begin(), aheadPriceWindow.end(), 0.0) / aheadPriceWindow.size();
    trainingRows_.append(TrainRow(priceWindow, solutionAverage));
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
