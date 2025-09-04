#ifndef TRACKER_CAMERA_LIDAR_INPUT_TYPE_H_
#define TRACKER_CAMERA_LIDAR_INPUT_TYPE_H_

#include <Eigen/Dense>
#include <cstdint>  // For fixed-width integer types like uint8_t, uint32_t, uint64_t
#include <string>  // For std::string, in case we need to include a model name
#include <vector>  // For std::vector

#ifdef Wrapper_Operation
#include "dpc/impl_type_slamlocalegoodom.h"
// #include "dpc/impl_type_slam_odometry.h"
#include "dpc/impl_type_trafficparticipantstruct.h"
#include "perception/impl_type_vector_cameraperception2fusion.h"
#include "perception/impl_type_vector_lidarperception2fusion.h"
// Change the following accordingly
// #include "utils.h" // internal camera perception header --> InstanceData
// #include "lidar_processing.h" // internal lidar perception header --> LidarPerceptionOutput

#endif  // Wrapper_Operation

#ifndef Wrapper_Operation
namespace perception {

struct Point3D {
  double x{0.0};
  double y{0.0};
  double z{0.0};
};

struct CameraPerception2Fusion {
  Point3D position;
  double theta{0.0};
  double length{0.0};
  double width{0.0};
  double height{0.0};
  Point3D velocity;
  uint8_t type{0U};
  uint8_t subtype{0U};
  double confidence{0.0};
  uint32_t ID{0U};
  double timestamp{0.0};
  uint64_t measurement_time{0U};
  Point3D acceleration;
  std::vector<float> position_covariance;
  std::vector<float> velocity_covariance;
  std::vector<float> acceleration_covariance;
};

using vector_CameraPerception2Fusion = std::vector<CameraPerception2Fusion>;

struct LiDARPerception2Fusion {
  Point3D position;
  double theta{0.0};
  double length{0.0};
  double width{0.0};
  double height{0.0};
  Point3D velocity;
  uint8_t type{0U};
  uint8_t subtype{0U};
  double confidence{0.0};
  uint32_t ID{0U};
  double timestamp{0.0};
  uint64_t measurement_time{0U};
  Point3D acceleration;
  std::vector<float> position_covariance;
  std::vector<float> velocity_covariance;
  std::vector<float> acceleration_covariance;
  uint8_t motion_type{0U};
};

using vector_LiDARPerception2Fusion =
    std::vector<perception::LiDARPerception2Fusion>;

}  // namespace perception

namespace dpc {

struct Vector3f {
  float x{0.0F};
  float y{0.0F};
  float z{0.0F};
};

struct SLAMLocalEgoOdom {
  double ego_odom_time_ns{0.0};
  std::string status_inf;
  std::string debug_inf;
  uint8_t origin_id{0U};
  float roll{0.0F};
  float pitch{0.0F};
  float yaw{0.0F};
  Vector3f position;
  Vector3f position_std_dev;
  Vector3f attitude_std_dev;
  Vector3f linear_velocity;
  Vector3f linear_acceleration;
  Vector3f angular_velocity;
};

struct ObjectStruct {
  uint32_t id{0U};
  uint8_t type{0U};
  float relative_x{0.0F};
  float relative_y{0.0F};
  float relative_orientation{0.0F};
  float absolute_velocity_x{0.0F};
  float absolute_velocity_y{0.0F};
  float absolute_acceleration_x{0.0F};
  float absolute_acceleration_y{0.0F};
  float width{0.0F};
  float height{0.0F};
  float length{0.0F};
  uint8_t motion_type{0U};
  uint8_t contributing_sensor{0U};
};

using vector_ObjectStruct = std::vector<dpc::ObjectStruct>;

struct TrafficParticipantStruct {
  int16_t number_of_objects{0};
  vector_ObjectStruct object_list;
};

}  // namespace dpc

// Struct definition for LidarPerceptionOutput
struct BBox2D {
  double xmin;
  double ymin;
  double xmax;
  double ymax;
};
struct BBox3D {
  double minx, miny, minz, maxx, maxy, maxz, yaw;
};
struct Point3D {
  double x{0.0};
  double y{0.0};
  double z{0.0};
};
struct LidarPerceptionOutput {
  BBox2D bbox;
  Point3D position;
  BBox3D bbox3d;
  double theta;
  double length;
  double width;
  double height;
  std::vector<Point3D> polygon_point;  // 3D points for polygon
  std::string type;
  std::string subtype;
  // a vector of Point3D representing a point cloud:
  std::vector<Point3D> point_cloud;
  double confidence;
};

// Struct definition for CameraPerceptionOutput
class InstanceData {
 public:
  Eigen::MatrixXd bboxes_3d;
  Eigen::MatrixXd bboxes_2d;
  Eigen::MatrixXd scores_3d;
  Eigen::VectorXi labels_3d;
};

#endif  // Wrapper_Operation

#endif  // TRACKER_CAMERA_LIDAR_INPUT_TYPE_H_
