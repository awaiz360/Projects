#pragma once
#include <inttypes.h>

#include <Eigen/Dense>
#include <optional>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tracker {

enum TrackerEngineType : int {
  TET_UNKNOWN = 0,
  TET_CAMERA = 1,
  TET_LIDAR = 2,
  TET_RADAR = 3,
  TET_FUSION = 4,
};

enum ObservationType : int  // linked with NUMBER_OF_ITEMS_IN_OBSERVATION_TYPE
{
  OT_UNKNOWN = 0,
  OT_VEHICLE = 1,
  OT_TRUCK = 2,
  OT_CYCLIST = 3,
  OT_PEDESTRIAN = 4,
};

enum ObservationSensorType : int {
  // onehot method
  ST_UNKNOWN = 0,
  ST_LIDAR = 1 << 0,
  ST_CAMERA = 1 << 1,
  ST_RADAR = 1 << 2,
  ST_LIDAR_CAMERA = (ST_LIDAR | ST_CAMERA),
  ST_LIDAR_RADAR = (ST_LIDAR | ST_RADAR),
  ST_CAMERA_RADAR = (ST_CAMERA | ST_RADAR),
  ST_LIDAR_CAMERA_RADAR = (ST_LIDAR | ST_CAMERA | ST_RADAR),
};

enum TrackLifeState : int {
  TLS_IDLE = 0,
  TLS_INIT = 1,
  TLS_CONFIRMED = 2,
  TLS_LOST = 3,
};

enum MotionStateType : int {
  MST_UNKNOWN = 0,
  MST_STATIONARY = 1,
  MST_MOVING = 2,
};

enum class MotionType {
  UNKNOWN,
  STATIONARY,
  MOVING_DRIVING,
  MOVING_STOPPED,
  MOVING_REVERSE,
  MOVING_CROSSING
};

class Mat2d {
 public:
  Mat2d() = default;  // Default constructor

  // Getter methods
  double v0() const { return v0_; }
  double v1() const { return v1_; }
  double v2() const { return v2_; }
  double v3() const { return v3_; }

  // Setter methods
  void set_v0(double v0) { v0_ = v0; }
  void set_v1(double v1) { v1_ = v1; }
  void set_v2(double v2) { v2_ = v2; }
  void set_v3(double v3) { v3_ = v3; }

 private:
  double v0_;
  double v1_;
  double v2_;
  double v3_;
};

template <typename T>
class Optional {
 public:
  Optional() : has_value_(false) {}

  Optional(const T &value) : has_value_(true), value_(value) {}

  Optional(T &&value) : has_value_(true), value_(std::move(value)) {}

  bool has_value() const { return has_value_; }
  T value() const { return value_; }

 private:
  bool has_value_;
  T value_;
};

class LidarObservationProto {
 public:
  LidarObservationProto() = default;
  double center_x() const { return center_x_; }
  double center_y() const { return center_y_; }
  double center_z() const { return center_z_; }
  double timestamp() const { return timestamp_; }
  double length() const { return length_; }
  double width() const { return width_; }
  double height() const { return height_; }
  double yaw() const { return yaw_; }
  ObservationType type() const { return type_; }
  double conf() const { return conf_; }

  void set_timestamp(double timestamp) { timestamp_ = timestamp; }
  void set_center_x(double center_x) { center_x_ = center_x; }
  void set_center_y(double center_y) { center_y_ = center_y; }
  void set_center_z(double center_z) { center_z_ = center_z; }
  void set_length(double length) { length_ = length; }
  void set_width(double width) { width_ = width; }
  void set_height(double height) { height_ = height; }
  void set_yaw(double yaw) { yaw_ = yaw; }
  void set_type(ObservationType type) { type_ = type; }
  void set_conf(double conf) { conf_ = conf; }

 private:
  double timestamp_;
  double center_x_;
  double center_y_;
  double center_z_;
  double length_;
  double width_;
  double height_;
  double yaw_;
  ObservationType type_;
  double conf_;
};

// TODO(somebody): in fusion, considering add velocity fields
class CameraObservationProto {
 public:
  CameraObservationProto() = default;
  double center_x() const { return center_x_; }
  double center_y() const { return center_y_; }
  double center_z() const { return center_z_; }
  double timestamp() const { return timestamp_; }
  double length() const { return length_; }
  double width() const { return width_; }
  double height() const { return height_; }
  double yaw() const { return yaw_; }
  ObservationType type() const { return type_; }
  double conf() const { return conf_; }

  void set_timestamp(double timestamp) { timestamp_ = timestamp; }
  void set_center_x(double center_x) { center_x_ = center_x; }
  void set_center_y(double center_y) { center_y_ = center_y; }
  void set_center_z(double center_z) { center_z_ = center_z; }
  void set_length(double length) { length_ = length; }
  void set_width(double width) { width_ = width; }
  void set_height(double height) { height_ = height; }
  void set_yaw(double yaw) { yaw_ = yaw; }
  void set_type(ObservationType type) { type_ = type; }
  void set_conf(double conf) { conf_ = conf; }

 private:
  double timestamp_;
  double center_x_;
  double center_y_;
  double center_z_;
  double length_;
  double width_;
  double height_;
  double yaw_;
  ObservationType type_;
  double conf_;
};

class RadarObservationProto {
 public:
  RadarObservationProto() = default;  // Default constructor

  // Getter methods
  double timestamp() const { return timestamp_; }
  double length() const { return length_; }
  double width() const { return width_; }
  double yaw() const { return yaw_; }
  double position_x() const { return position_x_; }
  double position_y() const { return position_y_; }
  const Mat2d &pos_cov() const { return pos_cov_; }
  double velocity_x() const { return velocity_x_; }
  double velocity_y() const { return velocity_y_; }
  const Mat2d &vel_cov() const { return vel_cov_; }
  double acceleration_x() const { return acceleration_x_; }
  double acceleration_y() const { return acceleration_y_; }
  const Mat2d &acc_cov() const { return acc_cov_; }
  MotionStateType motion_state() const { return motion_state_; }
  double obj_rcs() const { return obj_rcs_; }
  int32_t id() const { return id_; }
  ObservationType type() const { return type_; }

  // Setter methods
  void set_timestamp(double timestamp) { timestamp_ = timestamp; }
  void set_length(double length) { length_ = length; }
  void set_width(double width) { width_ = width; }
  void set_yaw(double yaw) { yaw_ = yaw; }
  void set_position_x(double position_x) { position_x_ = position_x; }
  void set_position_y(double position_y) { position_y_ = position_y; }
  void set_pos_cov(const Mat2d &pos_cov) { pos_cov_ = pos_cov; }
  void set_velocity_x(double velocity_x) { velocity_x_ = velocity_x; }
  void set_velocity_y(double velocity_y) { velocity_y_ = velocity_y; }
  void set_vel_cov(const Mat2d &vel_cov) { vel_cov_ = vel_cov; }
  void set_acceleration_x(double acceleration_x) {
    acceleration_x_ = acceleration_x;
  }
  void set_acceleration_y(double acceleration_y) {
    acceleration_y_ = acceleration_y;
  }
  void set_acc_cov(const Mat2d &acc_cov) { acc_cov_ = acc_cov; }
  void set_motion_state(MotionStateType motion_state) {
    motion_state_ = motion_state;
  }
  void set_obj_rcs(double obj_rcs) { obj_rcs_ = obj_rcs; }
  void set_id(int32_t id) { id_ = id; }
  void set_type(ObservationType type) { type_ = type; }

 private:
  double timestamp_;
  double length_;
  double width_;
  double yaw_;
  double position_x_;
  double position_y_;
  Mat2d pos_cov_;
  double velocity_x_;
  double velocity_y_;
  Mat2d vel_cov_;
  double acceleration_x_;
  double acceleration_y_;
  Mat2d acc_cov_;
  MotionStateType motion_state_;
  double obj_rcs_;
  int32_t id_;
  ObservationType type_;
};

class ObservationProto {
 public:
  ObservationProto() = default;

  double timestamp() const { return timestamp_; }
  void set_timestamp(double timestamp) { timestamp_ = timestamp; }
  void set_id(uint32_t id) { id_ = id; }
  void set_type(ObservationType type) { type_ = type; }
  void set_conf(double conf) { conf_ = conf; }
  void set_sensorType(ObservationSensorType SensorType) {
    sensorType_ = SensorType;
  }

  const ObservationSensorType get_sensorType() const { return sensorType_; }

  bool has_lidar_observation() const { return lidar_observation_.has_value(); }

  const LidarObservationProto &lidar_observation() const {
    return lidar_observation_.value();
  }
  LidarObservationProto *mutable_lidar_observation() {
    if (!lidar_observation_.has_value()) {
      lidar_observation_ = std::make_optional<LidarObservationProto>();
    }
    return &lidar_observation_.value();
  }

  bool has_camera_observation() const {
    return camera_observation_.has_value();
  }

  const CameraObservationProto &camera_observation() const {
    return camera_observation_.value();
  }
  CameraObservationProto *mutable_camera_observation() {
    if (!camera_observation_.has_value()) {
      camera_observation_ = std::make_optional<CameraObservationProto>();
    }
    return &(camera_observation_.value());
  }

  bool has_radar_observation() const { return radar_observation_.has_value(); }

  const RadarObservationProto &radar_observation() const {
    return radar_observation_.value();
  }
  RadarObservationProto *mutable_radar_observation() {
    if (!radar_observation_.has_value()) {
      radar_observation_ = std::make_optional<RadarObservationProto>();
    }
    return &(radar_observation_.value());
  }

 private:
  double timestamp_;
  uint32_t id_;
  ObservationType type_;
  double conf_;
  ObservationSensorType sensorType_;
  std::optional<LidarObservationProto> lidar_observation_;
  std::optional<CameraObservationProto> camera_observation_;
  std::optional<RadarObservationProto> radar_observation_;
};

class ObservationsProto {
 public:
  ObservationsProto() = default;
  int observations_size() const { return observations_.size(); }
  const std::vector<ObservationProto> &observations() const {
    return observations_;
  }
  ObservationProto *mutable_observation(uint32_t index) {
    if (index >= observations_.size()) return nullptr;
    return &observations_[index];
  }
  double min_timestamp() const { return min_timestamp_.value(); };
  bool has_min_timestamp() const { return min_timestamp_.has_value(); }
  void set_min_timestamp(double min_timestamp) {
    min_timestamp_ = min_timestamp;
  }
  ObservationProto *add_observation() {
    ObservationProto obs;
    observations_.push_back(obs);
    return &observations_[observations_.size() - 1];
  }

 private:
  std::vector<ObservationProto> observations_;
  Optional<double> min_timestamp_;
};

typedef Eigen::Matrix<double, 5, 1> Vector5d;
typedef Eigen::Matrix<double, 4, 1> Vector4d;
typedef Eigen::Matrix<double, 3, 1> Vector3d;
typedef Eigen::Matrix<double, 2, 1> Vector2d;
typedef Eigen::Matrix<double, 5, 5> Matrix5d;
typedef Eigen::Matrix<double, 4, 4> Matrix4d;
typedef Eigen::Matrix<double, 3, 3> Matrix3d;
typedef Eigen::Matrix<double, 2, 2> Matrix2d;
typedef Eigen::Matrix<double, 2, 5> Matrix25d;
typedef Eigen::Matrix<double, 3, 5> Matrix35d;
typedef Eigen::Matrix<double, 4, 5> Matrix45d;
typedef Eigen::Matrix<double, 5, 2> Matrix52d;
typedef Eigen::Matrix<double, 5, 3> Matrix53d;
typedef Eigen::Matrix<double, 5, 4> Matrix54d;

}  // namespace tracker
