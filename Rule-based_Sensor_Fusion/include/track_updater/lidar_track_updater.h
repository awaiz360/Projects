#ifndef TRACKER_TRACK_UPDATER_LIDAR_TRACK_UPDATER_H_
#define TRACKER_TRACK_UPDATER_LIDAR_TRACK_UPDATER_H_

#include "track_updater/updater_base.h"

namespace tracker {

class LidarTrackUpdater : public UpdaterBase {
 public:
  LidarTrackUpdater(const TrackLifeManager* track_life_manager);
  ~LidarTrackUpdater() = default;

  void UpdateTrackFromObservation(const ObservationProto& observation,
                                  Track* track) override;

 private:
  ObservationNoiseComputator measurement_noise_ =
      ObservationNoiseComputator(TrackerEngineType::TET_LIDAR);
};

}  // namespace tracker

#endif  // TRACKER_TRACK_UPDATER_LIDAR_TRACK_UPDATER_H_
