#ifndef BOUNDING_BOX_VISUALIZER_H
#define BOUNDING_BOX_VISUALIZER_H

#include <Eigen/Dense>  // for rotation matrix: Rotation_xy_plain
#include <cmath>        // For trigonometry

#include "estimator_filter/ctrv_ekf.h"
#include "opencv2/opencv.hpp"
#include "types.h"

namespace tracker {

class BoundingBoxVisualizer {
 public:
  struct BoxWithID {
    cv::Rect box;
    float yaw;
    int id;
  };

  BoundingBoxVisualizer();
  void addBoundingBox(const std::pair<cv::Rect, float>& boundingBox, int id,
                      bool Input_true);
  void drawBoundingBoxes() const;
  cv::Mat getImage() const;
  std::pair<cv::Rect, float> Normalization(std::vector<double>& Inbox);
  void New_image(int imageLength = 640, int imageWidth = 480,
                 float Xwindow = 50.0, float Ywindow = 100.0, float xCenter = 0,
                 float yCenter = 0);
  void Addvehicle(float Vehicle_width, float vehicle_length, float vehicle_x,
                  float vehicle_y, float yaw);
  void clearBoundingBoxes();
  void Addlegend();
  void Add_Velocity(double velocityX, double velocityY, double arrowBaseX,
                    double arrowBaseY);
  void Add_ID(double pos_x, double pos_y, uint ID,
              ObservationSensorType sensor_type, double Speed);
  void Add_Eclipse(double pso_x, double pos_y, tracker::Matrix5d CVM);

 private:
  cv::Mat image;
  std::vector<BoxWithID> boundingBoxes_in;
  std::vector<BoxWithID> boundingBoxes_out;
  cv::Scalar defaultColor =
      cv::Scalar(255, 255, 255);                      // Default to white in BGR
  cv::Scalar arrowColor = cv::Scalar(141, 255, 255);  // arrow color in BGR
  cv::Scalar eclipsColor =
      cv::Scalar(128, 158, 255, 125);  // eclipse color in BGR
  cv::Scalar ego_Veh_color =
      cv::Scalar(143, 143, 8);                       // Ego vehicle color in BGR
  cv::Scalar in_box_color = cv::Scalar(0, 255, 0);   // input boxes color in BGR
  cv::Scalar out_box_color = cv::Scalar(0, 0, 255);  // input boxes color in BGR
  cv::Scalar gridColor = cv::Scalar(255, 255, 255);  // gird color in BGR
  std::vector<cv::Scalar> randomColors;              // random color
  int colorNums = 32;                                // random color count
  int defaultThickness = 2;
  float Xscale = 0;
  float Yscale = 0;
  float xCenter = 0;
  float yCenter = 0;
  int imageWidth = 640;
  int imageLength = 640;
  int car_posX_inPixel = 0;
  int car_posY_inPixel = 0;
  int legend_linespace = 10;
  float Velocity_show_gain = 2;
  float eclips_majorAxis_scale = 5.0;
  float eclips_minorAxis_scale = 5.0;
  std::vector<cv::Point> Rotation_xy_plain(double angleRadians,
                                           const std::vector<cv::Point>& points,
                                           const cv::Point& center);
  void GuardAxis(double* axis, double min_value, double max_value);

  template <typename T>
  std::string to_string_with_precision(const T a_value, const int n = 6);
// #define Constant_Eclipse_Area_Visualizer
#undef Constant_Eclipse_Area_Visualizer
#ifdef Constant_Eclipse_Area_Visualizer
  const double constantAreaRadius = 3.0;  // radius of circle with constant area
#endif
};
}  // namespace tracker
#endif
