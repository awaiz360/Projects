#ifndef TRACKER_TRACK_UPDATER_FUSION_TRACK_UPDATER_H_
#define TRACKER_TRACK_UPDATER_FUSION_TRACK_UPDATER_H_

#include "track_updater/updater_base.h"

namespace tracker {

class FusionTrackUpdater : public UpdaterBase {
 public:
  explicit FusionTrackUpdater(const TrackLifeManager* track_life_manager);
  ~FusionTrackUpdater() = default;

  void UpdateTrackFromObservation(const ObservationProto& observation,
                                  Track* track) override;

 private:
  ObservationNoiseComputator measurement_noise_ =
      ObservationNoiseComputator(TrackerEngineType::TET_FUSION);
};

}  // namespace tracker

#endif  // TRACKER_TRACK_UPDATER_FUSION_TRACK_UPDATER_H_
