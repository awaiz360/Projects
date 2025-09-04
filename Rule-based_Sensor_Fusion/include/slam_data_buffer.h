#ifndef TRACKER_SLAM_DATA_BUFFER_H
#define TRACKER_SLAM_DATA_BUFFER_H

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "vecXd.h"

namespace tracker {

struct EgoCarInfo {
  double ego_odom_time_s{0.0};
  std::string status_inf;
  uint8_t origin_id{0};
  Vec3d position;
  Vec3d euler_angles;  // Roll;Pitch:Yaw
  Vec3d linear_velocity;
  Vec3d angular_velocity;
  Vec3d linear_acceleration;
  Vec3d angular_acceleration;

  /*friend bool operator==(const EgoCarInfo& lhs, const EgoCarInfo& rhs)
  noexcept { return (fabs(lhs.ego_odom_time_ns - rhs.ego_odom_time_ns) <
  DBL_EPSILON) && (lhs.status_inf == rhs.status_inf) && (lhs.origin_id ==
  rhs.origin_id);
  }*/
};

class SLAMDataBuffer {
 public:
  explicit SLAMDataBuffer(size_t capacity) : buffer_(capacity) {}

  void addData(const EgoCarInfo& data) {
    buffer_[head_] = data;
    head_ = (head_ + 1) % buffer_.size();
    size_ = std::min(size_ + 1, buffer_.size());
  }

  void getInterpolatedData(const double time, EgoCarInfo* interpolatedData) {
    targetTime_ = time;
    if (size_ > 1) {
      auto [before, after] = findInterpolationPoints();
      interpolate(buffer_.at(before), buffer_.at(after), interpolatedData);
    }
  }

 private:
  std::vector<EgoCarInfo> buffer_;
  size_t head_{0};
  size_t size_{0};
  double targetTime_{0};

  std::pair<size_t, size_t> findInterpolationPoints() {
    if (size_ < 2) {
      Logger::GetInstance().Log(
          Logger::WARNING,
          "Warning: Not enough data in the buffer for interpolation.");
    }

    size_t i = (head_ - size_ + buffer_.size()) % buffer_.size();
    while (i != head_ && buffer_[i].ego_odom_time_s <= targetTime_) {
      i = (i + 1) % buffer_.size();
    }
    if (i == head_) {
      i = (i - 1 + buffer_.size()) % buffer_.size();
    }
    size_t prevIndex = (i - 1 + buffer_.size()) % buffer_.size();
    return {prevIndex, i};
  }

  void interpolate(const EgoCarInfo& before, const EgoCarInfo& after,
                   EgoCarInfo* result) const {
    double fraction = (targetTime_ - before.ego_odom_time_s) /
                      (after.ego_odom_time_s - before.ego_odom_time_s);
    if (fraction < 0 || fraction > 1) {
      fraction = 1;
    }  // for safety
    // Interpolate Vec3d members (position, velocity and acceleration)
    result->ego_odom_time_s = targetTime_;
    result->euler_angles =
        before.euler_angles +
        fraction * (after.euler_angles - before.euler_angles);
    result->angular_velocity =
        before.angular_velocity +
        fraction * (after.angular_velocity - before.angular_velocity);
    result->angular_acceleration =
        before.angular_acceleration +
        fraction * (after.angular_acceleration - before.angular_acceleration);
    result->position =
        before.position + fraction * (after.position - before.position);
    result->linear_velocity =
        before.linear_velocity +
        fraction * (after.linear_velocity - before.linear_velocity);
    result->linear_acceleration =
        before.linear_acceleration +
        fraction * (after.linear_acceleration - before.linear_acceleration);
  }
};

}  // namespace tracker

#endif  // TRACKER_SLAM_DATA_BUFFER_H
