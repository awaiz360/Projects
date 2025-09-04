#include "pre_and_post_process/wrapper_base.h"

namespace tracker {

WrapperBase::WrapperBase(const size_t bufferSize) : buffer_{bufferSize} {}

void WrapperBase::MiddleWrapper(ObservationsProto* Lidar_frame,
                                ObservationsProto* Camera_frame,
                                const dpc::SLAMLocalEgoOdom& vehicle_odom_vec,
                                const double time) {
  this->EgoWrapper(vehicle_odom_vec);
  this->buffer_.addData(this->selected_vehicle_odom_data_);
  this->buffer_.getInterpolatedData(time, &this->selected_vehicle_odom_data_);
  this->ego_vehicle_motion_.InsertObserverInfo(
      selected_vehicle_odom_data_.position,
      selected_vehicle_odom_data_.euler_angles,
      selected_vehicle_odom_data_.linear_velocity,
      selected_vehicle_odom_data_.angular_velocity,
      selected_vehicle_odom_data_.linear_acceleration,
      selected_vehicle_odom_data_.angular_acceleration);

  // coordinate change from local to global for Lidar
  // Check if the pointer is valid (not null) before dereferencing.
  if (Lidar_frame != nullptr) {
    for (uint32_t index = 0; index < Lidar_frame->observations_size();
         index++) {
      auto Det_Obj = Lidar_frame->mutable_observation(index);
      if (Det_Obj->has_lidar_observation()) {
        this->ego_vehicle_motion_.InsertObjectInfo(
            {Det_Obj->lidar_observation().center_x(),
             Det_Obj->lidar_observation().center_y(),
             Det_Obj->lidar_observation().center_z()},
            {0.0, 0.0, Det_Obj->lidar_observation().yaw()});
        this->ego_vehicle_motion_.LocalToGlobalConversion();
        auto linear_pos = this->ego_vehicle_motion_.CalculatedLinearPosition();
        auto angular_pos =
            this->ego_vehicle_motion_.CalculatedAngularPosition();
        Det_Obj->mutable_lidar_observation()->set_center_x(
            std::get<0>(linear_pos));
        Det_Obj->mutable_lidar_observation()->set_center_y(
            std::get<1>(linear_pos));
        Det_Obj->mutable_lidar_observation()->set_center_z(
            std::get<2>(linear_pos));
        Det_Obj->mutable_lidar_observation()->set_yaw(std::get<2>(angular_pos));
        // To Do : Add velocity and acceleration to the input bounding boxes
      }
    }
  }

  // coordinate change from local to global for Camera
  // Check if the pointer is valid (not null) before dereferencing.
  if (Camera_frame != nullptr) {
    for (uint32_t index = 0; index < Camera_frame->observations_size();
         index++) {
      auto Det_Obj = Camera_frame->mutable_observation(index);
      if (Det_Obj->has_camera_observation()) {
        this->ego_vehicle_motion_.InsertObjectInfo(
            {Det_Obj->camera_observation().center_x(),
             Det_Obj->camera_observation().center_y(),
             Det_Obj->camera_observation().center_z()},
            {0.0, 0.0, Det_Obj->camera_observation().yaw()});
        this->ego_vehicle_motion_.LocalToGlobalConversion();
        auto linear_pos = this->ego_vehicle_motion_.CalculatedLinearPosition();
        auto angular_pos =
            this->ego_vehicle_motion_.CalculatedAngularPosition();
        Det_Obj->mutable_camera_observation()->set_center_x(
            std::get<0>(linear_pos));
        Det_Obj->mutable_camera_observation()->set_center_y(
            std::get<1>(linear_pos));
        Det_Obj->mutable_camera_observation()->set_center_z(
            std::get<2>(linear_pos));
        Det_Obj->mutable_camera_observation()->set_yaw(
            std::get<2>(angular_pos));
        // To Do : Add velocity and acceleration to the input bounding boxes
      }
    }
  }

  return;
}

void WrapperBase::EgoWrapper(const dpc::SLAMLocalEgoOdom& vehicle_odom_vec) {
  this->selected_vehicle_odom_data_.ego_odom_time_s =
      vehicle_odom_vec.ego_odom_time_ns / 1e9;
  this->selected_vehicle_odom_data_.status_inf = vehicle_odom_vec.status_inf;
  this->selected_vehicle_odom_data_.origin_id = vehicle_odom_vec.origin_id;

  this->selected_vehicle_odom_data_.euler_angles.x() = vehicle_odom_vec.roll;
  this->selected_vehicle_odom_data_.euler_angles.y() = vehicle_odom_vec.pitch;
  // for JACK_L4 Project
  this->selected_vehicle_odom_data_.euler_angles.z() = vehicle_odom_vec.yaw;
  this->selected_vehicle_odom_data_.angular_velocity.x() =
      vehicle_odom_vec.angular_velocity.x;
  this->selected_vehicle_odom_data_.angular_velocity.y() =
      vehicle_odom_vec.angular_velocity.y;
  this->selected_vehicle_odom_data_.angular_velocity.z() =
      vehicle_odom_vec.angular_velocity.z;
  this->selected_vehicle_odom_data_.angular_acceleration.x() = 0.0;
  this->selected_vehicle_odom_data_.angular_acceleration.y() = 0.0;
  this->selected_vehicle_odom_data_.angular_acceleration.z() = 0.0;
  this->selected_vehicle_odom_data_.position.x() = vehicle_odom_vec.position.x;
  this->selected_vehicle_odom_data_.position.y() = vehicle_odom_vec.position.y;
  this->selected_vehicle_odom_data_.position.z() = vehicle_odom_vec.position.z;
  this->selected_vehicle_odom_data_.linear_velocity.x() =
      vehicle_odom_vec.linear_velocity.x;
  this->selected_vehicle_odom_data_.linear_velocity.y() =
      vehicle_odom_vec.linear_velocity.y;
  this->selected_vehicle_odom_data_.linear_velocity.z() =
      vehicle_odom_vec.linear_velocity.z;
  this->selected_vehicle_odom_data_.linear_acceleration.x() =
      vehicle_odom_vec.linear_acceleration.x;
  this->selected_vehicle_odom_data_.linear_acceleration.y() =
      vehicle_odom_vec.linear_acceleration.y;
  this->selected_vehicle_odom_data_.linear_acceleration.z() =
      vehicle_odom_vec.linear_acceleration.z;
  return;
}

ObservationsProto WrapperBase::BuildObservation(
    const std::vector<tracker::Box3D>& boxes, const std::vector<uint32_t>& ids,
    const std::vector<tracker::ObservationType> types,
    const std::vector<double> confs,
    const std::vector<tracker::ObservationSensorType> sensorTypes,
    const std::vector<double> timestamp) {
  tracker::ObservationsProto observation;
  observation.set_min_timestamp(findMin(timestamp));
  for (int i = 0; i < boxes.size(); ++i) {
    auto detection = observation.add_observation();
    detection->set_timestamp(timestamp[i]);
    detection->set_id(ids[i]);
    detection->set_type(types[i]);
    detection->set_conf(confs[i]);
    detection->set_sensorType(sensorTypes[i]);
    if ((sensorTypes[i] & ST_LIDAR) == ST_LIDAR) {
      auto lidar_observation = detection->mutable_lidar_observation();
      lidar_observation->set_timestamp(timestamp[i]);
      lidar_observation->set_center_x(boxes[i].center_x());
      lidar_observation->set_center_y(boxes[i].center_y());
      lidar_observation->set_center_z(boxes[i].center_z());
      lidar_observation->set_length(boxes[i].length());
      lidar_observation->set_width(boxes[i].width());
      lidar_observation->set_height(boxes[i].height());
      lidar_observation->set_yaw(boxes[i].heading());
      lidar_observation->set_type(types[i]);
      lidar_observation->set_conf(confs[i]);
    } else if ((sensorTypes[i] & ST_CAMERA) == ST_CAMERA) {
      auto camera_observation = detection->mutable_camera_observation();
      camera_observation->set_timestamp(timestamp[i]);
      camera_observation->set_center_x(boxes[i].center_x());
      camera_observation->set_center_y(boxes[i].center_y());
      camera_observation->set_center_z(boxes[i].center_z());
      camera_observation->set_length(boxes[i].length());
      camera_observation->set_width(boxes[i].width());
      camera_observation->set_height(boxes[i].height());
      camera_observation->set_yaw(boxes[i].heading());
      camera_observation->set_type(types[i]);
      camera_observation->set_conf(confs[i]);
    } else {
      Logger::GetInstance().Log(Logger::ERROR,
                                "Error: Unknown observation type.");
      std::cout << "Unknown observation type." << std::endl;
    }
  }
  return observation;
}

void WrapperBase::rotateVector(double* x, double* y, double angle) {
  // Function to rotate a 2D vector by a given angle (in radian)
  // Calculate sine and cosine of the angle
  double cs = cos(angle);
  double sn = sin(angle);
  // Apply rotation formula
  double newX = *x * cs - *y * sn;
  double newY = *x * sn + *y * cs;
  // Update the original coordinates
  *x = newX;
  *y = newY;
}
void WrapperBase::rotateVector(float* x, float* y, double angle) {
  // Function to rotate a 2D vector by a given angle (in radian)
  // Calculate sine and cosine of the angle
  float cs = cos(angle);
  float sn = sin(angle);
  // Apply rotation formula
  float newX = *x * cs - *y * sn;
  float newY = *x * sn + *y * cs;
  // Update the original coordinates
  *x = newX;
  *y = newY;
}

double WrapperBase::Kinematics_apply_absolute_velocity_limit(double velocity) {
  if (std::abs(velocity) < 0.3) {
    velocity = 0.0;
  }
  return velocity;
}
float WrapperBase::Kinematics_apply_absolute_velocity_limit(float velocity) {
  if (std::abs(velocity) < 0.3f) {
    velocity = 0.0f;
  }
  return velocity;
}

double WrapperBase::findMin(const std::vector<double>& numbers) {
  if (numbers.empty()) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  double minVal = numbers[0];  // Initialization with the first element
  for (size_t i = 1; i < numbers.size(); ++i) {
    if (numbers[i] < minVal) {
      minVal = numbers[i];
    }
  }

  return minVal;
}

std::tuple<Eigen::Matrix4d, double> WrapperBase::getVehiclePos(
    const dpc::SLAMLocalEgoOdom& vehicle_odom_vec) {
  Eigen::Matrix4d ego_pos = Eigen::Matrix4d::Zero(4, 4);
  // position
  ego_pos(0, 3) = vehicle_odom_vec.position.x;
  ego_pos(1, 3) = vehicle_odom_vec.position.y;
  ego_pos(2, 3) = vehicle_odom_vec.position.z;
  ego_pos(3, 3) = 1;
  // rotation
  double ego_yaw = vehicle_odom_vec.yaw;
  ego_pos(0, 0) = cos(ego_yaw);
  ego_pos(1, 1) = cos(ego_yaw);
  ego_pos(0, 1) = -sin(ego_yaw);
  ego_pos(1, 0) = sin(ego_yaw);
  ego_pos(2, 2) = 1;

  return std::make_tuple(
      ego_pos, static_cast<double>(vehicle_odom_vec.ego_odom_time_ns / 1e9));
}

}  // namespace tracker
