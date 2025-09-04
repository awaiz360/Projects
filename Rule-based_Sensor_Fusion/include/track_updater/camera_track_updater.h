#ifndef TRACKER_TRACK_UPDATER_CAMERA_TRACK_UPDATER_H_
#define TRACKER_TRACK_UPDATER_CAMERA_TRACK_UPDATER_H_

#include "track_updater/updater_base.h"

namespace tracker {

class CameraTrackUpdater : public UpdaterBase {
 public:
  explicit CameraTrackUpdater(const TrackLifeManager* track_life_manager);
  ~CameraTrackUpdater() = default;

  void UpdateTrackFromObservation(const ObservationProto& observation,
                                  Track* track) override;

 private:
  ObservationNoiseComputator measurement_noise_ =
      ObservationNoiseComputator(TrackerEngineType::TET_CAMERA);
};

}  // namespace tracker

#endif  // TRACKER_TRACK_UPDATER_CAMERA_TRACK_UPDATER_H_
