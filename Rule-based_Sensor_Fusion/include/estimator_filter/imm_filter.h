#ifndef TRACKER_IMM_FILTER_H
#define TRACKER_IMM_FILTER_H

#include <math.h>

#include "Eigen/Cholesky"
#include "estimator_filter/ctrv_ekf.h"
#include "estimator_filter/cv_ekf.h"
namespace tracker {
class CIMMFilter {
 public:
  CIMMFilter() : initialized_(false) {};
  void Initialize(const Vector5d& initial_state,
                  const Matrix5d& initial_covariance) {
    initialized_ = true;
    x_ = initial_state;
    P_ = initial_covariance;
    kf1_.Initialize(x_, P_);
    kf2_.Initialize(x_, P_);
    mode_probabilities_ = Vector2d::Zero(2);
    mode_probabilities_(0) = 0.5;
    mode_probabilities_(1) = 0.5;

    transfer_prob_ = Matrix2d::Zero(2, 2);
    transfer_prob_ << 0.9, 0.1, 0.1, 0.9;
    c_ = Vector2d::Zero(2);
    X_ = Matrix52d::Zero(5, 2);
  };

  std::pair<Vector5d, Matrix5d> Compute_Prediction(double dt) const {
    const auto [x_pred_k1, P_pred_k1] = kf1_.Compute_Prediction(dt);
    const auto [x_pred_k2, P_pred_k2] = kf2_.Compute_Prediction(dt);
    Vector5d x =
        x_pred_k1 * mode_probabilities_(0) + x_pred_k2 * mode_probabilities_(1);
    Matrix5d P = Matrix5d::Zero(5, 5);
    Vector5d v = x_pred_k1 - x;
    P += (P_pred_k1 + v * v.transpose()) * mode_probabilities_(0);
    v = x_pred_k2 - x;
    P += (P_pred_k2 + v * v.transpose()) * mode_probabilities_(1);

    return {x, P};
  }

  void Predict(double dt) {
    stateInteraction();
    kf1_.Predict(dt);
    kf2_.Predict(dt);
  }

  void UpdatePos(const double& x_measurement, const double& y_measurement,
                 const double& yaw_measurement,
                 const Matrix3d& measurement_noise) {
    Vector3d y1 = Vector3d::Zero(3);
    y1 << x_measurement, y_measurement, yaw_measurement;
    Vector2d y2 = Vector2d::Zero(2);
    y2 << x_measurement, y_measurement;

    // int res = updateModeProbability(y1,y2);
    kf1_.UpdatePos(x_measurement, y_measurement, yaw_measurement,
                   measurement_noise);
    kf2_.UpdatePos(x_measurement, y_measurement, yaw_measurement,
                   measurement_noise);
    updateModelProb(y1, y1);
    //    if (mode_probabilities_[0] > mode_probabilities_[1])
    //    {
    //        x_ = kf1_.state();
    //        P_ = kf1_.covariance_matrix();
    //    }
    //    else
    //    {
    //        x_ = kf2_.state();
    //        P_ = kf2_.covariance_matrix();
    //    }
    X_.col(0) = kf1_.state();
    X_.col(1) = kf2_.state();
    x_ = X_ * mode_probabilities_;
    auto heading = x_(3);
    P_ = Matrix5d::Zero(5, 5);
    Vector5d v = X_.col(0) - x_;
    P_ +=
        (kf1_.covariance_matrix() + v * v.transpose()) * mode_probabilities_(0);
    v = X_.col(1) - x_;
    P_ +=
        (kf2_.covariance_matrix() + v * v.transpose()) * mode_probabilities_(1);
  }

  void UpdateVel(const double& Vx_measurement, const double& Vy_measurement,
                 const Matrix2d& measurement_noise) {
    ;
  }

  double x() const { return x_(0); }
  double y() const { return x_(1); }
  double v() const { return x_(2); }
  double vx() const { return (x_(2) * cos(x_(3))); }
  double vy() const { return (x_(2) * sin(x_(3))); }
  double yaw() const { return x_(3); }
  double yawdot() const { return x_(4); }
  Vector2d pos() const { return x_.block<2, 1>(0, 0); }
  Vector2d vel() const {
    Vector2d V_return;
    V_return << vx(), vy();
    return (V_return);
  }
  Vector5d state() const { return x_; }
  Matrix5d covariance_matrix() const { return P_; }

 private:
  bool initialized_;
  CTRVEKF kf1_;
  CVEKF kf2_;
  Vector2d mode_probabilities_;
  Vector5d x_;
  Matrix52d X_;
  Matrix5d P_;
  Matrix2d transfer_prob_;
  Vector2d c_;

 private:
  int updateModeProbability(Vector3d y1, Vector3d y2) {
    auto l1 = kf1_.likelihoods(y1);
    auto l2 = kf2_.likelihoods(y2);
    mode_probabilities_(0) = mode_probabilities_(0) * l1;
    mode_probabilities_(1) = mode_probabilities_(1) * l2;
    mode_probabilities_ = mode_probabilities_ / mode_probabilities_.sum();
    return mode_probabilities_(0) > mode_probabilities_(1)
               ? mode_probabilities_(0)
               : mode_probabilities_(1);
  }
  void stateInteraction() {
    Matrix52d states = Matrix52d::Zero(5, 2);
    states.col(0) = kf1_.state();
    states.col(1) = kf2_.state();
    c_ = Vector2d::Zero(2);
    for (size_t j = 0; j < 2; j++) {
      for (size_t i = 0; i < 2; i++) {
        c_(j) += transfer_prob_(i, j) * mode_probabilities_(i);
      }
    }
    Matrix2d U = Matrix2d::Zero(2, 2);
    for (size_t j = 0; j < 2; j++) {
      X_.col(j) = states.col(j);
      X_.col(j) = states.col(j);
    }

    Matrix52d X = X_;
    X_.fill(0);
    for (size_t j = 0; j < 2; j++) {
      for (size_t i = 0; i < 2; i++) {
        U(i, j) = 1.0 / c_(j) * transfer_prob_(i, j) * mode_probabilities_(i);
        X_.col(j) += X.col(i) * U(i, j);
      }
    }

    Matrix5d P = Matrix5d::Zero(5, 5);
    for (size_t j = 0; j < 2; j++) {
      Vector5d s = X.col(0) - X_.col(j);
      P += U(0, j) * (kf1_.covariance_matrix() + s * s.transpose());
    }
    kf1_.Initialize(X_.col(0), P);
    P = Matrix5d::Zero(5, 5);
    for (size_t j = 0; j < 2; j++) {
      Vector5d s = X.col(1) - X_.col(j);
      P += U(1, j) * (kf2_.covariance_matrix() + s * s.transpose());
    }
    kf2_.Initialize(X_.col(1), P);
  }

  void updateModelProb(Vector3d y1, Vector3d y2) {
    double likelihood[2];
    likelihood[0] = kf1_.likelihoods(y1);
    likelihood[1] = kf2_.likelihoods(y2);
    double c_sum = 0;
    c_sum += likelihood[0] * c_(0) + likelihood[1] * c_(1);
    c_sum += 1e-27;

    for (size_t i = 0; i < 2; i++) {
      mode_probabilities_(i) = likelihood[i] * c_(i) / c_sum;
    }
  }
};

}  // namespace tracker

#endif  // TRACKER_IMM_FILTER_H
