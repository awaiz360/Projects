#ifndef TRACKER_FUSION_BYPASS_H
#define TRACKER_FUSION_BYPASS_H

#include "external_types.h"
namespace tracker {
dpc::TrafficParticipantStruct bypass_lidar_data_to_fusion_output(
    perception::vector_LiDARPerception2Fusion lidadarObjectFromWrapper) {
  ////// Bypass the Fusion ouput
  dpc::TrafficParticipantStruct outPutData;
  dpc::ObjectStruct tmpObject;
  outPutData.number_of_objects = lidadarObjectFromWrapper.size();
  for (int i = 0; i < lidadarObjectFromWrapper.size(); i++) {
    tmpObject.id = lidadarObjectFromWrapper[i].ID;
    tmpObject.type = 0u;
    tmpObject.relative_x = lidadarObjectFromWrapper[i].position.x;
    tmpObject.relative_y = lidadarObjectFromWrapper[i].position.y;
    tmpObject.relative_orientation = lidadarObjectFromWrapper[i].theta;
    tmpObject.absolute_velocity_x = lidadarObjectFromWrapper[i].velocity.x;
    tmpObject.absolute_velocity_y = lidadarObjectFromWrapper[i].velocity.y;
    tmpObject.absolute_acceleration_x = 0;
    tmpObject.absolute_acceleration_y = 0;
    tmpObject.length = lidadarObjectFromWrapper[i].length;
    tmpObject.width = lidadarObjectFromWrapper[i].width;
    tmpObject.height = 0;
    tmpObject.motion_type = 0u;
    tmpObject.contributing_sensor = (1 << 2);
    outPutData.object_list.push_back(tmpObject);
  }
  return outPutData;
};
}  // namespace tracker
#endif  // TRACKER_FUSION_BYPASS_H
