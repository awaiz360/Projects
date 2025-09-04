#include "localization_service.h"

namespace tracker {

constexpr double kMaxLastTime = 10.0;  // 10 second

bool LocalizationService::PushBackPose(const Eigen::Matrix4d& pose,
                                       double timestamp) {
  if (!poses_.empty() && timestamp <= poses_.back().timestamp) {
    return false;
  }
  poses_.push_back(PoseWithTimestamp(timestamp, pose));
  while (!poses_.empty() &&
         poses_.back().timestamp - poses_.front().timestamp > kMaxLastTime) {
    poses_.pop_front();
  }
  return true;
}

const PoseWithTimestamp* LocalizationService::GetPoseAtTimestamp(
    double timestamp, double duration) const {
  if (poses_.empty()) {
    return nullptr;
  }

  const auto next_it = std::upper_bound(
      poses_.begin(), poses_.end(), timestamp,
      [](double ts, const auto& item) { return ts < item.timestamp; });

  // No pose is greater than timestamp
  if (next_it == poses_.end()) {
    if (std::fabs(poses_.back().timestamp - timestamp) <= duration) {
      return &poses_.back();
    } else {
      return nullptr;
    }
  }

  // All poses are greater than timestamp
  if (next_it == poses_.begin()) {
    if (fabs(poses_.front().timestamp - timestamp) <= duration) {
      return &poses_.front();
    } else {
      return nullptr;
    }
  }

  int next = next_it - poses_.begin();
  int prev = next - 1;

  const double prev_delta_time = fabs(poses_[prev].timestamp - timestamp);
  const double next_delta_time = fabs(poses_[next].timestamp - timestamp);
  if (std::min(prev_delta_time, next_delta_time) > duration) {
    return nullptr;
  }
  return prev_delta_time < next_delta_time ? &poses_[prev] : &poses_[next];
}

}  // namespace tracker
