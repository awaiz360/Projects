#include "pre_and_post_process/lidar_wrapper.h"

namespace tracker {
LidarWrapper::LidarWrapper(const size_t bufferSize) : WrapperBase(bufferSize) {}

ObservationsProto LidarWrapper::LidarInputWrapper(
    const std::vector<LidarPerceptionOutput>& Lidar_Det_Objs,
    const double time) {
  ObservationsProto frame;
  if (!Lidar_Det_Objs.empty()) {
    // Iterate over all lidar objects (read-only)
    std::vector<tracker::Box3D> Observation;
    std::vector<tracker::ObservationType> types;
    std::vector<double> confs;
    std::vector<tracker::ObservationSensorType> sensorTypes;
    std::vector<uint32_t> ids;
    uint32_t counter = 0;
    std::vector<double> time_vector;
    for (const LidarPerceptionOutput& obj : Lidar_Det_Objs) {
      ids.push_back(counter++);
      types.push_back(Type_Map_LidarPerception2Tracker(obj.type));
      sensorTypes.push_back(tracker::ST_LIDAR);
      confs.push_back(obj.confidence);
      tracker::Vec3d v(obj.position.x, obj.position.y, obj.position.z);
      tracker::Box3D b(v, obj.theta, 0.0, obj.length, obj.width, obj.height);
      Observation.push_back(b);
      time_vector.push_back(time);
    }
    frame = BuildObservation(Observation, ids, types, confs, sensorTypes,
                             time_vector);
  }
  return frame;
}

void LidarWrapper::OutputWrapper(
    std::vector<perception::LiDARPerception2Fusion>* MOT_output_objs,
    const std::vector<tracker::TrackPtr>& Track_List, const double timestamp) {
  // Important: Euler angles are in ZYX intrinsic order (Roll,Pitch,Yaw)
  // Empty the output list
  MOT_output_objs->clear();

  perception::LiDARPerception2Fusion MOT_output_obj;
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
    MOT_output_obj.motion_type = MotionType_Map_LidarTracker2Fusion(
        Track_List.at(i)->track_state.motion_type.getMotion());
    MOT_output_objs->push_back(MOT_output_obj);
  }
  return;
}

ObservationType LidarWrapper::Type_Map_LidarPerception2Tracker(
    std::string lidar_object_type) {
  // TODO: correct the mapping
  ObservationType tracker_data;
  tracker_data = OT_UNKNOWN;
  return tracker_data;
}

uint8_t LidarWrapper::MotionType_Map_LidarTracker2Fusion(
    MotionType lidar_object_motion_type) {
  uint8_t tracker_data;
  switch (lidar_object_motion_type) {
    case MotionType::UNKNOWN:
      tracker_data = 0u;
      break;
    case MotionType::STATIONARY:
      tracker_data = 1u;
      break;
    case MotionType::MOVING_DRIVING:
      tracker_data = 2u;
      break;
    case MotionType::MOVING_STOPPED:
      tracker_data = 3u;
      break;
    case MotionType::MOVING_REVERSE:
      tracker_data = 4u;
      break;
    case MotionType::MOVING_CROSSING:
      tracker_data = 5u;
      break;
    default:
      tracker_data = 0u;
  }
  return tracker_data;
}

}  // namespace tracker
