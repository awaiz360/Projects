#ifndef TRACKER_PRE_AND_POST_PROCESS_FUSION_WRAPPER_H_
#define TRACKER_PRE_AND_POST_PROCESS_FUSION_WRAPPER_H_

#include "pre_and_post_process/wrapper_base.h"

namespace tracker {
class FusionWrapper : public WrapperBase {
 public:
  explicit FusionWrapper(const size_t bufferSize);
  ~FusionWrapper() = default;

  // Overloading the father class functions
  ObservationsProto LidarInputWrapper(
      const perception::vector_LiDARPerception2Fusion& LidarResults,
      double CurrentTimestamp) override;
  ObservationsProto LidarInputWrapper(
      const std::vector<LidarPerceptionOutput>& Lidar_Det_Objs,
      const double time) override {
    return ObservationsProto();
  };

  ObservationsProto CameraInputWrapper(
      const perception::vector_CameraPerception2Fusion& FcosResults,
      double CurrentTimestamp) override;
  ObservationsProto CameraInputWrapper(
      const std::vector<InstanceData>& Camera_Det_Objs,
      double CurrentTimestamp) override {
    return ObservationsProto();
  };

  void OutputWrapper(dpc::TrafficParticipantStruct* MOT_output_objs,
                     const std::vector<TrackPtr>& Track_List,
                     const double timestamp) override;
  void OutputWrapper(
      std::vector<perception::LiDARPerception2Fusion>* MOT_output_objs,
      const std::vector<TrackPtr>& Track_List,
      const double timestamp) override {};
  void OutputWrapper(
      std::vector<perception::CameraPerception2Fusion>* MOT_output_objs,
      const std::vector<TrackPtr>& Track_List,
      const double timestamp) override {};

 private:
  uint8_t Type_Map_Fusion2EM(uint8_t fusion_type);
  uint8_t motion_type_Map_Fusion2EM(MotionType fusion_moving_state);
  uint8_t contributing_sensor_map_Fusion2EM(int fusion_sensor_type);
};

}  // namespace tracker
#endif  // TRACKER_PRE_AND_POST_PROCESS_FUSION_WRAPPER_H_
