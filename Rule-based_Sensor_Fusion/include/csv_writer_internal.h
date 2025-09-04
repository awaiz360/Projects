#ifndef TRACKER_CSV_WRITER_H
#define TRACKER_CSV_WRITER_H

#include <filesystem>  // for C++17 and later
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "external_types.h"

namespace tracker {

namespace csv {
enum class DataType {
  UnprocessedLidarData,
  ProcessedLidarData,
  UnprocessedCameraData,
  ProcessedCameraData,
  UnprocessedEgoVehicleData,
  UnprocessedTrackerOutput,
  ProcessedTrackerOutput
};
}

class CSVWriter {
 private:
  explicit CSVWriter(const std::string& file_address_, char delimiter_)
      : file_address_(file_address_), delimiter_(delimiter_) {
    // Check if the file exists, and if so, remove it
    if (std::filesystem::exists(file_address_)) {
      std::filesystem::remove(file_address_);
      std::cout << "Existing CSV file removed: " << file_address_ << std::endl;
    }

    outFile_.open(file_address_);  // Now open the file for writing
    if (!outFile_.is_open()) {
      std::cerr << "Error opening CSV file: " << file_address_ << std::endl;
    }
    writeRows(
        {{"Time",        "measure_time", "data_size", "X",           "Y",
          "Z",           "roll",         "pitch",     "yaw",         "Vx",
          "Vy",          "Vz",           "Ax",        "Ay",          "Az",
          "width",       "length",       "height",    "confidence",  "type",
          "motion_type", "ID",           "data_type", "first_object"}});
  }

  template <typename T>
  void writeRow(const std::vector<T>& row) {
    if (!row.empty()) {
      std::ostringstream ss;
      ss << row[0];
      for (size_t i = 1; i < row.size(); ++i) {
        ss << delimiter_ << row[i];
      }
      ss << std::endl;
      outFile_ << ss.str();
    }
  }

  void writeRows(const std::vector<std::vector<std::string>>& rows) {
    for (const auto& row : rows) {
      writeRow(row);
    }
  }

  void addEmptyRow(std::vector<std::vector<std::string>>* rows,
                   const double timestamp, const std::string data_type,
                   const std::string first_epoc) {
    rows->push_back({std::to_string(timestamp),
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     data_type,
                     first_epoc});
  }

  // Function to map DataType to the corresponding string in csvStringData
  std::string CsvDataTypeToString(csv::DataType data_type) {
    switch (data_type) {
      case csv::DataType::UnprocessedLidarData:
        return data_type_name_.unprocessed_lidar_data;
      case csv::DataType::ProcessedLidarData:
        return data_type_name_.processed_lidar_data;
      case csv::DataType::UnprocessedCameraData:
        return data_type_name_.unprocessed_camera_data;
      case csv::DataType::ProcessedCameraData:
        return data_type_name_.processed_camera_data;
      case csv::DataType::UnprocessedEgoVehicleData:
        return data_type_name_.unprocessed_ego_vehicle_data;
      case csv::DataType::UnprocessedTrackerOutput:
        return data_type_name_.unprocessed_tracker_output;
      case csv::DataType::ProcessedTrackerOutput:
        return data_type_name_.processed_tracker_output;
      default:
        // unexpected input
        std::cout << "Wrong CSV data type input!" << std::endl;
        return "Unknown";
    }
  }

 public:
  static CSVWriter& getInstance(
      const std::string& file_address_ = "./mot_output/fusion_analys_data.csv ",
      char delimiter_ = ',') {
    static CSVWriter instance(file_address_,
                              delimiter_);  // Instance created on first call
    return instance;
  }

  void writeEmptyData(const csv::DataType data_type, const double timestamp) {
    std::vector<std::vector<std::string>> rows;
    std::string first_epoc = "";
    first_epoc = "True";
    addEmptyRow(&rows, timestamp, CsvDataTypeToString(data_type), first_epoc);
    writeRows(rows);
  }

  void writeMotInput(
      const perception::vector_LiDARPerception2Fusion& RuleBasedResult,
      const perception::vector_CameraPerception2Fusion& FcosResults,
      const dpc::SLAMLocalEgoOdom& vehicle_odom_vec, const double timestamp) {
    int I_id = 0;  // Initialize object ID counter
    std::vector<std::vector<std::string>> rows;
    std::string first_epoc = "";

    // Write Lidar Data
    if (RuleBasedResult.empty()) {
      addEmptyRow(&rows, timestamp, data_type_name_.unprocessed_lidar_data,
                  first_epoc);
    }
    first_epoc = "True";
    for (const auto& lidarObj : RuleBasedResult) {
      rows.push_back({std::to_string(timestamp),
                      std::to_string(lidarObj.measurement_time),
                      std::to_string(RuleBasedResult.size()),
                      std::to_string(lidarObj.position.x),
                      std::to_string(lidarObj.position.y),
                      std::to_string(lidarObj.position.z),
                      "",
                      "",
                      std::to_string(lidarObj.theta),
                      "",
                      "",
                      "",
                      "",
                      "",
                      "",
                      std::to_string(lidarObj.width),
                      std::to_string(lidarObj.length),
                      std::to_string(lidarObj.height),
                      std::to_string(lidarObj.confidence),
                      std::to_string(lidarObj.type),
                      "",
                      std::to_string(++I_id),
                      data_type_name_.unprocessed_lidar_data,
                      first_epoc});
      first_epoc = "";
    }

    // Write Camera Data
    I_id = 0;  // Reset object ID counter
    if (FcosResults.empty()) {
      addEmptyRow(&rows, timestamp, data_type_name_.unprocessed_camera_data,
                  first_epoc);
    }
    first_epoc = "True";
    for (const auto& fcosObj : FcosResults) {
      rows.push_back({std::to_string(timestamp),
                      std::to_string(fcosObj.measurement_time),
                      std::to_string(FcosResults.size()),
                      std::to_string(fcosObj.position.x),
                      std::to_string(fcosObj.position.y),
                      std::to_string(fcosObj.position.z),
                      "",
                      "",
                      std::to_string(fcosObj.theta),
                      "",
                      "",
                      "",
                      "",
                      "",
                      "",
                      std::to_string(fcosObj.width),
                      std::to_string(fcosObj.length),
                      std::to_string(fcosObj.height),
                      std::to_string(fcosObj.confidence),
                      std::to_string(fcosObj.type),
                      "",
                      std::to_string(++I_id),
                      data_type_name_.unprocessed_camera_data,
                      first_epoc});
      first_epoc = "";
    }

    // Write Ego Vehicle Data
    first_epoc = "True";
    addVehicleOdomRow(&rows, timestamp, vehicle_odom_vec, first_epoc);
    writeRows(rows);
  }

  void writeMotInput(const std::vector<LidarPerceptionOutput>& RuleBasedResult,
                     const std::vector<InstanceData>& FcosResults,
                     const dpc::SLAMLocalEgoOdom& vehicle_odom_vec,
                     const double timestamp) {
    int I_id = 0;  // Initialize object ID counter
    std::vector<std::vector<std::string>> rows;
    std::string first_epoc = "";

    // Write Lidar Data
    if (RuleBasedResult.empty()) {
      addEmptyRow(&rows, timestamp, data_type_name_.unprocessed_lidar_data,
                  first_epoc);
    }
    first_epoc = "True";
    for (const auto& lidarObj : RuleBasedResult) {
      rows.push_back({std::to_string(timestamp),
                      "",
                      std::to_string(RuleBasedResult.size()),
                      std::to_string(lidarObj.position.x),
                      std::to_string(lidarObj.position.y),
                      std::to_string(lidarObj.position.z),
                      "",
                      "",
                      std::to_string(lidarObj.theta),
                      "",
                      "",
                      "",
                      "",
                      "",
                      "",
                      std::to_string(lidarObj.width),
                      std::to_string(lidarObj.length),
                      std::to_string(lidarObj.height),
                      std::to_string(lidarObj.confidence),
                      lidarObj.type,
                      "",
                      std::to_string(++I_id),
                      data_type_name_.unprocessed_lidar_data,
                      first_epoc});
      first_epoc = "";
    }

    // Write Camera Data
    if (FcosResults.empty()) {
      addEmptyRow(&rows, timestamp, data_type_name_.unprocessed_camera_data,
                  first_epoc);
    } else if (FcosResults[0].bboxes_3d.size()) {
      addEmptyRow(&rows, timestamp, data_type_name_.unprocessed_camera_data,
                  first_epoc);
    } else {
      I_id = 0;  // Reset object ID counter
      first_epoc = "True";
      const auto& bboxes_3d = FcosResults[0].bboxes_3d;
      const auto& labels = FcosResults[0].labels_3d;
      const auto& scores = FcosResults[0].scores_3d;
      for (uint32_t j = 0; j < bboxes_3d.rows(); j++) {
        rows.push_back({std::to_string(timestamp),
                        "",
                        std::to_string(bboxes_3d.size()),
                        std::to_string(bboxes_3d(j, 0)),
                        std::to_string(bboxes_3d(j, 1)),
                        std::to_string(bboxes_3d(j, 2)),
                        "",
                        "",
                        std::to_string(bboxes_3d(j, 6) * (M_PI / 180.0)),
                        "",
                        "",
                        "",
                        "",
                        "",
                        "",
                        std::to_string(bboxes_3d(j, 3)),  // width
                        std::to_string(bboxes_3d(j, 5)),  // length
                        std::to_string(bboxes_3d(j, 4)),  // height
                        std::to_string(scores(j)),
                        std::to_string(labels[j]),
                        "",
                        std::to_string(++I_id),
                        data_type_name_.unprocessed_camera_data,
                        first_epoc});
        first_epoc = "";
      }
    }

    // Write Ego Vehicle Data
    first_epoc = "True";
    addVehicleOdomRow(&rows, timestamp, vehicle_odom_vec, first_epoc);
    writeRows(rows);
  }

  void writeTrackerInput(const ObservationsProto& lidar_frame,
                         const ObservationsProto& camera_frame,
                         const double timestamp) {
    int I_id = 0;  // Initialize object ID counter
    std::vector<std::vector<std::string>> rows;
    std::string first_epoc = "";

    // Write Lidar Output Data
    if (lidar_frame.observations().empty()) {
      addEmptyRow(&rows, timestamp, data_type_name_.processed_lidar_data,
                  first_epoc);
    }
    first_epoc = "True";
    for (const auto& lidarObj : lidar_frame.observations()) {
      rows.push_back({std::to_string(timestamp),
                      std::to_string(lidarObj.lidar_observation().timestamp()),
                      std::to_string(lidar_frame.observations_size()),
                      std::to_string(lidarObj.lidar_observation().center_x()),
                      std::to_string(lidarObj.lidar_observation().center_y()),
                      std::to_string(lidarObj.lidar_observation().center_z()),
                      "",
                      "",
                      std::to_string(lidarObj.lidar_observation().yaw()),
                      "",
                      "",
                      "",
                      "",
                      "",
                      "",
                      std::to_string(lidarObj.lidar_observation().width()),
                      std::to_string(lidarObj.lidar_observation().length()),
                      std::to_string(lidarObj.lidar_observation().height()),
                      std::to_string(lidarObj.lidar_observation().conf()),
                      std::to_string(lidarObj.lidar_observation().type()),
                      "",
                      std::to_string(++I_id),
                      data_type_name_.processed_lidar_data,
                      first_epoc});
      first_epoc = "";
    }

    // Write Camera Output Data
    I_id = 0;  // Reset object ID counter
    if (camera_frame.observations().empty()) {
      addEmptyRow(&rows, timestamp, data_type_name_.processed_camera_data,
                  first_epoc);
    }
    first_epoc = "True";
    for (const auto& cameraObj : camera_frame.observations()) {
      rows.push_back(
          {std::to_string(timestamp),
           std::to_string(cameraObj.camera_observation().timestamp()),
           std::to_string(camera_frame.observations_size()),
           std::to_string(cameraObj.camera_observation().center_x()),
           std::to_string(cameraObj.camera_observation().center_y()),
           std::to_string(cameraObj.camera_observation().center_z()),
           "",
           "",
           std::to_string(cameraObj.camera_observation().yaw()),
           "",
           "",
           "",
           "",
           "",
           "",
           std::to_string(cameraObj.camera_observation().width()),
           std::to_string(cameraObj.camera_observation().length()),
           std::to_string(cameraObj.camera_observation().height()),
           std::to_string(cameraObj.camera_observation().conf()),
           std::to_string(cameraObj.camera_observation().type()),
           "",
           std::to_string(++I_id),
           data_type_name_.processed_camera_data,
           first_epoc});
      first_epoc = "";
    }

    writeRows(rows);
  }

  void writeTrackerOutput(
      const std::vector<std::shared_ptr<tracker::Track>>& tracks,
      const double timestamp) {
    std::vector<std::vector<std::string>> rows;
    std::string first_epoc = "";
    // Write Tracker Output Data
    if (tracks.empty()) {
      addEmptyRow(&rows, timestamp, data_type_name_.unprocessed_tracker_output,
                  first_epoc);
    }
    first_epoc = "True";
    for (const auto& track : tracks) {
      rows.push_back(
          {std::to_string(timestamp),
           std::to_string(timestamp),
           std::to_string(tracks.size()),
           std::to_string(track->track_state.bounding_box.center_x()),
           std::to_string(track->track_state.bounding_box.center_y()),
           std::to_string(track->track_state.bounding_box.center_z()),
           "",
           "",
           std::to_string(track->track_state.bounding_box.heading()),
           std::to_string(track->track_state.estimator.vx()),
           std::to_string(track->track_state.estimator.vy()),
           "",
           "",
           "",
           "",
           std::to_string(track->track_state.bounding_box.width()),
           std::to_string(track->track_state.bounding_box.length()),
           std::to_string(track->track_state.bounding_box.height()),
           std::to_string(track->track_state.conf),
           std::to_string(track->track_state.type),
           "",
           std::to_string(track->track_state.id),
           data_type_name_.unprocessed_tracker_output,
           first_epoc});
      first_epoc = "";
    }
    writeRows(rows);
  }

  void writeMotOutput(const dpc::TrafficParticipantStruct& MOT_output_objs,
                      const double timestamp) {
    std::vector<std::vector<std::string>> rows;
    std::string first_epoc = "";
    // Write MOT Output Data
    if (MOT_output_objs.object_list.empty()) {
      addEmptyRow(&rows, timestamp, data_type_name_.processed_tracker_output,
                  first_epoc);
    }
    first_epoc = "True";
    for (const auto& fusionObj : MOT_output_objs.object_list) {
      rows.push_back({std::to_string(timestamp),
                      "",
                      std::to_string(MOT_output_objs.number_of_objects),
                      std::to_string(fusionObj.relative_x),
                      std::to_string(fusionObj.relative_y),
                      "",
                      "",
                      "",
                      std::to_string(fusionObj.relative_orientation),
                      std::to_string(fusionObj.absolute_velocity_x),
                      std::to_string(fusionObj.absolute_velocity_y),
                      "",
                      std::to_string(fusionObj.absolute_acceleration_x),
                      std::to_string(fusionObj.absolute_acceleration_y),
                      "",
                      std::to_string(fusionObj.width),
                      std::to_string(fusionObj.length),
                      std::to_string(fusionObj.height),
                      "",
                      std::to_string(fusionObj.type),
                      std::to_string(fusionObj.motion_type),
                      std::to_string(fusionObj.id),
                      data_type_name_.processed_tracker_output,
                      first_epoc});
      first_epoc = "";
    }
    writeRows(rows);
  }

  void writeMotOutput(
      const perception::vector_LiDARPerception2Fusion& MOT_output_objs,
      const double timestamp) {
    std::vector<std::vector<std::string>> rows;
    std::string first_epoc = "";
    // Write MOT Output Data
    if (MOT_output_objs.empty()) {
      addEmptyRow(&rows, timestamp, data_type_name_.processed_tracker_output,
                  first_epoc);
    }
    first_epoc = "True";
    for (const auto& lidarObj : MOT_output_objs) {
      rows.push_back({std::to_string(timestamp),
                      std::to_string(lidarObj.measurement_time),
                      std::to_string(MOT_output_objs.size()),
                      std::to_string(lidarObj.position.x),
                      std::to_string(lidarObj.position.y),
                      std::to_string(lidarObj.position.z),
                      "",
                      "",
                      std::to_string(lidarObj.theta),
                      std::to_string(lidarObj.velocity.x),
                      std::to_string(lidarObj.velocity.y),
                      std::to_string(lidarObj.velocity.z),
                      std::to_string(lidarObj.acceleration.x),
                      std::to_string(lidarObj.acceleration.y),
                      std::to_string(lidarObj.acceleration.z),
                      std::to_string(lidarObj.width),
                      std::to_string(lidarObj.length),
                      std::to_string(lidarObj.height),
                      std::to_string(lidarObj.confidence),
                      std::to_string(lidarObj.type),
                      std::to_string(lidarObj.motion_type),
                      std::to_string(lidarObj.ID),
                      data_type_name_.processed_tracker_output,
                      first_epoc});
      first_epoc = "";
    }
    writeRows(rows);
  }

  void writeMotOutput(
      const perception::vector_CameraPerception2Fusion& MOT_output_objs,
      const double timestamp) {
    std::vector<std::vector<std::string>> rows;
    std::string first_epoc = "";
    // Write MOT Output Data
    if (MOT_output_objs.empty()) {
      addEmptyRow(&rows, timestamp, data_type_name_.processed_tracker_output,
                  first_epoc);
    }
    first_epoc = "True";
    for (const auto& cameraObj : MOT_output_objs) {
      rows.push_back({std::to_string(timestamp),
                      std::to_string(cameraObj.measurement_time),
                      std::to_string(MOT_output_objs.size()),
                      std::to_string(cameraObj.position.x),
                      std::to_string(cameraObj.position.y),
                      std::to_string(cameraObj.position.z),
                      "",
                      "",
                      std::to_string(cameraObj.theta),
                      std::to_string(cameraObj.velocity.x),
                      std::to_string(cameraObj.velocity.y),
                      std::to_string(cameraObj.velocity.z),
                      std::to_string(cameraObj.acceleration.x),
                      std::to_string(cameraObj.acceleration.y),
                      std::to_string(cameraObj.acceleration.z),
                      std::to_string(cameraObj.width),
                      std::to_string(cameraObj.length),
                      std::to_string(cameraObj.height),
                      std::to_string(cameraObj.confidence),
                      std::to_string(cameraObj.type),
                      "",
                      std::to_string(cameraObj.ID),
                      data_type_name_.processed_tracker_output,
                      first_epoc});
      first_epoc = "";
    }
    writeRows(rows);
  }

  void close() { outFile_.close(); }

 private:
  void addVehicleOdomRow(std::vector<std::vector<std::string>>* rows,
                         const double timestamp,
                         const dpc::SLAMLocalEgoOdom& vehicle_odom_vec,
                         std::string first_epoc) {
    rows->push_back({std::to_string(timestamp),
                     std::to_string(vehicle_odom_vec.ego_odom_time_ns),
                     "1",
                     std::to_string(vehicle_odom_vec.position.x),
                     std::to_string(vehicle_odom_vec.position.y),
                     std::to_string(vehicle_odom_vec.position.z),
                     std::to_string(vehicle_odom_vec.roll),
                     std::to_string(vehicle_odom_vec.pitch),
                     std::to_string(vehicle_odom_vec.yaw),
                     std::to_string(vehicle_odom_vec.linear_velocity.x),
                     std::to_string(vehicle_odom_vec.linear_velocity.y),
                     std::to_string(vehicle_odom_vec.linear_velocity.z),
                     std::to_string(vehicle_odom_vec.linear_acceleration.x),
                     std::to_string(vehicle_odom_vec.linear_acceleration.y),
                     std::to_string(vehicle_odom_vec.linear_acceleration.z),
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     "",
                     data_type_name_.unprocessed_ego_vehicle_data,
                     first_epoc});
  }

 private:
  struct csvStringData {
    std::string unprocessed_lidar_data = "Unprocessed Lidar Data";
    std::string processed_lidar_data = "Processed Lidar Data";
    std::string unprocessed_camera_data = "Unprocessed Camera Data";
    std::string processed_camera_data = "Processed Camera Data";
    std::string unprocessed_ego_vehicle_data = "Unprocessed Ego Vehicle Data";
    std::string unprocessed_tracker_output = "Unprocessed Tracker Output";
    std::string processed_tracker_output = "Processed Tracker Output";
  } data_type_name_;
  std::string file_address_;
  std::ofstream outFile_;
  char delimiter_;
};

}  // namespace tracker

#endif  // TRACKER_CSV_WRITER_H
