#ifndef TRACKER_PRE_AND_POST_PROCESS_LIDAR_WRAPPER_H_
#define TRACKER_PRE_AND_POST_PROCESS_LIDAR_WRAPPER_H_

#include "pre_and_post_process/wrapper_base.h"

namespace tracker {
class LidarWrapper : public WrapperBase {
 public:
  explicit LidarWrapper(const size_t bufferSize);
  ~LidarWrapper() = default;

  ObservationsProto LidarInputWrapper(
      const perception::vector_LiDARPerception2Fusion& LidarResults,
      double CurrentTimestamp) override {
    return ObservationsProto();
  };
  ObservationsProto LidarInputWrapper(
      const std::vector<LidarPerceptionOutput>& Lidar_Det_Objs,
      const double time) override;

  ObservationsProto CameraInputWrapper(
      const perception::vector_CameraPerception2Fusion& FcosResults,
      double CurrentTimestamp) override {
    return ObservationsProto();
  };
  ObservationsProto CameraInputWrapper(
      const std::vector<InstanceData>& Camera_Det_Objs,
      double CurrentTimestamp) override {
    return ObservationsProto();
  };

  void OutputWrapper(dpc::TrafficParticipantStruct* MOT_output_objs,
                     const std::vector<TrackPtr>& Track_List,
                     const double timestamp) override {};
  void OutputWrapper(
      std::vector<perception::LiDARPerception2Fusion>* MOT_output_objs,
      const std::vector<TrackPtr>& Track_List, const double timestamp) override;
  void OutputWrapper(
      std::vector<perception::CameraPerception2Fusion>* MOT_output_objs,
      const std::vector<TrackPtr>& Track_List,
      const double timestamp) override {};

 private:
  ObservationType Type_Map_LidarPerception2Tracker(
      std::string lidar_object_type);
  uint8_t MotionType_Map_LidarTracker2Fusion(
      MotionType lidar_object_motion_type);
};

}  // namespace tracker
#endif  // TRACKER_PRE_AND_POST_PROCESS_LIDAR_WRAPPER_H_
