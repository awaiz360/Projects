#ifndef TRACKER_PRE_AND_POST_PROCESS_WRAPPER_BASE_H_
#define TRACKER_PRE_AND_POST_PROCESS_WRAPPER_BASE_H_

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES  // for PI constant
#endif                     // !_USE_MATH_DEFINES
#include <math.h>

#include <Eigen/Dense>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <tuple>
#include <variant>
#include <vector>

#include "const_def.h"
#include "external_types.h"
#include "logger.h"
#include "motion_converter.h"
#include "slam_data_buffer.h"
#include "track.h"
#include "vecXd.h"

namespace tracker {

class WrapperBase {
 public:
  WrapperBase(const size_t bufferSize);
  virtual ~WrapperBase() = default;

  virtual ObservationsProto LidarInputWrapper(
      const perception::vector_LiDARPerception2Fusion& LidarResults,
      double CurrentTimestamp) = 0;
  virtual ObservationsProto LidarInputWrapper(
      const std::vector<LidarPerceptionOutput>& Lidar_Det_Objs,
      const double time) = 0;

  virtual ObservationsProto CameraInputWrapper(
      const perception::vector_CameraPerception2Fusion& FcosResults,
      double CurrentTimestamp) = 0;
  virtual ObservationsProto CameraInputWrapper(
      const std::vector<InstanceData>& Camera_Det_Objs,
      double CurrentTimestamp) = 0;

  virtual void OutputWrapper(dpc::TrafficParticipantStruct* MOT_output_objs,
                             const std::vector<TrackPtr>& Track_List,
                             const double timestamp) = 0;
  virtual void OutputWrapper(
      std::vector<perception::LiDARPerception2Fusion>* MOT_output_objs,
      const std::vector<TrackPtr>& Track_List, const double timestamp) = 0;
  virtual void OutputWrapper(
      std::vector<perception::CameraPerception2Fusion>* MOT_output_objs,
      const std::vector<TrackPtr>& Track_List, const double timestamp) = 0;

  virtual void MiddleWrapper(ObservationsProto* Lidar_frame,
                             ObservationsProto* Camera_frame,
                             const dpc::SLAMLocalEgoOdom& vehicle_odom_vec,
                             const double time);
  virtual std::tuple<Eigen::Matrix4d, double> getVehiclePos(
      const dpc::SLAMLocalEgoOdom& vehicle_odom_vec);

 protected:
  virtual void EgoWrapper(const dpc::SLAMLocalEgoOdom& vehicle_odom_vec);
  virtual ObservationsProto BuildObservation(
      const std::vector<tracker::Box3D>& boxes,
      const std::vector<uint32_t>& ids,
      const std::vector<tracker::ObservationType> types,
      const std::vector<double> confs,
      const std::vector<tracker::ObservationSensorType> sensorTypes,
      const std::vector<double> timestamp);
  virtual void rotateVector(double* x, double* y, double angle);
  virtual void rotateVector(float* x, float* y, double angle);
  virtual double Kinematics_apply_absolute_velocity_limit(double velocity);
  virtual float Kinematics_apply_absolute_velocity_limit(float velocity);

 protected:
  MotionConverter ego_vehicle_motion_;
  SLAMDataBuffer buffer_;
  double timestamp_{0.0};
  EgoCarInfo selected_vehicle_odom_data_;
  dpc::SLAMLocalEgoOdom vehicle_odom_vec_;

 private:
  double findMin(const std::vector<double>& numbers);
};

}  // namespace tracker

#endif  // TRACKER_PRE_AND_POST_PROCESS_WRAPPER_BASE_H_
