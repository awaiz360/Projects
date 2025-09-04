#ifndef TRACKER_MEASUREMENT_NOISE_CALCULATOR_H_
#define TRACKER_MEASUREMENT_NOISE_CALCULATOR_H_

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES  // for PI constant
#endif                     // !_USE_MATH_DEFINES
#include <math.h>

#include <cmath>
#include <iostream>
#include <tuple>
#include <utility>

#include "Eigen/Dense"
#include "logger.h"
#include "types.h"

namespace tracker {

template <typename T>
constexpr T Sqr(T val) {
  return val * val;
}

class ObservationNoiseComputator {
 public:
  using Matrix2d = Eigen::Matrix<double, 2, 2>;
  using Matrix3d = Eigen::Matrix<double, 3, 3>;

  explicit ObservationNoiseComputator(TrackerEngineType engine_mode_input)
      : engine_mode_(engine_mode_input) {}

  Matrix3d ComputeXYYawObservationNoise(
      const ObservationProto& observation) const {
    constexpr double kStateXObservationNoiseStd = 2.0;
    constexpr double kStateYObservationNoiseStd = 2.0;
    constexpr double kStateYawObservationNoiseStd = 0.7;
    constexpr double kStateXYObservationCorrelationCoefficient = 0.0;
    constexpr double kStateXYawObservationCorrelationCoefficient = 0.0;
    constexpr double kStateYYawObservationCorrelationCoefficient = 0.0;
    double kLargeValue = 1.0;
    if (engine_mode_ == TET_FUSION) {
      kLargeValue = 3.0;
    }

    Matrix3d noise = Matrix3d::Zero();
    if (observation.has_lidar_observation()) {
      noise(0, 0) = Sqr(kStateXObservationNoiseStd);
      noise(1, 1) = Sqr(kStateYObservationNoiseStd);
      noise(2, 2) = Sqr(kStateYawObservationNoiseStd);
      noise(0, 1) = kStateXYObservationCorrelationCoefficient *
                    kStateXObservationNoiseStd * kStateYObservationNoiseStd;
      noise(1, 0) = noise(0, 1);
      noise(0, 2) = kStateXYawObservationCorrelationCoefficient *
                    kStateXObservationNoiseStd * kStateYawObservationNoiseStd;
      noise(2, 0) = noise(0, 2);
      noise(1, 2) = kStateYYawObservationCorrelationCoefficient *
                    kStateYObservationNoiseStd * kStateYawObservationNoiseStd;
      noise(2, 1) = noise(1, 2);
    } else if (observation.has_camera_observation()) {
      noise(0, 0) = Sqr(kStateXObservationNoiseStd * kLargeValue);
      noise(1, 1) = Sqr(kStateYObservationNoiseStd * kLargeValue);
      noise(2, 2) = Sqr(kStateYawObservationNoiseStd * kLargeValue);
      noise(0, 1) = kStateXYObservationCorrelationCoefficient *
                    kStateXObservationNoiseStd * kStateYObservationNoiseStd *
                    Sqr(kLargeValue);
      noise(1, 0) = noise(0, 1);
      noise(0, 2) = kStateXYawObservationCorrelationCoefficient *
                    kStateXObservationNoiseStd * kStateYawObservationNoiseStd *
                    Sqr(kLargeValue);
      noise(2, 0) = noise(0, 2);
      noise(1, 2) = kStateYYawObservationCorrelationCoefficient *
                    kStateYObservationNoiseStd * kStateYawObservationNoiseStd *
                    Sqr(kLargeValue);
      noise(2, 1) = noise(1, 2);
    } else {
      Logger::GetInstance().Log(
          Logger::ERROR,
          "Error: Unknown observation type in measurement noise calculation.");
      std::cout << "Unknown observation type." << std::endl;
    }
    return noise;
  }

  Matrix2d ComputeVelObservationNoise(
      const ObservationProto& observation) const {
    Matrix2d noise = Matrix2d::Zero();
    if (observation.has_radar_observation()) {
      noise = VelocityCovariance(observation);
    } else {
      Logger::GetInstance().Log(
          Logger::ERROR,
          "Error: Mistaken or Unknown observation type in "
          "measurement noise calculation.");
      std::cout << "Unknown observation type." << std::endl;
    }
    return noise;
  }

 private:
  TrackerEngineType engine_mode_ = TrackerEngineType::TET_UNKNOWN;
  // Function to calculate covariance of magnitude and heading
  Matrix2d VelocityCovariance(const ObservationProto& observation) const {
    Matrix2d J = Matrix2d::Zero(2, 2);
    Matrix2d velCov = Matrix2d::Zero(2, 2);
    if (observation.has_radar_observation()) {
      velCov(0, 0) = observation.radar_observation().vel_cov().v0();
      velCov(0, 1) = observation.radar_observation().vel_cov().v1();
      velCov(1, 0) = observation.radar_observation().vel_cov().v2();
      velCov(1, 1) = observation.radar_observation().vel_cov().v3();

      double vx = observation.radar_observation().velocity_x();
      double vy = observation.radar_observation().velocity_y();
      double magnitude = 0;
      double heading = 0;
      std::tie(magnitude, heading) = CalculatePolarCoordinatesSafely(vx, vy);

      // Jacobian matrix
      if (magnitude < 1e-6) {
        magnitude = 1e-6;
      }
      J(0, 0) = vx / magnitude;
      J(0, 1) = vy / magnitude;
      J(1, 0) = -vy / (magnitude * magnitude);
      J(1, 1) = vx / (magnitude * magnitude);
      // Calculate and return the covariance of magnitude and heading
    }
    return J * velCov * J.transpose();
  }

  std::pair<double, double> CalculatePolarCoordinatesSafely(double x,
                                                            double y) const {
    // Magnitude Calculation (Safe for Overflow)
    double magnitude = std::hypot(x, y);

    // Angle Calculation (Handle Special Cases)
    double angle_radians;
    if (x == 0.0 && y == 0.0) {
      angle_radians = 0.0;  // Or any default you prefer
    } else if (x == 0.0) {
      angle_radians = (y > 0.0) ? M_PI_2 : -M_PI_2;
    } else {
      angle_radians = std::atan2(y, x);
    }

    return std::make_pair(magnitude, angle_radians);
  }
};
}  // namespace tracker
#endif  // TRACKER_MEASUREMENT_NOISE_CALCULATOR_H_
