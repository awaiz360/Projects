#include "tracker_main.h"

namespace tracker {

Tracker ObjectTracker(kEngineType);

#ifdef CSV_WRITING_FUSION
CSVWriter& tracker_csv_writer = CSVWriter::getInstance();
#endif  // CSV_WRITING_FUSION

template <typename OutputType, typename LidarType, typename CameraType>
OutputType TrackerMain(LidarType LidarResults, CameraType FcosResults,
                       dpc::SLAMLocalEgoOdom vehicle_odom_vec,
                       double CurrentTimestamp) {
#ifdef Opencv_Operation
  TrackerVisualizer& visualizer = TrackerVisualizer::getInstance();
  visualizer.initializeVisualizer();
#endif  // Opencv_Operation

#ifdef CSV_WRITING_FUSION
  tracker_csv_writer.writeMotInput(LidarResults, FcosResults, vehicle_odom_vec,
                                   CurrentTimestamp);
#endif  // CSV_WRITING_FUSION

  // Input wrapper
  ObservationsProto lidar_frame, camera_frame;
  std::tie(lidar_frame, camera_frame) =
      ObjectTracker.PreProcess<LidarType, CameraType>(
          &LidarResults, &FcosResults, vehicle_odom_vec, CurrentTimestamp);

#ifdef CSV_WRITING_FUSION
  tracker_csv_writer.writeTrackerInput(lidar_frame, camera_frame,
                                       CurrentTimestamp);
#endif  // CSV_WRITING_FUSION

  if (ObjectTracker.GetEnginType() == TET_LIDAR ||
      ObjectTracker.GetEnginType() == TET_FUSION) {
#ifdef Opencv_Operation
    visualizer.visualizeLidarTracks(lidar_frame);
#endif  // Opencv_Operation
    ObjectTracker.Tracking(lidar_frame);
  }

  if (ObjectTracker.GetEnginType() == TET_CAMERA ||
      ObjectTracker.GetEnginType() == TET_FUSION) {
#ifdef Opencv_Operation
    visualizer.visualizeCameraTracks(camera_frame);
#endif  // Opencv_Operation
    ObjectTracker.Tracking(camera_frame);
  }

  // Check result
  auto tracks = ObjectTracker.GetTracks();

#ifdef Opencv_Operation
  visualizer.visualizeTracks(tracks, vehicle_odom_vec);
#endif  // Opencv_Operation

#ifdef CSV_WRITING_FUSION
  tracker_csv_writer.writeTrackerOutput(tracks, CurrentTimestamp);
#endif  // CSV_WRITING_FUSION

  auto MOT_output_objs =
      ObjectTracker.PostProcess<OutputType>(tracks, CurrentTimestamp);

#ifdef CSV_WRITING_FUSION
  tracker_csv_writer.writeMotOutput(MOT_output_objs, CurrentTimestamp);
#endif  // CSV_WRITING_FUSION

#ifdef Opencv_Operation
  visualizer.renderAndSaveFrame();
#endif  // Opencv_Operation

  return MOT_output_objs;
}  // TrackerMain

}  // namespace tracker

// Explicit template instantiation for Fusion_Tracker case
template dpc::TrafficParticipantStruct tracker::TrackerMain<
    dpc::TrafficParticipantStruct, perception::vector_LiDARPerception2Fusion,
    perception::vector_CameraPerception2Fusion>(
    perception::vector_LiDARPerception2Fusion,
    perception::vector_CameraPerception2Fusion, dpc::SLAMLocalEgoOdom, double);

// Explicit template instantiation for Lidar-Tracker case
template std::vector<perception::LiDARPerception2Fusion> tracker::TrackerMain<
    std::vector<perception::LiDARPerception2Fusion>,
    std::vector<LidarPerceptionOutput>, std::vector<InstanceData>>(
    std::vector<LidarPerceptionOutput>, std::vector<InstanceData>,
    dpc::SLAMLocalEgoOdom, double);

// Explicit template instantiation for Camera-Tracker case
template std::vector<perception::CameraPerception2Fusion> tracker::TrackerMain<
    std::vector<perception::CameraPerception2Fusion>,
    std::vector<LidarPerceptionOutput>, std::vector<InstanceData>>(
    std::vector<LidarPerceptionOutput>, std::vector<InstanceData>,
    dpc::SLAMLocalEgoOdom, double);
