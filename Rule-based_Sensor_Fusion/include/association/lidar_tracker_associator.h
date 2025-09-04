#ifndef TRACKER_ASSOCIATION_LIDAR_TRACKER_ASSOCIATOR_H_
#define TRACKER_ASSOCIATION_LIDAR_TRACKER_ASSOCIATOR_H_

#include "association/associator_base.h"

namespace tracker {

class LidarTrackerAssociator : public AssociatorBase {
 public:
  explicit LidarTrackerAssociator(
      const LocalizationService* localization_service);
  ~LidarTrackerAssociator() = default;

 protected:
  double ComputeSimilarity(const Track& track,
                           const ObservationProto& observation) const override;
  // TODO: Maybe remove this function in lidar tracker associator.
  double CameraSimilarity(
      const Track& track,
      const CameraObservationProto& camera_observation) const;
};

}  // namespace tracker

#endif  // TRACKER_ASSOCIATION_LIDAR_TRACKER_ASSOCIATOR_H_
