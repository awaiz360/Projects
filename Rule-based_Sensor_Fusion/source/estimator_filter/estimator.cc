#include "estimator_filter/estimator.h"

namespace tracker {

void Estimator::Init(double x, double y, double v, double yaw, double yawdot,
                     double timestamp) {
  Matrix5d Initial_P = 10 * Matrix5d::Identity(5, 5);
  Initial_P(3, 3) = 1;
  Initial_P(4, 4) = 1;
  Vector5d initial_state;
  initial_state << x, y, v, yaw, yawdot;
  kf_.Initialize(initial_state, Initial_P);
  kf_timestamp_ = timestamp;
}

std::pair<Vector5d, Matrix5d> Estimator::ComputePrediction(
    double timestamp) const {
  const double dt = timestamp - kf_timestamp_;
  // TODO(somebody) check dt > 0
  return kf_.Compute_Prediction(dt);
}

void Estimator::PredictAndUpdate(const ObservationProto& observation,
                                 const Matrix3d& measurement_noise) {
  const double dt = observation.timestamp() - kf_timestamp_;
  kf_.Predict(dt);

  if (observation.has_lidar_observation()) {
    const auto& lidar_observation = observation.lidar_observation();
    kf_.UpdatePos(lidar_observation.center_x(), lidar_observation.center_y(),
                  lidar_observation.yaw(), measurement_noise);
  } else if (observation.has_camera_observation()) {
    const auto& camera_observation = observation.camera_observation();
    kf_.UpdatePos(camera_observation.center_x(), camera_observation.center_y(),
                  camera_observation.yaw(), measurement_noise);
  } else {
    Logger::GetInstance().Log(Logger::ERROR,
                              "Error: Unknown observation type.");
    std::cout << "Unknown observation type." << std::endl;
  }
  kf_timestamp_ = observation.timestamp();
}

void Estimator::PredictAndUpdate(const ObservationProto& observation,
                                 const Matrix2d& measurement_noise) {
  const double dt = observation.timestamp() - kf_timestamp_;
  kf_.Predict(dt);

  if (observation.has_radar_observation()) {
    const auto& radar_observation = observation.radar_observation();
    kf_.UpdateVel(radar_observation.velocity_x(),
                  radar_observation.velocity_y(), measurement_noise);
  } else {
    Logger::GetInstance().Log(Logger::ERROR,
                              "Error: Unknown observation type.");
    std::cout << "Unknown observation type." << std::endl;
  }
  kf_timestamp_ = observation.timestamp();
}

double Estimator::x() const { return kf_.x(); }
double Estimator::y() const { return kf_.y(); }
double Estimator::vx() const { return kf_.vx(); }
double Estimator::vy() const { return kf_.vy(); }
double Estimator::yaw() const { return kf_.yaw(); }
double Estimator::yawdot() const { return kf_.yawdot(); }
Eigen::Vector2d Estimator::pos() const { return kf_.pos(); }
Eigen::Vector2d Estimator::vel() const { return kf_.vel(); }
Matrix5d Estimator::P_matrix() const { return kf_.covariance_matrix(); }

}  // namespace tracker
