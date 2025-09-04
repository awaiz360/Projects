#include "track_updater/updater_base.h"

namespace tracker {

UpdaterBase::UpdaterBase(const TrackLifeManager* track_life_manager)
    : track_life_manager_(track_life_manager) {}

double UpdaterBase::ConfidenceCalculator(double obs_timestamp,
                                         double track_timestamp,
                                         double obs_conf,
                                         double track_conf) const {
  double dt = obs_timestamp - track_timestamp;
  if (dt < 0) dt = 0;
  constexpr double maximumLifeTime = 1.0;  // sec
  double new_conf = 0.0;
  // update posterior probability of existence
  new_conf = track_conf - dt / maximumLifeTime;
  if (new_conf < 0.01) new_conf = 0.01;
  // update total probability of the measurement
  double PB = new_conf * obs_conf + ((1 - new_conf) * (1 - obs_conf));
  // P_B|A = P_A|B * P_A / P_B
  new_conf = new_conf * obs_conf / PB;
  return new_conf;
}

}  // namespace tracker
