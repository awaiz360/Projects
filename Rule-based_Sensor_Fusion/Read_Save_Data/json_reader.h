#ifndef TRACKER_JSON_READER_H
#define TRACKER_JSON_READER_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES  // for PI constant
#endif                     // !_USE_MATH_DEFINES
#include <math.h>

#include "const_def.h"
#include "external_types.h"
#include "nlohmann/json.hpp"
#include "types.h"

namespace tracker {

// Structure to represent a single box
struct Box {
  double sensor_type;
  double confidence;
  double x_center, y_center, z_center, yaw, length, width, height;
};

// Structure to represent a data entry
struct DataEntry {
  std::vector<Box> boxes;
  double ts_double;
  dpc::SLAMLocalEgoOdom ego_info;
};

class JsonDataRead {
 public:
  explicit JsonDataRead(const std::string& filename) {
    filename_ = filename;
    ReadDataEntriesFromJson();
  };
  void ConversionFromJasonToInput(
      const int frame_number,
      perception::vector_LiDARPerception2Fusion* lidar_data_out,
      perception::vector_CameraPerception2Fusion* camera_data_out,
      dpc::SLAMLocalEgoOdom* ego_data_out, double* timestamp);
  void ConversionFromJasonToInput(
      const int frame_number,
      std::vector<LidarPerceptionOutput>* lidar_data_out,
      std::vector<InstanceData>* camera_data_out,
      dpc::SLAMLocalEgoOdom* ego_data_out, double* timestamp);
  int NumberOfFrames() { return entries_.size(); };

 private:
  void ReadDataEntriesFromJson();
  std::vector<DataEntry> entries_;
  std::string filename_;
};
}  // namespace tracker
#endif  // TRACKER_JSON_READER_H
