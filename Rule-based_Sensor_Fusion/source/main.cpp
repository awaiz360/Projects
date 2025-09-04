#include "fusion_bypass.h"
#include "tracker_main.h"

#ifdef OfflineReadWrite_Operation
#include <thread>

#include "json_reader.h"
#endif  // OfflineReadWrite_Operation
#ifdef Opencv_Operation
#include "tracker_visualizer.h"
#endif  // Opencv_Operation
#ifdef Wrapper_Operation
#include "Fusion_GeneralWrapper.h"
#include "ara/log/logging.h"
ara::log::Logger& mainLog{ara::log::CreateLogger(
    "main", "Fusion MOT data logging", ara::log::LogLevel::kVerbose)};
mainLog.LogInfo() << "Fusion_MOT module data...";
GeneralWrapper gw(mainLog);
#endif  // Wrapper_Operation

TrackerLidarInputType LidarResults;
TrackerCameraInputType FcosResults;
double timestamp;
dpc::SLAMLocalEgoOdom vehicle_odom_vec;

#ifdef OfflineReadWrite_Operation
bool Loop_finished = false;
int package_counter = 0;
#endif  // OfflineReadWrite_Operation

int main() {
#ifdef Wrapper_Operation
  gw._SLAMLocalEgoOdom2SWC_Required.s.GetData(vehicle_odom_vec);
  // just for JACK_L4 project:
  /*vehicle_odom_vec.yaw = M_PI_2 - vehicle_odom_vec.yaw;*/
#endif  // Wrapper_Operation

#ifdef OfflineReadWrite_Operation
  // Scenario 1 : scenario_1_radius_15.0_velocity_2.0_noise_0.01.json
  // Radius [15.0, 20.0, 25.0, 30.0] meter
  // Velocity of the car [2.0, 5.0, 10.0, 15.0] m/s
  // Position noise [0.01, 0.02, 0.05, 0.10, 0.20]
  //
  // Scenario 2 : @test1 or @test2 :
  // scenario_2_test_1_ego_velocity_10.0_car_velocity_10.0_noise_0.01.json
  // Velocity of the Ego vehicle [10.0, 20.0, 30.0, 40.0, 50.0] m/s
  // Velocity of the car @test1 [10.0, 20.0, 30.0, 40.0, 50.0] m/s
  // Velocity of the car @test2 [2.0, 5.0, 10.0, 15.0, 20.0] m/s
  // Position noise [0.01, 0.02, 0.05, 0.10, 0.20]
  tracker::JsonDataRead ReadDataEntriesFromJson(
      "./resources/scenario_5_data/"
      "scenario_5_test_1_ego_velocity_5.0_car_velocity_10.0_noise_0.01.json");
  int num_frames = ReadDataEntriesFromJson.NumberOfFrames();
  std::cout << "Number of frames: " << num_frames << std::endl;
  while (!Loop_finished) {
    ReadDataEntriesFromJson.ConversionFromJasonToInput(
        package_counter, &LidarResults, &FcosResults, &vehicle_odom_vec,
        &timestamp);
    std::cout << "Frame number: " << package_counter
              << "   Timestamp: " << std::to_string(timestamp) << std::endl;
#endif  // OfflineReadWrite_Operation

    /*if (LidarResults.size() != 0) {
      timestamp = LidarResults.at(0).timestamp;
    }*/

#if Fusion_Bypass
    MOT_output_objs.number_of_objects = 0;
    MOT_output_objs.object_list.clear();
    MOT_output_objs = tracker::bypass_lidar_data_to_fusion_output(LidarResults);
#else
    TrackerOutputType MOT_output_objs =
        tracker::TrackerMain<TrackerOutputType, TrackerLidarInputType,
                             TrackerCameraInputType>(
            LidarResults, FcosResults, vehicle_odom_vec, timestamp);
#endif

#ifdef Wrapper_Operation
    gw._LiDARPerception2Fusion_Provide.s.SetData(MOT_output_objs);
    gw._LiDARPerception2Fusion_Provide.Publish();
#endif  // Wrapper_Operation

#ifdef OfflineReadWrite_Operation
    // loop handler
    package_counter++;
    if (package_counter == num_frames) {
      Loop_finished = true;
#ifdef Opencv_Operation
      tracker::TrackerVisualizer::getInstance().finalizeVideo();
#endif  // Opencv_Operation
#ifdef CSV_WRITING_FUSION
      tracker::CSVWriter::getInstance().close();
#endif  // CSV_WRITING_FUSION
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
#endif  // OfflineReadWrite_Operation

}  // main
