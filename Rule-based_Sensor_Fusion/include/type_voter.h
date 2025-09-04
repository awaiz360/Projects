#ifndef TRACKER_TYPE_VOTER_H_
#define TRACKER_TYPE_VOTER_H_

#include <iterator>

#include "const_def.h"
#include "logger.h"
#include "types.h"

namespace tracker {

class TypeVoter {
 public:
  ObservationType Vote(const Track& track, const ObservationType current_type,
                       const double current_confidence) {
    std::vector<double> types(kNumberOfItemsInObservationType, 0.0);
    types[static_cast<int>(current_type)] =
        current_confidence;  // considering the new observation
    for (auto it = track.checkpoints.rbegin(); it != track.checkpoints.rend();
         ++it) {
      const auto& ckpt = *it;  // Dereference the iterator to get the checkpoint
      const double dt = track.track_state.timestamp - ckpt.timestamp;
      if (dt < 0) {
        Logger::GetInstance().Log(
            Logger::WARNING,
            "Warning: The checkpoint time stamp inversion is found.");
        std::cout << "The checkpoint time stamp inversion is found."
                  << std::endl;
        continue;
      }
      if (dt > kDuration_) {
        break;
      }

      if (ckpt.observation) {  // because the track states have been updated, we
                               // need to use the track observation
        if (ckpt.observation->has_lidar_observation()) {
          types[static_cast<int>(
              ckpt.observation->lidar_observation().type())] +=
              ckpt.observation->lidar_observation().conf() *
              exp(-5 * dt / kDuration_);
        } else if (ckpt.observation->has_camera_observation()) {
          types[static_cast<int>(
              ckpt.observation->camera_observation().type())] +=
              ckpt.observation->camera_observation().conf() *
              exp(-5 * dt / kDuration_);
        } else if (ckpt.observation->has_radar_observation()) {
          types[static_cast<int>(
              ckpt.observation->radar_observation().type())] +=
              1 * exp(-5 * dt /
                      kDuration_);  // ToDo: replace 1 with radar confidence
        } else {
          Logger::GetInstance().Log(Logger::ERROR,
                                    "Error: Unknown observation type.");
          std::cout << "Unknown observation type." << std::endl;
        }
      }
      return static_cast<ObservationType>(std::distance(
          types.begin(), std::max_element(types.begin(), types.end())));
    }
  }

 private:
  double kDuration_ = 0.5;
};

}  // namespace tracker

#endif  // TRACKER_TYPE_VOTER_H_
