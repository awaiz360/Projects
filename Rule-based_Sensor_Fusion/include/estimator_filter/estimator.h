#ifndef TRACKER_ESTIMATOR_H_
#define TRACKER_ESTIMATOR_H_

#include <memory>

#include "const_def.h"
#include "estimator_filter/imm_filter.h"
#include "logger.h"
#include "types.h"

namespace tracker {

class Estimator {
 public:
  Estimator() = default;

  void Init(double x, double y, double v, double yaw, double yawdot,
            double timestamp);

  std::pair<Vector5d, Matrix5d> ComputePrediction(double timestamp) const;

  void PredictAndUpdate(const ObservationProto& observation,
                        const Matrix3d& measurement_noise);
  void PredictAndUpdate(const ObservationProto& observation,
                        const Matrix2d& measurement_noise);

  double x() const;
  double y() const;
  double vx() const;
  double vy() const;
  double yaw() const;
  double yawdot() const;
  Eigen::Vector2d pos() const;
  Eigen::Vector2d vel() const;
  Matrix5d P_matrix() const;

 private:
  CIMMFilter kf_;
  double kf_timestamp_;
};

}  // namespace tracker

#endif  // TRACKER_ESTIMATOR_H_
