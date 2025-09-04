#ifndef TRACKER_ASSOCIATION_FUSION_TRACKER_ASSOCIATOR_H_
#define TRACKER_ASSOCIATION_FUSION_TRACKER_ASSOCIATOR_H_

#include "association/associator_base.h"
#include "localization_service.h"
#include "track.h"

namespace tracker {

class FusionTrackerAssociator : public AssociatorBase {
 public:
  explicit FusionTrackerAssociator(
      const LocalizationService* localization_service);
  ~FusionTrackerAssociator() = default;

 private:
  double ComputeSimilarity(const Track& track,
                           const ObservationProto& observation) const override;

  double CameraSimilarity(
      const Track& track,
      const CameraObservationProto& camera_observation) const;
};

}  // namespace tracker

#endif  // TRACKER_ASSOCIATION_FUSION_TRACKER_ASSOCIATOR_H_
