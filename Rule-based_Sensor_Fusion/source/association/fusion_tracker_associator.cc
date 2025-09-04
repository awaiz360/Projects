#include "association/fusion_tracker_associator.h"

#include "Eigen/Core"
#include "iou.h"
#include "logger.h"
#include "type_gate.h"
#include "util.h"

namespace tracker {

// [0.0, 1.0] for valid similarity
double FusionTrackerAssociator::CameraSimilarity(
    const Track& track,
    const CameraObservationProto& camera_observation) const {
  // TODO(vahid) add type gate here
  // EllipseSimilarity
  const double ellipse_sim = util::TwoEllipseSimilarity(
      track, camera_observation, *localization_service_);
  // Gate
  if (ellipse_sim < 0.0) {
    return 0.0;
  }
  // Sum similarity
  const double sim = ellipse_sim;
  return sim;
}

FusionTrackerAssociator::FusionTrackerAssociator(
    const LocalizationService* localization_service)
    : AssociatorBase(localization_service) {}

double FusionTrackerAssociator::ComputeSimilarity(
    const Track& track, const ObservationProto& observation) const {
  // Predict track to observation timestamp
  const auto [predicted_state, _] =
      track.track_state.estimator.ComputePrediction(observation.timestamp());

  RectangleYaw trkRect = {predicted_state(0, 0), predicted_state(1, 0),
                          track.track_state.bounding_box.width(),
                          track.track_state.bounding_box.length(),
                          predicted_state(3, 0)};

  double similarity = 0.0;
  if (observation.has_lidar_observation()) {
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
    similarity = dist_similarity * 0.6 + iou * 0.4;
  } else if (observation.has_camera_observation()) {
    similarity = CameraSimilarity(track, observation.camera_observation());
  } else if (observation.has_radar_observation()) {
    // L2 --> later replace with mahalanobis distance.
    Eigen::Matrix<double, 4, 1> predicted_state_cartesian(
        predicted_state(0, 0), predicted_state(1, 0),
        predicted_state(2, 0) * std::cos(predicted_state(3, 0)),
        predicted_state(2, 0) * std::sin(predicted_state(3, 0)));

    const double dist = (predicted_state_cartesian -
                         Eigen::Matrix<double, 4, 1>(
                             observation.radar_observation().position_x(),
                             observation.radar_observation().position_y(),
                             observation.radar_observation().velocity_x(),
                             observation.radar_observation().velocity_y()))
                            .norm();  // L2;

    constexpr double kMaxDist = 6.0;
    similarity = dist > kMaxDist
                     ? 0.0
                     : 1.0 - dist / kMaxDist;  // euclidean distance [0, 1]
  } else {
    Logger::GetInstance().Log(Logger::ERROR,
                              "Error: Unknown observation type.");
    std::cout << "Unknown observation type." << std::endl;
  }

  return similarity;
}

}  // namespace tracker
