#ifndef NEURALCONFIG_H
#define NEURALCONFIG_H

#include <QString>

struct NeuralConfig
{
public:
  QString Nameset_;
  double MaxPriceExpected_;
  unsigned int DayAheadShort_;
  unsigned int DayAheadLong_;
  unsigned int NumLayers_;
  unsigned int NumNeuronsHidden_;
  unsigned int NumOutputs_;
  double DesiredError_;
  unsigned int MaxEpochs_;
  unsigned int EpochsBetweenReports_;
};

#endif // NEURALCONFIG_H
