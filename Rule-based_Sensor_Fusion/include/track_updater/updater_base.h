#ifndef TRACKER_TRACK_UPDATER_UPDATER_BASE_H_
#define TRACKER_TRACK_UPDATER_UPDATER_BASE_H_

#include <math.h>

#include "measurement_noise_calculator.h";
#include "sensor_voter.h"
#include "track.h"
#include "track_life_manager.h"
#include "type_voter.h"
#include "types.h"

namespace tracker {

class UpdaterBase {
 public:
  UpdaterBase(const TrackLifeManager* track_life_manager);
  virtual ~UpdaterBase() = default;

  virtual void UpdateTrackFromObservation(const ObservationProto& observation,
                                          Track* track) = 0;

 protected:
  virtual double ConfidenceCalculator(double obs_timestamp,
                                      double track_timestamp, double obs_conf,
                                      double track_conf) const;

 protected:
  TypeVoter type_voter_;
  SensorVoter sensor_voter_;
  const TrackLifeManager* track_life_manager_ = nullptr;
};

}  // namespace tracker

#endif  // TRACKER_TRACK_UPDATER_UPDATER_BASE_H_
