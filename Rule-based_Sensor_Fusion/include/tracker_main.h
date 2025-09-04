#ifndef TRACKER_MAIN_H
#define TRACKER_MAIN_H

#include <math.h>

#include <Eigen/Geometry>

#include "box3d.h"
#include "compile_type_def.h"
#include "const_def.h"
#include "external_types.h"
#include "tracker.h"
#ifdef CSV_WRITING_FUSION
#include "csv_writer_internal.h"
#endif  // CSV_WRITING_FUSION
#ifdef Opencv_Operation
#include "tracker_visualizer.h"
#endif  // Opencv_Operation

namespace tracker {

template <typename OutputType, typename LidarType, typename CameraType>
OutputType TrackerMain(LidarType LidarResults, CameraType FcosResults,
                       dpc::SLAMLocalEgoOdom vehicle_odom_vec,
                       double timestamp);

}  // namespace tracker
#endif  // TRACKER_MAIN_H
