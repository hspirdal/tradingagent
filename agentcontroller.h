#ifndef AGENTCONTROLLER_H
#define AGENTCONTROLLER_H

#include <QMap>
#include <memory>
#include "logger.h"
#include "Config.h"
#include "neuralnet.h"
#include "assetsmanager.h"

class AgentController
{
public:
  AgentController(std::shared_ptr<Config> config, std::shared_ptr<NeuralNet> neurnet, std::shared_ptr<AssetsManager> assets);


  void predictPriceAhead();
  void createAndTrainSet(const QString& trainSetName, const QMap<QDateTime, double>& spotprices);
  void setLatestDailyPrices(const QMap<QDateTime, double>& latestDaily);

    QDateTime currentDate() const { return currentTime_; }
    bool isFreshSystemPrice() const { return isFreshSystemPrice_; }
    bool isFreshPriceData() const { return isFreshPriceData_; }
    bool hasPredictedAndMadeOrder() const { return hasPredictedAndMadeOrder_; }
    bool hasUpdatedAssetReserves() const { return hasUpdatedAssetReserves_; }
    bool agentSleepingUntilNextDay() const { return agentSleepingUntilNextDay_; }

    bool tryToWake();
    void completeRemainingTransactions();
    void setSystemPrice(double systemPrice)
    {
      assets_->setRealSystemPrice(systemPrice);
      // Don't want to set this flag until it is fairly close to  the time the flag is ment to work for.
      if(currentDate().time().hour() >= config_->miscConfig().Time_Predict_Price_.hour()-2)
        isFreshSystemPrice_ = true;
    }

  private:
    AgentInfoConfig agentInfoConfig_;
    std::shared_ptr<Config> config_;
    std::shared_ptr<Logger> logger_;
    std::shared_ptr<NeuralNet> neurnet_;
    std::shared_ptr<AssetsManager> assets_;
    std::vector<double>latestDailyPrices_;
    QDateTime currentTime_;
    bool agentSleepingUntilNextDay_;

    double predictDayAheadPrice();

    /* logic */
    bool isFreshSystemPrice_;
    bool isFreshPriceData_;
    bool hasPredictedAndMadeOrder_;
    bool hasUpdatedAssetReserves_;
    QDateTime dateStarted_;


    void resetFlags();


};

#endif // AGENTCONTROLLER_H
