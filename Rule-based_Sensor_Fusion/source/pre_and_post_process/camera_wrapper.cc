#include "pre_and_post_process/camera_wrapper.h"

namespace tracker {
CameraWrapper::CameraWrapper(const size_t bufferSize)
    : WrapperBase(bufferSize) {}

ObservationsProto CameraWrapper::CameraInputWrapper(
    const std::vector<InstanceData>& Camera_Det_Objs, const double time) {
  ObservationsProto frame;
  if (!(Camera_Det_Objs.size() == 0)) {
    if (!(Camera_Det_Objs[0].bboxes_3d.size() == 0)) {
      // Iterate over all camera objects (read-only)
      std::vector<tracker::Box3D> Observation;
      std::vector<tracker::ObservationType> types;
      std::vector<double> confs;
      std::vector<tracker::ObservationSensorType> sensorTypes;
      std::vector<uint32_t> ids;
      uint32_t counter = 0;
      std::vector<double> time_vector;
      const auto& bboxes_3d = Camera_Det_Objs[0].bboxes_3d;
      const auto& labels = Camera_Det_Objs[0].labels_3d;
      const auto& scores = Camera_Det_Objs[0].scores_3d;
      for (uint32_t j = 0; j < bboxes_3d.rows(); j++) {
        ids.push_back(counter++);
        types.push_back(Type_Map_CameraPerception2Tracker(labels[j]));
        sensorTypes.push_back(tracker::ST_CAMERA);
        confs.push_back(scores(j));
        tracker::Vec3d v(bboxes_3d(j, 0), bboxes_3d(j, 1), bboxes_3d(j, 2));
        // b: (position{x,y,z}, yaw, yawdot, length, width, height)
        tracker::Box3D b(v, bboxes_3d(j, 6) * (M_PI / 180.0), 0.0,
                         bboxes_3d(j, 5), bboxes_3d(j, 3), bboxes_3d(j, 4));
        Observation.push_back(b);
        time_vector.push_back(time);
      }
      frame = BuildObservation(Observation, ids, types, confs, sensorTypes,
                               time_vector);
    }
  }
  return frame;
}

void CameraWrapper::OutputWrapper(
    std::vector<perception::CameraPerception2Fusion>* MOT_output_objs,
    const std::vector<tracker::TrackPtr>& Track_List, const double timestamp) {
  // Important: Euler angles are in ZYX intrinsic order (Roll,Pitch,Yaw)
  // Empty the output list
  MOT_output_objs->clear();

  perception::CameraPerception2Fusion MOT_output_obj;
  for (uint16_t i = 0; i < Track_List.size(); i++) {
    if (Track_List.at(i)->track_state.lifetime < kMinimumlifetime) {
      continue;
    }

    // Coordinate change from global to local
    this->ego_vehicle_motion_.InsertObjectInfo(
        {Track_List.at(i)->track_state.estimator.x(),
         Track_List.at(i)->track_state.estimator.y(),
         Track_List.at(i)->track_state.bounding_box.center_z()},
        {0.0, 0.0, Track_List.at(i)->track_state.estimator.yaw()},
        {Track_List.at(i)->track_state.estimator.vx(),
         Track_List.at(i)->track_state.estimator.vy(), 0.0},
        {Track_List.at(i)->track_state.estimator.yawdot(), 0.0, 0.0});
    // To Do : add acceleration
    this->ego_vehicle_motion_.GlobalToLocalConversion();

    std::tie(MOT_output_obj.position.x, MOT_output_obj.position.y,
             MOT_output_obj.position.z) =
        this->ego_vehicle_motion_.CalculatedLinearPosition();
    std::tie(std::ignore, std::ignore, MOT_output_obj.theta) =
        this->ego_vehicle_motion_.CalculatedAngularPosition();
    std::tie(MOT_output_obj.velocity.x, MOT_output_obj.velocity.y,
             MOT_output_obj.velocity.z) =
        this->ego_vehicle_motion_.CalculatedLinearVelocity();
    std::tie(MOT_output_obj.acceleration.x, MOT_output_obj.acceleration.y,
             MOT_output_obj.acceleration.z) =
        this->ego_vehicle_motion_.CalculatedLinearAcceleration();

    // change the velocity to local absolute for JAC project
    std::tie(MOT_output_obj.velocity.x, MOT_output_obj.velocity.y,
             MOT_output_obj.velocity.z) = std::tuple<double, double, double>{
        Track_List.at(i)->track_state.estimator.vx(),
        Track_List.at(i)->track_state.estimator.vy(), 0};
    std::tie(MOT_output_obj.acceleration.x, MOT_output_obj.acceleration.y,
             MOT_output_obj.acceleration.z) =
        std::tuple<double, double, double>{0, 0, 0};
    rotateVector(&MOT_output_obj.velocity.x, &MOT_output_obj.velocity.y,
                 -this->selected_vehicle_odom_data_.euler_angles.z());
    MOT_output_obj.velocity.x =
        Kinematics_apply_absolute_velocity_limit(MOT_output_obj.velocity.x);
    MOT_output_obj.velocity.y =
        Kinematics_apply_absolute_velocity_limit(MOT_output_obj.velocity.y);

    MOT_output_obj.length = Track_List.at(i)->track_state.bounding_box.length();
    MOT_output_obj.width = Track_List.at(i)->track_state.bounding_box.width();
    MOT_output_obj.height = Track_List.at(i)->track_state.bounding_box.height();
    MOT_output_obj.type = Track_List.at(i)->track_state.type;
    MOT_output_obj.subtype = 0u;
    MOT_output_obj.confidence = Track_List.at(i)->track_state.conf;
    MOT_output_obj.ID = Track_List.at(i)->track_state.id;
    MOT_output_obj.timestamp = timestamp;  // in seconds
    MOT_output_obj.measurement_time =
        Track_List.at(i)->track_state.timestamp * 1e6;  // in micro second
    MOT_output_obj.position_covariance = {0.0, 0.0, 0.0};
    MOT_output_obj.velocity_covariance = {0.0, 0.0, 0.0};
    MOT_output_obj.acceleration_covariance = {0.0, 0.0, 0.0};
    MOT_output_objs->push_back(MOT_output_obj);
  }
  return;
}

ObservationType CameraWrapper::Type_Map_CameraPerception2Tracker(
    int camera_object_type) {
  ObservationType tracker_data;
  switch (camera_object_type) {
    case 0:
      tracker_data = OT_VEHICLE;
      break;
    case 1:
      tracker_data = OT_CYCLIST;
      break;
    case 2:
      tracker_data = OT_PEDESTRIAN;
      break;
    default:
      tracker_data = OT_UNKNOWN;
      break;
  }
  return tracker_data;
}

}  // namespace tracker
