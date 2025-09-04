#pragma once

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES  // for PI constant
#endif                     // !_USE_MATH_DEFINES
#include <math.h>

#include <opencv2/opencv.hpp>

#include "bounding_box_visualizer.h"
#include "const_def.h"
#include "types.h"

namespace tracker {

class TrackerVisualizer {
 public:
  static TrackerVisualizer& getInstance() {
    static TrackerVisualizer instance;
    return instance;
  }

  BoundingBoxVisualizer& getVisualizer() { return visualizer_; }
  cv::VideoWriter& getVideoWriter() { return video_writer_; }

  void initializeVisualizer() {
    visualizer_.New_image(frameLength_, frameWidth_, X_window_, Y_window_,
                          xCenter_window_, yCenter_window_);
  }

  void visualizeLidarTracks(const ObservationsProto& lidar_frame) {
    for (auto& track : lidar_frame.observations()) {
      std::cout << "X in: " << track.lidar_observation().center_x()
                << " ,Y in: " << track.lidar_observation().center_y()
                << " ,Yaw: " << track.lidar_observation().yaw() << std::endl;

      std::vector<double> box1 = {
          track.lidar_observation().center_x(),
          track.lidar_observation().center_y(),
          track.lidar_observation().width() * car_scale_,
          track.lidar_observation().length() * car_scale_,
          track.lidar_observation().yaw()};

      visualizer_.addBoundingBox(visualizer_.Normalization(box1), 0, true);
    }
  }

  void visualizeCameraTracks(const ObservationsProto& camera_frame) {
    for (auto& track : camera_frame.observations()) {
      std::cout << "X in: " << track.camera_observation().center_x()
                << " ,Y in: " << track.camera_observation().center_y()
                << " ,Yaw: " << track.camera_observation().yaw() << std::endl;

      std::vector<double> box1 = {
          track.camera_observation().center_x(),
          track.camera_observation().center_y(),
          track.camera_observation().width() * car_scale_,
          track.camera_observation().length() * car_scale_,
          track.camera_observation().yaw()};

      visualizer_.addBoundingBox(visualizer_.Normalization(box1), 0, true);
    }
  }

  void visualizeTracks(const std::vector<TrackPtr>& tracks,
                       const dpc::SLAMLocalEgoOdom& vehicle_odom_vec) {
    for (auto& track : tracks) {
      if (track->track_state.lifetime < kMinimumlifetime) {
        continue;
      }
      std::cout << "X out: " << track->track_state.estimator.x()
                << " ,Y out: " << track->track_state.estimator.y()
                << " ,Yaw: " << track->track_state.bounding_box.heading()
                << " ,ID: " << std::to_string(track->track_state.id)
                << " ,Life_State: "
                << std::to_string(track->track_state.life_state) << std::endl;

      std::vector<double> box1 = {track->track_state.estimator.x(),
                                  track->track_state.estimator.y(),
                                  track->track_state.bounding_box.width(),
                                  track->track_state.bounding_box.length(),
                                  track->track_state.bounding_box.heading()};

      visualizer_.addBoundingBox(visualizer_.Normalization(box1),
                                 track->track_state.id, false);

      visualizer_.Add_Eclipse(track->track_state.estimator.x(),
                              track->track_state.estimator.y(),
                              track->track_state.estimator.P_matrix());

      visualizer_.Add_Velocity(
          track->track_state.estimator.vx(), track->track_state.estimator.vy(),
          track->track_state.estimator.x(), track->track_state.estimator.y());

      double object_speed = sqrt(pow(track->track_state.estimator.vx(), 2) +
                                 pow(track->track_state.estimator.vy(), 2));

      visualizer_.Add_ID(
          track->track_state.estimator.x(), track->track_state.estimator.y(),
          track->track_state.id, track->track_state.sensor_type, object_speed);
    }

    visualizer_.Addvehicle(
        2 * car_scale_, 4 * car_scale_, vehicle_odom_vec.position.x,
        vehicle_odom_vec.position.y, M_PI_2 - vehicle_odom_vec.yaw);

    visualizer_.Add_Velocity(
        vehicle_odom_vec.linear_velocity.x, vehicle_odom_vec.linear_velocity.y,
        vehicle_odom_vec.position.x, vehicle_odom_vec.position.y);
  }

  void renderAndSaveFrame() {
    visualizer_.drawBoundingBoxes();
    visualizer_.Addlegend();

    cv::Mat finalImage_in = visualizer_.getImage();
    visualizer_.clearBoundingBoxes();

    // Write images (commented out):
    // cv::imwrite(("./mot_output/output_image_"+std::to_string(package_counter)+".jpg"),
    // finalImage_in); cv::imshow("Image with Bounding Boxes", finalImage_in);
    // cv::waitKey(0);

    // Write video:
    video_writer_.write(finalImage_in);
  }

  void finalizeVideo() {
    video_writer_.release();
    std::cout << "Video created successfully!" << std::endl;
  }

 private:
  TrackerVisualizer()
      : frameWidth_(1500),
        frameLength_(1500),
        X_window_(400),
        Y_window_(400),
        xCenter_window_(150),
        yCenter_window_(0),
        car_scale_(2),
        fps_(10),
        visualizer_(),  // Initialize with custom dimensions if needed
        video_writer_("./mot_output/output_video.avi",
                      cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps_,
                      cv::Size(frameLength_, frameWidth_)) {}
  // other format: 'M','J','P','G'   -    'D','I','V','X'   -    'H','2','6','4'

  TrackerVisualizer(const TrackerVisualizer&) = delete;
  TrackerVisualizer& operator=(const TrackerVisualizer&) = delete;

  int frameWidth_;        // in pixel
  int frameLength_;       // in pixel
  float X_window_;        // in meter
  float Y_window_;        // in meter
  float xCenter_window_;  // in meter
  float yCenter_window_;  // in meter
  int car_scale_;
  int fps_;

  BoundingBoxVisualizer visualizer_;
  cv::VideoWriter video_writer_;
};
}  // namespace tracker
