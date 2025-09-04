#include "json_reader.h"

namespace tracker {

void JsonDataRead::ReadDataEntriesFromJson() {
  // 1. Read the JSON file
  std::ifstream inputFile(filename_);

  if (!inputFile.is_open()) {
    std::cerr << "Error: Could not open file " << filename_ << std::endl;
    return;  // Return an empty vector on error
  }

  nlohmann::json jsonData;
  inputFile >> jsonData;

  // 2. Parse the data into a vector of DataEntry structs
  for (const auto& entryJson : jsonData) {
    DataEntry entry;

    // Parse stamps
    entry.ts_double = entryJson["ts_double"].get<double>();

    // Parse ego vehicle information
    entry.ego_info.ego_odom_time_ns =
        entryJson["ts_double"].get<double>() * 1e9;
    entry.ego_info.status_inf =
        entryJson["ego_vehicle"]["status"].get<double>();
    entry.ego_info.position.x =
        entryJson["ego_vehicle"]["x_center"].get<double>();
    entry.ego_info.position.y =
        entryJson["ego_vehicle"]["y_center"].get<double>();
    entry.ego_info.position.z =
        entryJson["ego_vehicle"]["z_center"].get<double>();
    entry.ego_info.yaw = entryJson["ego_vehicle"]["yaw"].get<double>();
    entry.ego_info.angular_velocity.z =
        entryJson["ego_vehicle"]["yawdot"].get<double>();
    entry.ego_info.linear_velocity.x =
        entryJson["ego_vehicle"]["vx_center"].get<double>();
    entry.ego_info.linear_velocity.y =
        entryJson["ego_vehicle"]["vy_center"].get<double>();
    entry.ego_info.linear_velocity.z =
        entryJson["ego_vehicle"]["vz_center"].get<double>();

    // Parse boxes
    for (const auto& boxJson : entryJson["boxes"]) {
      Box box;
      box.sensor_type = boxJson["sensor_type"].get<double>();
      box.confidence = boxJson["confidence"].get<double>();
      box.x_center = boxJson["x_center"].get<double>();
      box.y_center = boxJson["y_center"].get<double>();
      box.z_center = boxJson["z_center"].get<double>();
      box.yaw = boxJson["yaw"].get<double>();
      box.length = boxJson["length"].get<double>();
      box.width = boxJson["width"].get<double>();
      box.height = boxJson["height"].get<double>();
      entry.boxes.push_back(box);
    }

    entries_.push_back(entry);
  }

  return;
}

void JsonDataRead::ConversionFromJasonToInput(
    const int frame_number,
    perception::vector_LiDARPerception2Fusion* lidar_data_out,
    perception::vector_CameraPerception2Fusion* camera_data_out,
    dpc::SLAMLocalEgoOdom* ego_data_out, double* timestamp) {
  *timestamp = entries_[frame_number].ts_double;
  lidar_data_out->clear();
  camera_data_out->clear();
  perception::LiDARPerception2Fusion data_out_1;
  perception::CameraPerception2Fusion data_out_2;
  for (auto& in_line_obj : entries_[frame_number].boxes) {
    int sentype = in_line_obj.sensor_type;
    if ((sentype & ST_LIDAR) == ST_LIDAR) {
      data_out_1.position.x = in_line_obj.x_center;
      data_out_1.position.y = in_line_obj.y_center;
      data_out_1.position.z = in_line_obj.z_center;
      data_out_1.theta = in_line_obj.yaw;
      data_out_1.width = abs(in_line_obj.width);
      data_out_1.length = abs(in_line_obj.length);
      data_out_1.height = abs(in_line_obj.height);
      data_out_1.confidence = abs(in_line_obj.confidence);
      data_out_1.timestamp = *timestamp;               // in seconds
      data_out_1.measurement_time = *timestamp * 1e6;  // in micro seconds
      lidar_data_out->push_back(data_out_1);
    } else if ((sentype & ST_CAMERA) == ST_CAMERA) {
      data_out_2.position.x = in_line_obj.x_center;
      data_out_2.position.y = in_line_obj.y_center;
      data_out_2.position.z = in_line_obj.z_center;
      data_out_2.theta = in_line_obj.yaw;
      data_out_2.width = abs(in_line_obj.width);
      data_out_2.length = abs(in_line_obj.length);
      data_out_2.height = abs(in_line_obj.height);
      data_out_2.confidence = abs(in_line_obj.confidence);
      data_out_2.timestamp = *timestamp;               // in seconds
      data_out_2.measurement_time = *timestamp * 1e6;  // in micro seconds
      camera_data_out->push_back(data_out_2);
    } else {
      std::cout << "Jason Read Erroe: Unknown observation type." << std::endl;
    }
  }
  *ego_data_out = entries_[frame_number].ego_info;

  if (kEngineType == TrackerEngineType::TET_LIDAR) {
    camera_data_out->clear();
  } else if (kEngineType == TrackerEngineType::TET_CAMERA) {
    lidar_data_out->clear();
  }

  return;
};

void JsonDataRead::ConversionFromJasonToInput(
    const int frame_number, std::vector<LidarPerceptionOutput>* lidar_data_out,
    std::vector<InstanceData>* camera_data_out,
    dpc::SLAMLocalEgoOdom* ego_data_out, double* timestamp) {
  *timestamp = entries_[frame_number].ts_double;
  lidar_data_out->clear();
  camera_data_out->clear();
  LidarPerceptionOutput data_out_1;
  InstanceData data_out_2;

  int cam_object_num = 0;
  for (auto& in_line_obj : entries_[frame_number].boxes) {
    int sentype = in_line_obj.sensor_type;
    if ((sentype & ST_CAMERA) == ST_CAMERA) ++cam_object_num;
  }
  int cam_feature_num = 9;
  data_out_2.bboxes_3d.resize(cam_object_num, cam_feature_num);
  data_out_2.labels_3d.resize(cam_object_num);
  data_out_2.scores_3d.resize(cam_object_num, 1);

  int j = 0;
  for (auto& in_line_obj : entries_[frame_number].boxes) {
    int sentype = in_line_obj.sensor_type;
    if ((sentype & ST_LIDAR) == ST_LIDAR) {
      data_out_1.position.x = in_line_obj.x_center;
      data_out_1.position.y = in_line_obj.y_center;
      data_out_1.position.z = in_line_obj.z_center;
      data_out_1.theta = in_line_obj.yaw;
      data_out_1.width = abs(in_line_obj.width);
      data_out_1.length = abs(in_line_obj.length);
      data_out_1.height = abs(in_line_obj.height);
      data_out_1.confidence = abs(in_line_obj.confidence);
      lidar_data_out->push_back(data_out_1);
    } else if ((sentype & ST_CAMERA) == ST_CAMERA) {
      data_out_2.bboxes_3d(j, 0) = in_line_obj.x_center;
      data_out_2.bboxes_3d(j, 1) = in_line_obj.y_center;
      data_out_2.bboxes_3d(j, 2) = in_line_obj.z_center;
      data_out_2.bboxes_3d(j, 6) = in_line_obj.yaw * (180 / M_PI);
      data_out_2.bboxes_3d(j, 3) = abs(in_line_obj.width);
      data_out_2.bboxes_3d(j, 5) = abs(in_line_obj.length);
      data_out_2.bboxes_3d(j, 4) = abs(in_line_obj.height);
      data_out_2.scores_3d(j, 0) = abs(in_line_obj.confidence);
      j++;
    } else {
      std::cout << "Jason Read Erroe: Unknown observation type." << std::endl;
    }
  }
  camera_data_out->push_back(data_out_2);
  *ego_data_out = entries_[frame_number].ego_info;

  if (kEngineType == TrackerEngineType::TET_LIDAR) {
    camera_data_out->clear();
  } else if (kEngineType == TrackerEngineType::TET_CAMERA) {
    lidar_data_out->clear();
  }

  return;
};

}  // namespace tracker
