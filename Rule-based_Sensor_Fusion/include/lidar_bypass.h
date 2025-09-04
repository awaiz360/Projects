#ifndef TRACKER_LIDAR_BYPASS_H
#define TRACKER_LIDAR_BYPASS_H

#include "external_types.h"
namespace tracker {
std::vector<perception::LiDARPerception2Fusion>
bypass_lidar_perception_data_to_tracker_output(
    const std::vector<LidarPerceptionOutput>& lidar_perception_data,
    const double timestamp) {
  ////// Bypass the lidar tracker ouput
  std::vector<perception::LiDARPerception2Fusion> outPutData;
  perception::LiDARPerception2Fusion tmpObject;
  for (int i = 0; i < lidar_perception_data.size(); i++) {
    tmpObject.ID = i + 1;
    tmpObject.type = 0u;  // Unknown
    tmpObject.position.x = lidar_perception_data[i].position.x;
    tmpObject.position.y = lidar_perception_data[i].position.y;
    tmpObject.position.z = lidar_perception_data[i].position.z;
    tmpObject.theta = lidar_perception_data[i].theta;
    tmpObject.timestamp = timestamp;
    tmpObject.measurement_time = timestamp * 1e6;  // in micro seconds
    tmpObject.length = lidar_perception_data[i].length;
    tmpObject.width = lidar_perception_data[i].width;
    tmpObject.height = lidar_perception_data[i].height;
    tmpObject.confidence = lidar_perception_data[i].confidence;
    tmpObject.motion_type = 0;  // Unknown
    tmpObject.position_covariance = {0, 0, 0};
    tmpObject.velocity = {0, 0, 0};
    tmpObject.velocity_covariance = {0, 0, 0};
    tmpObject.acceleration = {0, 0, 0};
    tmpObject.acceleration_covariance = {0, 0, 0};
    outPutData.push_back(tmpObject);
  }
  return outPutData;
};
}  // namespace tracker
#endif  // TRACKER_LIDAR_BYPASS_H
