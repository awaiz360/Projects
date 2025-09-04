#ifndef TRACKER_ASSOCIATION_CAMERA_TRACKER_ASSOCIATOR_H_
#define TRACKER_ASSOCIATION_CAMERA_TRACKER_ASSOCIATOR_H_

#include "association/associator_base.h"

namespace tracker {

class CameraTrackerAssociator : public AssociatorBase {
 public:
  explicit CameraTrackerAssociator(
      const LocalizationService* localization_service);
  ~CameraTrackerAssociator() = default;

 protected:
  double ComputeSimilarity(const Track& track,
                           const ObservationProto& observation) const override;
  double CameraSimilarity(
      const Track& track,
      const CameraObservationProto& camera_observation) const;
};

}  // namespace tracker

#endif  // TRACKER_ASSOCIATION_CAMERA_TRACKER_ASSOCIATOR_H_
