#ifndef TRACKER_CV_EKF_H
#define TRACKER_CV_EKF_H

#define _USE_MATH_DEFINES  // for PI constant
#include <math.h>

#include <Eigen/Dense>

#include "types.h"

namespace tracker {

class CVEKF {
 public:
  CVEKF() : initialized_(false) {
    // Initialize matrices and vectors
    // states: {position X, position Y, Velocity magnetiude, heading, heading
    // rate}
    x_ = Vector5d::Zero(5);
    P_ = Matrix5d::Identity(5, 5);
    Q_ =
        Matrix5d::Identity(5, 5);  // process noise based on the system dynamics
    R_ = Matrix3d::Identity(
        3, 3);  // measurement noise based on the sensor accuracy

    // Measurement function Jacobian (H_) is initialized to zero. It will be
    // calculated dynamically during update.
    H_ = Matrix35d::Zero(3, 5);
  }

  void Initialize(const Vector5d& initial_state,
                  const Matrix5d& initial_covariance) {
    initialized_ = true;
    x_ = initial_state;
    P_ = initial_covariance;
  }

  std::pair<Vector5d, Matrix5d> Compute_Prediction(double dt) const {
    // State transition function (f)
    double v = x_(2);    // velocity magnitude
    double yaw = x_(3);  // yaw angle
    double yaw_rate = x_(4);

    Vector5d xp = x_;
    Matrix5d Pp = P_;

    // Update the state vector x_ using the CTRV motion model
    xp(0) += v * cos(yaw) * dt;
    xp(1) += v * sin(yaw) * dt;
    xp(2) += 0;  // Constant velocity assumption
    xp(3) += yaw_rate * dt;
    xp(4) += 0;  // Constant yaw rate assumption

    // Jacobian of the state transition function (F)
    Matrix5d F = Matrix5d::Zero(5, 5);
    F(0, 2) = dt * cos(yaw);
    F(0, 3) = -v * dt * sin(yaw);
    F(1, 2) = dt * sin(yaw);
    F(1, 3) = v * dt * cos(yaw);
    F(3, 4) = dt;
    F.diagonal().setOnes();

    // Predict state covariance
    Pp = F * Pp * F.transpose() + ComputeProcessNoise(dt);
    return std::pair(xp, Pp);
  }

  void Predict(double dt) {
    const auto [x_pred, P_pred] = Compute_Prediction(dt);
    x_ = x_pred;
    P_ = P_pred;

    // Update covariances in the class
    Q_ = ComputeProcessNoise(dt);
  }

  void UpdatePos(const double x_measurement, const double y_measurement,
                 const double yaw_measurement,
                 const Matrix3d& measurement_noise) {
    Vector3d z;
    z << x_measurement, y_measurement, yaw_measurement;
    R_ = measurement_noise;

    // Calculate measurement Jacobian (H_)
    H_(0, 0) = 1;
    H_(1, 1) = 1;
    H_(2, 3) = 1;

    // Standard Kalman filter update steps
    Matrix3d S = H_ * P_ * H_.transpose() + R_;
    Matrix53d K = P_ * H_.transpose() * S.inverse();
    Vector3d y = z - H_ * x_;
    y(2) = wrapToPI(y(2));
    x_ = x_ + K * y;
    P_ = (Matrix5d::Identity(5, 5) - K * H_) * P_;
  }

  void UpdateVel(const double Vx_measurement, const double Vy_measurement,
                 const Matrix2d& measurement_noise) {
    Vector2d z;
    std::tie(z(0), z(1)) =
        CalculatePolarCoordinatesSafely(Vx_measurement, Vy_measurement);
    Matrix2d R = measurement_noise;

    // Calculate measurement Jacobian (H_)
    Matrix25d H = Matrix25d::Zero(2, 5);
    H(0, 2) = 1;
    H(1, 3) = 1;

    // Standard Kalman filter update steps
    Matrix2d S = H * P_ * H.transpose() + R;
    Matrix52d K = P_ * H.transpose() * S.inverse();
    Vector2d y = z - H * x_;
    y(1) = wrapToPI(y(1));
    x_ = x_ + K * y;
    P_ = (Matrix5d::Identity(5, 5) - K * H) * P_;
  }

  void UpdatePosVel(const double x_measurement, const double y_measurement,
                    const double Vx_measurement, const double Vy_measurement,
                    const Matrix4d& measurement_noise) {
    Vector4d z;
    z(0) = x_measurement;
    z(1) = y_measurement;
    std::tie(z(2), z(3)) =
        CalculatePolarCoordinatesSafely(Vx_measurement, Vy_measurement);
    Matrix4d R = measurement_noise;

    // Calculate measurement Jacobian (H_)
    Matrix45d H = Matrix45d::Zero(4, 5);
    H(0, 1) = 1;
    H(1, 1) = 1;
    H(2, 2) = 1;
    H(3, 3) = 1;

    // Standard Kalman filter update steps
    Matrix4d S = H * P_ * H.transpose() + R;
    Matrix54d K = P_ * H.transpose() * S.inverse();
    Vector4d y = z - H * x_;
    y(3) = wrapToPI(y(3));
    x_ = x_ + K * y;
    P_ = (Matrix5d::Identity(5, 5) - K * H) * P_;
  }

  double likelihoods(Vector3d y) {
    Vector3d mu = H_ * x_;
    Matrix3d R = R_ * 1000;
    return mvnpdf(y, mu, R);
  }

  double x() const { return x_(0); }
  double y() const { return x_(1); }
  double v() const { return x_(2); }
  double vx() const { return (x_(2) * cos(x_(3))); }
  double vy() const { return (x_(2) * sin(x_(3))); }
  double yaw() const { return x_(3); }
  double yawdot() const { return x_(4); }
  Eigen::Vector2d pos() const { return x_.block<2, 1>(0, 0); }
  Eigen::Vector2d vel() const {
    Eigen::Vector2d V_return;
    V_return << vx(), vy();
    return (V_return);
  }
  Vector5d state() const { return x_; }
  Matrix5d covariance_matrix() const { return P_; }

 private:
  bool initialized_;
  Vector5d x_;   // State vector (px, py, v, yaw, yaw_rate)
  Matrix5d P_;   // State covariance
  Matrix5d Q_;   // Process noise covariance
  Matrix3d R_;   // Measurement noise covariance
  Matrix35d H_;  // Measurement Jacobian

  Matrix5d ComputeProcessNoise(double dt) const {
    Matrix5d Q = Matrix5d::Zero(5, 5);

    const double dt2 = dt * dt;
    const double dt3 = dt2 * dt;
    const double dt4 = dt3 * dt;

    // Process noise parameters (need tunning based on the system's
    // characteristics)
    constexpr double v_rate_var = 0.2 * 0.2;
    constexpr double yaw_accel_var = 0;  // Variance of yaw acceleration

    // Common factors for readability
    const double x_common_factor = 0.5 * v_rate_var;
    const double y_common_factor = 0.5 * v_rate_var;
    const double yaw_common_factor = 0.5 * yaw_accel_var;

    // Fill in Q matrix elements
    Q(0, 0) = x_common_factor * dt4 * 0.5;
    Q(0, 2) = x_common_factor * dt3;

    Q(1, 1) = y_common_factor * dt4 * 0.5;
    Q(1, 3) = y_common_factor * dt3;

    Q(2, 0) = Q(0, 2);
    Q(2, 2) = v_rate_var * dt2;

    Q(3, 3) = yaw_common_factor * dt4 * 0.5;
    Q(3, 4) = yaw_common_factor * dt3;

    Q(4, 3) = Q(3, 4);
    Q(4, 4) = yaw_accel_var * dt2;

    return Q;
  }

  std::pair<double, double> CalculatePolarCoordinatesSafely(double x,
                                                            double y) {
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

  double mvnpdf(Vector3d X, Vector3d Mu, Matrix3d Sigma) {
    if (X.size() != Mu.size()) return 0;
    if (Sigma.rows() != Sigma.cols()) return 0;
    if (X.size() != Sigma.rows()) return 0;
    int d = X.size();
    Vector3d X0 = X - Mu;
    X0(2) = wrapToPI(X0(2));
    Matrix3d R(Sigma.llt().matrixL());
    Vector3d xRinv = R.inverse() * X0;
    double logSqrtDetSigma = 0;
    for (int i = 0; i < d; i++) logSqrtDetSigma += log(R(i, i));
    double quadform = 0;
    for (int i = 0; i < d; i++) quadform += xRinv(i) * xRinv(i);
    return exp(-0.5 * quadform - logSqrtDetSigma - d * log(2 * M_PI) / 2) +
           1e-27;
  }

  double wrapTo2PI(double angle) {
    double res = fmod(angle, 2 * M_PI);
    if (res < 0) res += 2 * M_PI;
    return res;
  }
  double wrapToPI(double angle) {
    double res = wrapTo2PI(angle);
    if (res > M_PI) res -= 2 * M_PI;
    return res;
  }
};
}  // namespace tracker
#endif  // TRACKER_CV_EKF_H
