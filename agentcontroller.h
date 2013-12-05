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
    bool currentDay() const { return config_->agentInfoConfig().CurrentDay_; }
    bool hasMadeOrder() const
    {
      bool f = config_->agentInfoConfig().HasMadeOrder_;
      return config_->agentInfoConfig().HasMadeOrder_; }
    bool hasCompletedTransaction() const { return config_->agentInfoConfig().HasCompletedTransaction_; }
    bool agentSleepingUntilNextDay() const
    {
      return config_->agentInfoConfig().IsAgentSleeping_;
    }

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
    std::shared_ptr<Config> config_;
    std::shared_ptr<Logger> logger_;
    std::shared_ptr<NeuralNet> neurnet_;
    std::shared_ptr<AssetsManager> assets_;
    std::vector<double>latestDailyPrices_;
    QDateTime currentTime_;
    //bool agentSleepingUntilNextDay_;

    double predictDayAheadPrice();

    /* logic */
    bool isFreshSystemPrice_;
    bool isFreshPriceData_;
//    bool hasPredictedAndMadeOrder_;
//    bool hasUpdatedAssetReserves_;
    QDateTime dateStarted_;


    void resetFlags();
    void setHasMadeOrder(bool hasMadeOrder) { config_->setValue("agentinfo", "hasMadeOrder", static_cast<unsigned int>(hasMadeOrder ? 1:0)); }
    void setHasCompletedTransaction(bool hasCompletedTransaction) { config_->setValue("agentinfo", "hasCompletedTransaction", static_cast<unsigned int>(hasCompletedTransaction ? 1:0)); }
    void setSleeping(bool sleeping) { config_->setValue("agentinfo", "isAgentSleeping", static_cast<unsigned int>(sleeping ? 1:0)); }
    void setCurrentDay(unsigned int day) { config_->setValue("agentinfo", "currentDay", day); }


};

#endif // AGENTCONTROLLER_H
