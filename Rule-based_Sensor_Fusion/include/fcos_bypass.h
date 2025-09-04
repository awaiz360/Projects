#ifndef TRACKER_FCOS_BYPASS_H
#define TRACKER_FCOS_BYPASS_H

#include "external_types.h"
#include "types.h"

namespace tracker {

ObservationType Type_Map_CameraPerception2Tracker(int camera_object_type) {
  ObservationType tracker_data;
  switch (camera_object_type) {
    case 0:
      tracker_data = ObservationType::OT_VEHICLE;
      break;
    case 1:
      tracker_data = ObservationType::OT_CYCLIST;
      break;
    case 2:
      tracker_data = ObservationType::OT_PEDESTRIAN;
      break;
    default:
      tracker_data = ObservationType::OT_UNKNOWN;
      break;
  }
  return tracker_data;
}

perception::vector_CameraPerception2Fusion
bypass_camera_perception_data_to_tracker_output(
    const std::vector<InstanceData>& camera_perception_data,
    const double timestamp) {
  ////// Bypass the camera tracker ouput
  perception::vector_CameraPerception2Fusion outPutData;
  perception::CameraPerception2Fusion tmpObject;
  const auto& bboxes_3d = camera_perception_data[0].bboxes_3d;
  const auto& labels = camera_perception_data[0].labels_3d;
  const auto& scores = camera_perception_data[0].scores_3d;
  for (int i = 0; i < bboxes_3d.rows(); i++) {
    tmpObject.ID = i + 1;
    tmpObject.type = Type_Map_CameraPerception2Tracker(labels[j]);
    tmpObject.position.x = bboxes_3d(i, 0);
    tmpObject.position.y = bboxes_3d(i, 1);
    tmpObject.position.z = bboxes_3d(i, 2);
    tmpObject.theta = bboxes_3d(i, 6);
    tmpObject.timestamp = timestamp;
    tmpObject.measurement_time = timestamp * 1e6;  // in micro seconds
    tmpObject.length = bboxes_3d(i, 5);
    tmpObject.width = bboxes_3d(i, 3);
    tmpObject.height = bboxes_3d(i, 4);
    tmpObject.confidence = scores(j);
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
#endif  // TRACKER_FCOS_BYPASS_H
