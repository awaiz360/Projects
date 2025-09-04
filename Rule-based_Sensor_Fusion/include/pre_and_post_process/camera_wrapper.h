#ifndef TRACKER_PRE_AND_POST_PROCESS_CAMERA_WRAPPER_H_
#define TRACKER_PRE_AND_POST_PROCESS_CAMERA_WRAPPER_H_

#include "pre_and_post_process/wrapper_base.h"

namespace tracker {
class CameraWrapper : public WrapperBase {
 public:
  explicit CameraWrapper(const size_t bufferSize);
  ~CameraWrapper() = default;

  ObservationsProto LidarInputWrapper(
      const perception::vector_LiDARPerception2Fusion& LidarResults,
      double CurrentTimestamp) override {
    return ObservationsProto();
  };
  ObservationsProto LidarInputWrapper(
      const std::vector<LidarPerceptionOutput>& Lidar_Det_Objs,
      const double time) override {
    return ObservationsProto();
  };

  ObservationsProto CameraInputWrapper(
      const perception::vector_CameraPerception2Fusion& FcosResults,
      double CurrentTimestamp) override {
    return ObservationsProto();
  };
  ObservationsProto CameraInputWrapper(
      const std::vector<InstanceData>& Camera_Det_Objs,
      double CurrentTimestamp) override;

  void OutputWrapper(dpc::TrafficParticipantStruct* MOT_output_objs,
                     const std::vector<TrackPtr>& Track_List,
                     const double timestamp) override {};
  void OutputWrapper(
      std::vector<perception::LiDARPerception2Fusion>* MOT_output_objs,
      const std::vector<TrackPtr>& Track_List,
      const double timestamp) override {};
  void OutputWrapper(
      std::vector<perception::CameraPerception2Fusion>* MOT_output_objs,
      const std::vector<TrackPtr>& Track_List, const double timestamp) override;

 private:
  ObservationType Type_Map_CameraPerception2Tracker(int lidar_object_type);
};

}  // namespace tracker
#endif  // TRACKER_PRE_AND_POST_PROCESS_CAMERA_WRAPPER_H_
