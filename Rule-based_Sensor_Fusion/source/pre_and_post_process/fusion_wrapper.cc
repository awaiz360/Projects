#include "pre_and_post_process/fusion_wrapper.h"

namespace tracker {
FusionWrapper::FusionWrapper(const size_t bufferSize)
    : WrapperBase(bufferSize) {}

ObservationsProto FusionWrapper::LidarInputWrapper(
    const perception::vector_LiDARPerception2Fusion& LidarResults,
    double CurrentTimestamp) {
  ObservationsProto frame;
  if (!LidarResults.empty()) {
    // Iterate over all lidar objects (read-only)
    std::vector<tracker::Box3D> Observation;
    std::vector<tracker::ObservationType> types;
    std::vector<double> confs;
    std::vector<tracker::ObservationSensorType> sensorTypes;
    std::vector<uint32_t> ids;
    uint32_t counter = 0;
    std::vector<double> time_vector;
    for (const perception::LiDARPerception2Fusion& obj : LidarResults) {
      ids.push_back(counter++);
      types.push_back(static_cast<ObservationType>(obj.type));
      sensorTypes.push_back(tracker::ST_LIDAR);
      confs.push_back(obj.confidence);
      tracker::Vec3d v(obj.position.x, obj.position.y, obj.position.z);
      tracker::Box3D b(v, obj.theta, 0.0, obj.length, obj.width, obj.height);
      Observation.push_back(b);
      time_vector.push_back(obj.timestamp);
    }
    frame = BuildObservation(Observation, ids, types, confs, sensorTypes,
                             time_vector);
  }
  return frame;
}

ObservationsProto FusionWrapper::CameraInputWrapper(
    const perception::vector_CameraPerception2Fusion& FcosResults,
    double CurrentTimestamp) {
  ObservationsProto frame;
  if (!FcosResults.empty()) {
    // Iterate over all camera objects (read-only)
    std::vector<tracker::Box3D> Observation;
    std::vector<tracker::ObservationType> types;
    std::vector<double> confs;
    std::vector<tracker::ObservationSensorType> sensorTypes;
    std::vector<uint32_t> ids;
    uint32_t counter = 0;
    std::vector<double> time_vector;
    for (const perception::CameraPerception2Fusion& obj : FcosResults) {
      ids.push_back(counter++);
      types.push_back(static_cast<ObservationType>(obj.type));
      sensorTypes.push_back(tracker::ST_CAMERA);
      confs.push_back(obj.confidence);
      tracker::Vec3d v(obj.position.x, obj.position.y, obj.position.z);
      tracker::Box3D b(v, obj.theta, 0.0, obj.length, obj.width, obj.height);
      Observation.push_back(b);
      time_vector.push_back(obj.timestamp);
    }
    frame = BuildObservation(Observation, ids, types, confs, sensorTypes,
                             time_vector);
  }
  return frame;
}

void FusionWrapper::OutputWrapper(
    dpc::TrafficParticipantStruct* MOT_output_objs,
    const std::vector<tracker::TrackPtr>& Track_List, const double timestamp) {
  // Important: Euler angles are in ZYX intrinsic order (Roll,Pitch,Yaw)
  // Empty the output list
  MOT_output_objs->number_of_objects = 0;
  MOT_output_objs->object_list.clear();

  dpc::ObjectStruct MOT_output_obj;
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

    std::tie(MOT_output_obj.relative_x, MOT_output_obj.relative_y,
             std::ignore) =
        this->ego_vehicle_motion_.CalculatedLinearPosition();
    std::tie(std::ignore, std::ignore, MOT_output_obj.relative_orientation) =
        this->ego_vehicle_motion_.CalculatedAngularPosition();
    std::tie(MOT_output_obj.absolute_velocity_x,
             MOT_output_obj.absolute_velocity_y, std::ignore) =
        this->ego_vehicle_motion_.CalculatedLinearVelocity();
    std::tie(MOT_output_obj.absolute_acceleration_x,
             MOT_output_obj.absolute_acceleration_y, std::ignore) =
        this->ego_vehicle_motion_.CalculatedLinearAcceleration();

    // change the velocity to local absolute for JAC project
    std::tie(MOT_output_obj.absolute_velocity_x,
             MOT_output_obj.absolute_velocity_y, std::ignore) =
        std::tuple<double, double, double>{
            Track_List.at(i)->track_state.estimator.vx(),
            Track_List.at(i)->track_state.estimator.vy(), 0};
    std::tie(MOT_output_obj.absolute_acceleration_x,
             MOT_output_obj.absolute_acceleration_y, std::ignore) =
        std::tuple<double, double, double>{0, 0, 0};
    rotateVector(&MOT_output_obj.absolute_velocity_x,
                 &MOT_output_obj.absolute_velocity_y,
                 -this->selected_vehicle_odom_data_.euler_angles.z());
    MOT_output_obj.absolute_velocity_x =
        Kinematics_apply_absolute_velocity_limit(
            MOT_output_obj.absolute_velocity_x);
    MOT_output_obj.absolute_velocity_y =
        Kinematics_apply_absolute_velocity_limit(
            MOT_output_obj.absolute_velocity_y);

    MOT_output_obj.length = Track_List.at(i)->track_state.bounding_box.length();
    MOT_output_obj.width = Track_List.at(i)->track_state.bounding_box.width();
    MOT_output_obj.height = Track_List.at(i)->track_state.bounding_box.height();
    MOT_output_obj.type =
        this->Type_Map_Fusion2EM(Track_List.at(i)->track_state.type);
    MOT_output_obj.contributing_sensor =
        this->contributing_sensor_map_Fusion2EM(
            Track_List.at(i)->track_state.sensor_type);
    MOT_output_obj.id = Track_List.at(i)->track_state.id;
    MOT_output_obj.motion_type = this->motion_type_Map_Fusion2EM(
        Track_List.at(i)->track_state.motion_type.getMotion());
    MOT_output_objs->object_list.push_back(MOT_output_obj);
  }
  MOT_output_objs->number_of_objects = MOT_output_objs->object_list.size();
  return;
}

uint8_t FusionWrapper::Type_Map_Fusion2EM(uint8_t fusion_type) {
  // output format:
  /*0=unknown,
        1 = car,
        2 = truck,
        3 = bicyle,
        4 = ped,
        5 = other*/
  uint8_t EM_data;
  switch (fusion_type) {
    case OT_UNKNOWN:
      EM_data = 0u;
      break;
    case OT_VEHICLE:
      EM_data = 1u;
      break;
    case OT_TRUCK:
      EM_data = 2u;
      break;
    case OT_CYCLIST:
      EM_data = 3u;
      break;
    case OT_PEDESTRIAN:
      EM_data = 4u;
      break;
    default:
      EM_data = 0u;
  }
  return EM_data;
}

uint8_t FusionWrapper::motion_type_Map_Fusion2EM(
    MotionType fusion_moving_state) {
  // output format:
  /*0=unknown,
        1 = stationary,
        2 = stopped,
        3 = moving*/
  uint8_t EM_data;
  switch (fusion_moving_state) {
    case MotionType::UNKNOWN:
      EM_data = 0u;
      break;
    case MotionType::STATIONARY:
      EM_data = 1u;
      break;
    case MotionType::MOVING_DRIVING:
      EM_data = 3u;
      break;
    case MotionType::MOVING_STOPPED:
      EM_data = 2u;
      break;
    case MotionType::MOVING_REVERSE:
      EM_data = 3u;
      break;
    case MotionType::MOVING_CROSSING:
      EM_data = 3u;
      break;
    default:
      EM_data = 0u;
  }
  return EM_data;
}

uint8_t FusionWrapper::contributing_sensor_map_Fusion2EM(
    int fusion_sensor_type) {
  // output format:
  /*bit 0 = camera,
        1 = radar,
        2 = lidar*/
  uint8_t EM_data;
  switch (fusion_sensor_type) {
    case ST_UNKNOWN:
      EM_data = 0u;
      break;
    case ST_LIDAR:
      EM_data = 1 << 2;
      break;
    case ST_CAMERA:
      EM_data = 1 << 0;
      break;
    default:
      EM_data = 0u;
  }
  return EM_data;
}

}  // namespace tracker
