#pragma once

#include "track.h"

namespace tracker {

class MotionTypeDetection {
 public:
  explicit MotionTypeDetection() = default;  // Default constructor

  void updateMotionType(double vx, double vy) {
    speed_x_ = vx;
    speed_y_ = vy;
    detectMotionType();
  }

  MotionType getMotion() const {  // Added accessor method
    return previous_motion_;
  }

 private:
  void detectMotionType() {
    double speed = std::sqrt(speed_x_ * speed_x_ + speed_y_ * speed_y_);

    if (speed < kSpeedThreshold) {
      if (previous_motion_ == MotionType::MOVING_DRIVING ||
          previous_motion_ == MotionType::MOVING_REVERSE ||
          previous_motion_ == MotionType::MOVING_CROSSING) {
        previous_motion_ = MotionType::MOVING_STOPPED;
      } else if (previous_motion_ == MotionType::MOVING_STOPPED ||
                 previous_motion_ == MotionType::STATIONARY) {
        stop_cycle_count_++;
        if (stop_cycle_count_ >= kMaxStopCycles) {
          previous_motion_ = MotionType::STATIONARY;
          stop_cycle_count_ = kMaxStopCycles;
        }
      } else {
        previous_motion_ = MotionType::STATIONARY;
        stop_cycle_count_ = 0;
      }
    } else {
      stop_cycle_count_ = 0;  // Reset the counter when moving
      if (speed_x_ > kSpeedThreshold) {
        previous_motion_ = MotionType::MOVING_DRIVING;
      } else if (speed_x_ < -kSpeedThreshold) {
        previous_motion_ = MotionType::MOVING_REVERSE;
      } else if (std::abs(speed_y_) > kSpeedThreshold) {
        previous_motion_ = MotionType::MOVING_CROSSING;
      } else {
        previous_motion_ = MotionType::UNKNOWN;
      }
    }
  }

  double speed_x_ = 0.0;
  double speed_y_ = 0.0;
  MotionType previous_motion_ = MotionType::MOVING_STOPPED;
  int stop_cycle_count_ = 0;

  static constexpr double kSpeedThreshold = 0.5;
  static constexpr int kMaxStopCycles = 100;
};

}  // namespace tracker
