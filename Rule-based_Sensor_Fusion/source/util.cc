#include "util.h"

namespace tracker::util {

const TrackState* FindLatestCheckPointBeforeTime(const Track& track,
                                                 double timestamp) {
  for (int i = track.checkpoints.size() - 1; i >= 0; --i) {
    if (track.checkpoints[i].timestamp < timestamp) {
      return &track.checkpoints[i];
    }
  }
  return nullptr;
}

// If observation lies in ellipse, sim is (0.0, 1.0]
// If observation lies on ellipse, sim is 0.0
// If observation lies out of ellipse, sim is -1.0
double CalculateEllipseSimilarity(const Eigen::Vector2d& track_pos,
                                  const Eigen::Vector2d& observation_pos,
                                  const Eigen::Vector2d& major_axis_dir,
                                  double major_axis_threshold,
                                  double minor_axis_threshold) {
  const Eigen::Vector2d offset = observation_pos - track_pos;
  const double observation_in_major_axis = offset.dot(major_axis_dir);
  if (fabs(observation_in_major_axis) > major_axis_threshold) {
    return -1.0;
  }

  const Eigen::Vector2d minor_axis_dir =
      Eigen::Vector2d(-major_axis_dir.y(), major_axis_dir.x());
  const double observation_in_minor_axis = offset.dot(minor_axis_dir);
  if (fabs(observation_in_minor_axis) > minor_axis_threshold) {
    return -1.0;
  }

  const double dx =
      observation_in_major_axis /
      (major_axis_threshold + std::numeric_limits<double>::epsilon());
  const double dy =
      observation_in_minor_axis /
      (minor_axis_threshold + std::numeric_limits<double>::epsilon());
  const double dist2 = dx * dx + dy * dy;
  return dist2 > 1.0 ? -1.0 : 1.0 - dist2;
}

double TwoEllipseSimilarity(const Track& track,
                            const CameraObservationProto& camera_observation,
                            const LocalizationService& localization_service) {
  const double obs_timestamp = camera_observation.timestamp();
  const auto* ego_pose_info =
      localization_service.GetPoseAtTimestamp(obs_timestamp);
  if (nullptr == ego_pose_info) {
    return 0.0;
  }

  const auto* ckpt = util::FindLatestCheckPointBeforeTime(track, obs_timestamp);
  if (nullptr == ckpt) {
    return 0.0;
  }
  const auto [predicted_state, _] =
      ckpt->estimator.ComputePrediction(obs_timestamp);

  // The 1st ellipse use track viewing direction as major-axis and set threshold
  // according to depth.
  const Eigen::Vector2d track_pos = predicted_state.block<2, 1>(0, 0);
  const Eigen::Vector2d observation_pos = Eigen::Vector2d(
      camera_observation.center_x(), camera_observation.center_y());
  const Eigen::Vector2d ego_pos = ego_pose_info->pose.block<2, 1>(0, 3);
  const Eigen::Vector2d track_viewing_vector = track_pos - ego_pos;
  const double track_viewing_dist = track_viewing_vector.norm();
  const Eigen::Vector2d track_viewing_dir =
      track_viewing_vector /
      (track_viewing_dist + std::numeric_limits<double>::epsilon());

  constexpr double kEllipseAxisDistFactor = 0.15;
  constexpr double kMajorAxisMin = 8.0;
  const double major_axis =
      std::max(track_viewing_dist * kEllipseAxisDistFactor, kMajorAxisMin);

  constexpr double kMinorAxisMin = 3.0;
  const double width_length_ratio = track.track_state.bounding_box.width() /
                                    (track.track_state.bounding_box.length() +
                                     std::numeric_limits<double>::epsilon());
  const double minor_axis =
      std::max(major_axis * width_length_ratio, kMinorAxisMin);

  const double viewing_ellipse_sim = CalculateEllipseSimilarity(
      track_pos, observation_pos, track_viewing_dir, major_axis, minor_axis);

  // If observation is out of ellipse, gate it.
  if (viewing_ellipse_sim < 0.0) {
    return -1.0;
  }

  // The 2nd ellipse use track heading direction as major-axis.
  double heading_ellipse_sim = 0.0;
  if (track.track_state.type != OT_PEDESTRIAN) {
    Eigen::Vector2d track_heading = predicted_state.block<2, 1>(2, 0);
    track_heading = track_heading.normalized();
    constexpr double kMajorAxisThreshold = 4.5;
    constexpr double kMinorAxisThreshold = 2.5;
    heading_ellipse_sim =
        CalculateEllipseSimilarity(track_pos, observation_pos, track_heading,
                                   kMajorAxisThreshold, kMinorAxisThreshold);
    // If observation is out of ellipse, gate it.
    if (heading_ellipse_sim < 0.0) {
      return -1.0;
    }
  }

  return std::max(viewing_ellipse_sim, heading_ellipse_sim);
}

}  // namespace tracker::util
