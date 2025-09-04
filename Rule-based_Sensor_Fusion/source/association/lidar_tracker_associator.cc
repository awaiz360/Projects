#include "association/lidar_tracker_associator.h"

#include "Eigen/Core"
#include "iou.h"
#include "logger.h"
#include "type_gate.h"
#include "util.h"

namespace tracker {

LidarTrackerAssociator::LidarTrackerAssociator(
    const LocalizationService* localization_service)
    : AssociatorBase(localization_service) {}

// TODO: Refactor this function, remove unnecessary code.
double LidarTrackerAssociator::ComputeSimilarity(
    const Track& track, const ObservationProto& observation) const {
  if (!(observation.has_lidar_observation())) {
    Logger::GetInstance().Log(
        Logger::ERROR, "Error: non-lidar observation in lidar associator.");
  }
  // Predict track to observation timestamp
  const auto [predicted_state, _] =
      track.track_state.estimator.ComputePrediction(observation.timestamp());

  RectangleYaw trkRect = {predicted_state(0, 0), predicted_state(1, 0),
                          track.track_state.bounding_box.width(),
                          track.track_state.bounding_box.length(),
                          predicted_state(3, 0)};

  RectangleYaw detRectLidar = {observation.lidar_observation().center_x(),
                               observation.lidar_observation().center_y(),
                               observation.lidar_observation().width(),
                               observation.lidar_observation().length(),
                               observation.lidar_observation().yaw()};
  const double iou = CalculateIoU(detRectLidar, trkRect);  // iou [0, 1]
  const double dist =
      (predicted_state.block<2, 1>(0, 0) -
       Eigen::Vector2d(observation.lidar_observation().center_x(),
                       observation.lidar_observation().center_y()))
          .norm();  // L2
  constexpr double kMaxDist = 6.0;
  const double dist_similarity =
      dist > kMaxDist ? 0.0
                      : 1.0 - dist / kMaxDist;  // euclidean distance [0, 1]
  return dist_similarity * 0.6 + iou * 0.4;
}

}  // namespace tracker
