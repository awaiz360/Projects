#ifndef TRACKER_POINT_CV_KALMAN_FILTER_H_
#define TRACKER_POINT_CV_KALMAN_FILTER_H_

#include "Eigen/Core"
#include "Eigen/LU"
// #include "lidar_tracker.pb.h"
#include "types.h"

namespace tracker {
// Naive kalman filter
class PointCvKalmanFilter {
 public:
  PointCvKalmanFilter() = default;

  PointCvKalmanFilter(double x, double y, double vx, double vy) {
    x_ << x, y, vx, vy;
    P_.setIdentity();
  }

  void Init(double x, double y, double vx, double vy) {
    x_ << x, y, vx, vy;
    P_.setIdentity();
  }

  std::pair<Eigen::Vector4d, Eigen::Matrix4d> ComputePrediction(
      double dt) const {
    // Compute transition matrix F
    Eigen::Matrix4d F;
    F.setIdentity();
    F(0, 2) = dt;
    F(1, 3) = dt;
    // Compute state transition
    const Eigen::Vector4d x_pred = F * x_;
    // Compute covariance
    const Eigen::Matrix4d Q = ComputeProcessNoise(dt);
    const Eigen::Matrix4d P = F * P_ * F.transpose() + Q;
    return {x_pred, P};
  }

  void Predict(double dt) {
    const auto [x_pred, P] = ComputePrediction(dt);
    x_ = x_pred;
    P_ = P;
  }

  void Update(double x, double y) {
    // Compute observation matrix H
    Eigen::Matrix<double, 2, 4> H = Eigen::Matrix<double, 2, 4>::Zero();
    H(0, 0) = 1.0;
    H(1, 1) = 1.0;
    // Compute measurement noise matrix
    Eigen::Matrix2d R = Eigen::Matrix2d::Zero();

    constexpr double state_x_measurement_noise_std = 0.05;
    constexpr double state_y_measurement_noise_std = 0.05;
    constexpr double state_x_y_measurement_correlation_coefficient = 0.0;

    R(0, 0) = state_x_measurement_noise_std * state_x_measurement_noise_std;
    R(1, 1) = state_y_measurement_noise_std * state_y_measurement_noise_std;
    R(0, 1) = state_x_y_measurement_correlation_coefficient *
              state_x_measurement_noise_std * state_y_measurement_noise_std;
    R(1, 0) = R(0, 1);

    // Compute Kalman gain
    const Eigen::Matrix2d S = H * P_ * H.transpose() + R;
    const Eigen::Vector2d hx = H * x_;
    const Eigen::Matrix<double, 4, 2> K = P_ * H.transpose() * S.inverse();

    // Update
    const Eigen::Vector2d z(x, y);
    const Eigen::Vector2d dz = z - hx;
    x_ += K * dz;
    P_ -= K * H * P_;
  }

  void Update(double x, double y, double vx, double vy) {
    // Compute observation matrix H
    Eigen::Matrix4d H = Eigen::Matrix4d::Identity();
    // Compute measurement noise matrix
    Eigen::Matrix4d R = Eigen::Matrix4d::Zero();

    constexpr double state_x_measurement_noise_std = 0.1;
    constexpr double state_y_measurement_noise_std = 0.1;
    constexpr double state_x_y_measurement_correlation_coefficient = 0.0;
    constexpr double state_vel_x_measurement_noise_std = 2.0;
    constexpr double state_vel_y_measurement_noise_std = 2.0;
    constexpr double state_vel_x_vel_y_measurement_correlation_coefficient =
        0.0;

    R(0, 0) = state_x_measurement_noise_std * state_x_measurement_noise_std;
    R(1, 1) = state_y_measurement_noise_std * state_y_measurement_noise_std;
    R(0, 1) = state_x_y_measurement_correlation_coefficient *
              state_x_measurement_noise_std * state_y_measurement_noise_std;
    R(1, 0) = R(0, 1);

    R(2, 2) =
        state_vel_x_measurement_noise_std * state_vel_x_measurement_noise_std;
    R(3, 3) =
        state_vel_y_measurement_noise_std * state_vel_y_measurement_noise_std;
    R(2, 3) = state_vel_x_vel_y_measurement_correlation_coefficient *
              state_vel_x_measurement_noise_std *
              state_vel_y_measurement_noise_std;
    R(2, 3) = R(3, 2);

    // Compute Kalman gain
    const Eigen::Matrix4d S = H * P_ * H.transpose() + R;
    const Eigen::Vector4d hx = H * x_;
    const Eigen::Matrix4d K = P_ * H.transpose() * S.inverse();

    // Update
    const Eigen::Vector4d z(x, y, vx, vy);
    const Eigen::Vector4d dz = z - hx;
    x_ += K * dz;
    P_ -= K * H * P_;
  }

  double x() const { return x_(0); }
  double y() const { return x_(1); }
  double vx() const { return x_(2); }
  double vy() const { return x_(3); }
  Eigen::Vector2d pos() { return x_.block<2, 1>(0, 0); }
  Eigen::Vector2d vel() { return x_.block<2, 1>(2, 0); }
  Eigen::Vector4d state() { return x_; }
  Eigen::Matrix4d covariance_matrix() { return P_; }

 private:
  Eigen::Matrix4d ComputeProcessNoise(double dt) const {
    Eigen::Matrix4d Q;
    Q.setZero();
    const double dt2 = dt * dt;
    const double dt3 = dt2 * dt;
    const double dt4 = dt3 * dt;

    constexpr double vx_rate_var = 0.2 * 0.2;
    constexpr double vy_rate_var = 0.2 * 0.2;

    const double x_common_factor = 0.5 * vx_rate_var;
    const double y_common_factor = 0.5 * vy_rate_var;

    const double q_s_x_x = x_common_factor * dt4 * 0.5;
    const double q_s_x_vel_x = x_common_factor * dt3;

    const double q_s_y_y = y_common_factor * dt4 * 0.5;
    const double q_s_y_vel_y = y_common_factor * dt3;

    Q(0, 0) = q_s_x_x;
    Q(0, 2) = q_s_x_vel_x;

    Q(1, 1) = q_s_y_y;
    Q(1, 3) = q_s_y_vel_y;

    Q(2, 0) = q_s_x_vel_x;
    Q(2, 2) = vx_rate_var * dt2;

    Q(3, 1) = q_s_y_vel_y;
    Q(3, 3) = vy_rate_var * dt2;

    return Q;
  }

  Eigen::Vector4d x_;
  Eigen::Matrix4d P_;
};
}  // namespace tracker

#endif  // TRACKER_POINT_CV_KALMAN_FILTER_H_
