#ifndef TRACKER_LOCALIZATION_SERVICE_
#define TRACKER_LOCALIZATION_SERVICE_

#include <deque>

#include "Eigen/Core"

namespace tracker {

struct PoseWithTimestamp {
  double timestamp;
  Eigen::Matrix4d pose;
  // Constructor for initialization
  PoseWithTimestamp(double t, const Eigen::Matrix4d& p)
      : timestamp(t), pose(p) {}
};

class LocalizationService {
 public:
  LocalizationService() = default;

  ~LocalizationService() = default;

  bool PushBackPose(const Eigen::Matrix4d& pose, const double timestamp);

  // Find pose cloest to timestamp within duration
  const PoseWithTimestamp* GetPoseAtTimestamp(
      const double timestamp, const double duration = 1e3) const;

 private:
  std::deque<PoseWithTimestamp> poses_;
};

}  // namespace tracker

#endif  // TRACKER_LOCALIZATION_SERVICE_
