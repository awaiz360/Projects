#ifndef TRACKER_MOTION_CONVERTER_H
#define TRACKER_MOTION_CONVERTER_H

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES  // for PI constant
#endif                     // !_USE_MATH_DEFINES
#include <math.h>

#include <Eigen/Geometry>  // Include Eigen for quaternion and vector operations
#include <cmath>
#include <tuple>

namespace tracker {

class MotionConverter {
 public:
  explicit MotionConverter() {};

  // Constructor taking current orientation (as a quaternion) and the observer's
  // motion
  void InsertObserverInfo(
      const Eigen::Vector3d& observerLinearPosition = Eigen::Vector3d::Zero(),
      const Eigen::Quaterniond& observerAngularPosition =
          Eigen::Quaterniond::Identity(),
      const Eigen::Vector3d& observerLinearVelocity = Eigen::Vector3d::Zero(),
      const Eigen::Vector3d& observerAngularVelocity = Eigen::Vector3d::Zero(),
      const Eigen::Vector3d& observerLinearAcceleration =
          Eigen::Vector3d::Zero(),
      const Eigen::Vector3d& observerAngularAcceleration =
          Eigen::Vector3d::Zero()) {
    observerLinearPosition_ = observerLinearPosition;
    observerAngularPosition_ = observerAngularPosition;
    observerLinearVelocity_ = observerLinearVelocity;
    observerAngularVelocity_ = observerAngularVelocity;
    observerLinearAcceleration_ = observerLinearAcceleration;
    observerAngularAcceleration_ = observerAngularAcceleration;
    return;
  }
  // Constructor taking Euler angles (roll, pitch, yaw in radians) for the
  // orientation and the observer's motion
  void InsertObserverInfo(
      const Eigen::Vector3d& observerLinearPosition = Eigen::Vector3d::Zero(),
      const Eigen::Vector3d& observerAngularPosition = Eigen::Vector3d::Zero(),
      const Eigen::Vector3d& observerLinearVelocity = Eigen::Vector3d::Zero(),
      const Eigen::Vector3d& observerAngularVelocity = Eigen::Vector3d::Zero(),
      const Eigen::Vector3d& observerLinearAcceleration =
          Eigen::Vector3d::Zero(),
      const Eigen::Vector3d& observerAngularAcceleration =
          Eigen::Vector3d::Zero()) {
    observerLinearPosition_ = observerLinearPosition;
    observerAngularPosition_ =
        Eigen::AngleAxisd(observerAngularPosition[2],
                          Eigen::Vector3d::UnitZ()) *
        Eigen::AngleAxisd(observerAngularPosition[1],
                          Eigen::Vector3d::UnitY()) *
        Eigen::AngleAxisd(observerAngularPosition[0], Eigen::Vector3d::UnitX());
    observerLinearVelocity_ = observerLinearVelocity;
    observerAngularVelocity_ = observerAngularVelocity;
    observerLinearAcceleration_ = observerLinearAcceleration;
    observerAngularAcceleration_ = observerAngularAcceleration;
    return;
  }

  void InsertObjectInfo(
      const Eigen::Vector3d& objectLinearPosition = Eigen::Vector3d::Zero(),
      const Eigen::Vector3d& objectAngularPosition = Eigen::Vector3d::Zero(),
      const Eigen::Vector3d& objectLinearVelocity = Eigen::Vector3d::Zero(),
      const Eigen::Vector3d& objectAngularVelocity = Eigen::Vector3d::Zero(),
      const Eigen::Vector3d& objectLinearAcceleration = Eigen::Vector3d::Zero(),
      const Eigen::Vector3d& objectAngularAcceleration =
          Eigen::Vector3d::Zero()) {
    objectLinearPosition_ = objectLinearPosition;
    objectAngularPosition_ = objectAngularPosition;
    objectLinearVelocity_ = objectLinearVelocity;
    objectAngularVelocity_ = objectAngularVelocity;
    objectLinearAcceleration_ = objectLinearAcceleration;
    objectAngularAcceleration_ = objectAngularAcceleration;
    return;
  }

  void LocalToGlobalConversion() {
    calculatedLinearPosition_ = localToGlobalPosition(objectLinearPosition_);
    calculatedAngularPosition_ =
        localToGlobalOrientation(objectAngularPosition_);
    calculatedLinearVelocity_ =
        localToGlobalLinearVelocity(objectLinearVelocity_);
    calculatedAngularVelocity_ =
        localToGlobalAngularVelocity(objectAngularVelocity_);
    calculatedLinearAcceleration_ =
        localToGlobalLinearAcceleration(objectLinearAcceleration_);
    calculatedAngularAcceleration_ =
        localToGlobalAngularAcceleration(objectAngularAcceleration_);
  }

  void GlobalToLocalConversion() {
    calculatedLinearPosition_ = globalToLocalPosition(objectLinearPosition_);
    calculatedAngularPosition_ =
        globalToLocalOrientation(objectAngularPosition_);
    calculatedLinearVelocity_ =
        globalToLocalLinearVelocity(objectLinearVelocity_);
    calculatedAngularVelocity_ =
        globalToLocalAngularVelocity(objectAngularVelocity_);
    calculatedLinearAcceleration_ =
        globalToLocalLinearAcceleration(objectLinearAcceleration_);
    calculatedAngularAcceleration_ =
        globalToLocalAngularAcceleration(objectAngularAcceleration_);
  }

  std::tuple<double, double, double> CalculatedLinearPosition() const {
    return std::make_tuple(calculatedLinearPosition_.x(),
                           calculatedLinearPosition_.y(),
                           calculatedLinearPosition_.z());
  };
  std::tuple<double, double, double> CalculatedAngularPosition() const {
    return std::make_tuple(calculatedAngularPosition_.x(),
                           calculatedAngularPosition_.y(),
                           calculatedAngularPosition_.z());
  };
  std::tuple<double, double, double> CalculatedLinearVelocity() const {
    return std::make_tuple(calculatedLinearVelocity_.x(),
                           calculatedLinearVelocity_.y(),
                           calculatedLinearVelocity_.z());
  };
  std::tuple<double, double, double> CalculatedAngularVelocity() const {
    return std::make_tuple(calculatedAngularVelocity_.x(),
                           calculatedAngularVelocity_.y(),
                           calculatedAngularVelocity_.z());
  };
  std::tuple<double, double, double> CalculatedLinearAcceleration() const {
    return std::make_tuple(calculatedLinearAcceleration_.x(),
                           calculatedLinearAcceleration_.y(),
                           calculatedLinearAcceleration_.z());
  };
  std::tuple<double, double, double> CalculatedAngularAcceleration() const {
    return std::make_tuple(calculatedAngularAcceleration_.x(),
                           calculatedAngularAcceleration_.y(),
                           calculatedAngularAcceleration_.z());
  };

 private:
  Eigen::Vector3d observerLinearPosition_{Eigen::Vector3d::Zero()};
  Eigen::Quaterniond observerAngularPosition_{Eigen::Quaterniond::Identity()};
  Eigen::Vector3d observerLinearVelocity_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d observerAngularVelocity_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d observerLinearAcceleration_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d observerAngularAcceleration_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d objectLinearPosition_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d objectAngularPosition_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d objectLinearVelocity_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d objectAngularVelocity_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d objectLinearAcceleration_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d objectAngularAcceleration_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d calculatedLinearPosition_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d calculatedAngularPosition_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d calculatedLinearVelocity_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d calculatedAngularVelocity_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d calculatedLinearAcceleration_{Eigen::Vector3d::Zero()};
  Eigen::Vector3d calculatedAngularAcceleration_{Eigen::Vector3d::Zero()};

  // Converts an Eigen::Quaterniond to roll, pitch, yaw angles in radian.
  Eigen::Vector3d QuaternionToEulerAngles(
      const Eigen::Quaterniond& quaternion) const {
    Eigen::Matrix3d rotation_matrix = quaternion.toRotationMatrix();
    Eigen::Vector3d euler_angles_radians =
        rotation_matrix.eulerAngles(0, 1, 2);  // roll, pitch, yaw
    // Reorder to roll, pitch, yaw
    return Eigen::Vector3d(euler_angles_radians(0), euler_angles_radians(1),
                           euler_angles_radians(2));
  }

  // Converts an Eigen::Quaterniond to intrinsic roll, pitch, yaw angles in
  // radian.
  Eigen::Vector3d quaternionToEulerAngles(const Eigen::Quaterniond& q) {
    double w = q.w();
    double x = q.x();
    double y = q.y();
    double z = q.z();

    // Calculate sin(pitch)
    double sinp = 2 * (w * z + x * y);

    // Calculate cos(pitch)
    double cosp = 1 - 2 * (y * y + z * z);

    // Check for gimbal lock (pitch near +/- 90 degrees)
    if (std::abs(cosp) < 1e-6) {  // Tolerance for numerical precision
      double roll = 0.0;
      double pitch = (sinp > 0) ? M_PI_2 : -M_PI_2;  // +/- 90 degrees
      double yaw = std::atan2(x, w);
      return Eigen::Vector3d(roll, pitch, yaw);
    } else {
      // Standard Euler angle calculation
      double yaw = std::atan2(2 * (y * z - x * w), cosp);
      double pitch = std::asin(sinp);
      double roll = std::atan2(2 * (x * z + w * y), cosp);
      return Eigen::Vector3d(roll, pitch, yaw);
    }
  }

  // -- Position Conversions --

  Eigen::Vector3d localToGlobalPosition(
      const Eigen::Vector3d& localPosition) const {
    return observerAngularPosition_ * localPosition + observerLinearPosition_;
  }

  Eigen::Vector3d globalToLocalPosition(
      const Eigen::Vector3d& globalPosition) const {
    return observerAngularPosition_.conjugate() *
           (globalPosition - observerLinearPosition_);
  }

  Eigen::Vector3d localToGlobalOrientation(
      const Eigen::Quaterniond& localOrientation) {
    return QuaternionToEulerAngles(observerAngularPosition_ * localOrientation);
  }
  Eigen::Vector3d localToGlobalOrientation(
      const Eigen::Vector3d& localOrientation) {
    Eigen::Quaterniond localOrientationQ =
        Eigen::AngleAxisd(localOrientation[2], Eigen::Vector3d::UnitZ()) *
        Eigen::AngleAxisd(localOrientation[1], Eigen::Vector3d::UnitY()) *
        Eigen::AngleAxisd(localOrientation[0], Eigen::Vector3d::UnitX());
    return QuaternionToEulerAngles(observerAngularPosition_ *
                                   localOrientationQ);
  }

  Eigen::Vector3d globalToLocalOrientation(
      const Eigen::Quaterniond& globalOrientation) {
    return QuaternionToEulerAngles(observerAngularPosition_.conjugate() *
                                   globalOrientation);
  }
  Eigen::Vector3d globalToLocalOrientation(
      const Eigen::Vector3d& globalOrientation) {
    Eigen::Quaterniond globalOrientationQ =
        Eigen::AngleAxisd(globalOrientation[2], Eigen::Vector3d::UnitZ()) *
        Eigen::AngleAxisd(globalOrientation[1], Eigen::Vector3d::UnitY()) *
        Eigen::AngleAxisd(globalOrientation[0], Eigen::Vector3d::UnitX());
    return QuaternionToEulerAngles(observerAngularPosition_.conjugate() *
                                   globalOrientationQ);
  }
  // -- Velocity Conversions --

  Eigen::Vector3d localToGlobalLinearVelocity(
      const Eigen::Vector3d& localVelocity) const {
    return observerAngularPosition_ * localVelocity + observerLinearVelocity_;
  }

  Eigen::Vector3d localToGlobalAngularVelocity(
      const Eigen::Vector3d& localAngularVelocity) const {
    return observerAngularPosition_ * localAngularVelocity +
           observerAngularVelocity_;
  }

  Eigen::Vector3d globalToLocalLinearVelocity(
      const Eigen::Vector3d& globalVelocity) const {
    return observerAngularPosition_.conjugate() *
           (globalVelocity - observerLinearVelocity_);
  }

  Eigen::Vector3d globalToLocalAngularVelocity(
      const Eigen::Vector3d& globalAngularVelocity) const {
    return observerAngularPosition_.conjugate() *
           (globalAngularVelocity - observerAngularVelocity_);
  }

  // -- Acceleration Conversions --

  Eigen::Vector3d localToGlobalLinearAcceleration(
      const Eigen::Vector3d& localAcceleration) const {
    Eigen::Vector3d coriolisAcceleration =
        2 * observerAngularVelocity_.cross(localAcceleration);
    Eigen::Vector3d centripetalAcceleration = observerAngularVelocity_.cross(
        observerAngularVelocity_.cross(localAcceleration));
    Eigen::Vector3d eulerAcceleration =
        observerAngularAcceleration_.cross(localAcceleration);

    return observerAngularPosition_ * localAcceleration +
           observerLinearAcceleration_ + coriolisAcceleration +
           centripetalAcceleration + eulerAcceleration;
  }

  Eigen::Vector3d localToGlobalAngularAcceleration(
      const Eigen::Vector3d& localAngularAcceleration) const {
    return observerAngularPosition_ * localAngularAcceleration +
           observerAngularAcceleration_;
  }

  Eigen::Vector3d globalToLocalLinearAcceleration(
      const Eigen::Vector3d& globalAcceleration) const {
    Eigen::Vector3d relativeVelocity =
        globalAcceleration - observerLinearAcceleration_;
    Eigen::Vector3d coriolisAcceleration =
        -2 * observerAngularVelocity_.cross(relativeVelocity);
    Eigen::Vector3d centripetalAcceleration = -observerAngularVelocity_.cross(
        observerAngularVelocity_.cross(relativeVelocity));
    Eigen::Vector3d eulerAcceleration =
        -observerAngularAcceleration_.cross(relativeVelocity);

    return observerAngularPosition_.conjugate() *
           (relativeVelocity + coriolisAcceleration + centripetalAcceleration +
            eulerAcceleration);
  }

  Eigen::Vector3d globalToLocalAngularAcceleration(
      const Eigen::Vector3d& globalAngularAcceleration) const {
    return observerAngularPosition_.conjugate() *
           (globalAngularAcceleration - observerAngularAcceleration_);
  }
};

}  // namespace tracker
#endif  // TRACKER_MOTION_CONVERTER_H
