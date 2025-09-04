#include "bounding_box_visualizer.h"

namespace tracker {

BoundingBoxVisualizer::BoundingBoxVisualizer() {
  cv::RNG rng(12345);
  for (size_t i = 0; i < colorNums; ++i) {
    // not (0, 256) To avoid confusion with black backgrounds
    randomColors.push_back(cv::Scalar(
        static_cast<uchar>(rng.uniform(50, 256)),  // B
        static_cast<uchar>(rng.uniform(50, 256)),  // G
        static_cast<uchar>(rng.uniform(50, 256))   // R
        ));
  }
}  // Black background

void BoundingBoxVisualizer::New_image(int imageLength, int imageWidth,
                                      float Xwindow, float Ywindow,
                                      float xCenter, float yCenter) {
  this->imageWidth = imageWidth;
  this->imageLength = imageLength;
  this->Yscale = imageWidth / Ywindow;
  this->Xscale = imageLength / Xwindow;
  this->xCenter = xCenter;
  this->yCenter = yCenter;
  this->car_posX_inPixel = (int)((this->imageLength) / 2.0);
  this->car_posY_inPixel = (int)((this->imageWidth) / 2.0);
  this->image = cv::Mat(imageWidth, imageLength, CV_8UC3, cv::Scalar(0, 0, 0));
  int gridLineWidth = 1;
  for (int i = 0; i <= Ywindow / 100; ++i) {
    int pixel = 50 * i * this->Yscale;  // Interval of 10m
    cv::line(this->image, cv::Point(0, this->car_posY_inPixel - pixel),
             cv::Point(this->imageLength, this->car_posY_inPixel - pixel),
             this->gridColor, gridLineWidth);
    cv::line(this->image, cv::Point(0, this->car_posY_inPixel + pixel),
             cv::Point(this->imageLength, this->car_posY_inPixel + pixel),
             this->gridColor, gridLineWidth);
  }
  for (int i = 0; i <= Xwindow / 100; ++i) {
    int pixel = 50 * i * this->Xscale;  // Interval of 10m
    cv::line(this->image, cv::Point(this->car_posX_inPixel - pixel, 0),
             cv::Point(this->car_posX_inPixel - pixel, this->imageWidth),
             this->gridColor, gridLineWidth);
    cv::line(this->image, cv::Point(this->car_posX_inPixel + pixel, 0),
             cv::Point(this->car_posX_inPixel + pixel, this->imageWidth),
             this->gridColor, gridLineWidth);
  }
}

void BoundingBoxVisualizer::addBoundingBox(
    const std::pair<cv::Rect, float>& boundingBox, int id, bool Input_true) {
  BoxWithID boxWithId = {boundingBox.first, boundingBox.second, id};
  if (Input_true) {
    this->boundingBoxes_in.push_back(boxWithId);
  } else {
    this->boundingBoxes_out.push_back(boxWithId);
  }
}

void BoundingBoxVisualizer::drawBoundingBoxes() const {
  cv::Mat blankImg = this->image.clone();
  std::vector<std::vector<cv::Point>> polygons;
  for (const auto& boxWithID : this->boundingBoxes_in) {
    // Create RotatedRect input:{center,size,angle}
    cv::RotatedRect rotatedRect(
        cv::Point2f(boxWithID.box.x - 0 * xCenter,
                    boxWithID.box.y - 0 * yCenter),
        cv::Size2f(boxWithID.box.width, boxWithID.box.height),
        boxWithID.yaw * 180.0 / 3.1415);
    cv::Point2f vertices[4];
    rotatedRect.points(vertices);
    std::vector<cv::Point> points_int;
    for (size_t i = 0; i < 4; ++i) {
      points_int.push_back(vertices[i]);
    }
    polygons.push_back(points_int);
    cv::fillPoly(blankImg, polygons, this->in_box_color, cv::LINE_8);
  }
  cv::addWeighted(this->image, 0.7, blankImg, 0.3, 0, this->image);
  for (const auto& boxWithID : this->boundingBoxes_out) {
    cv::RotatedRect rotatedRect(
        cv::Point2f(boxWithID.box.x - 0 * xCenter,
                    boxWithID.box.y - 0 * yCenter),
        cv::Size2f(boxWithID.box.width, boxWithID.box.height),
        boxWithID.yaw * 180.0 / 3.1415);
    cv::Point2f vertices[4];
    rotatedRect.points(vertices);
    for (int i = 0; i < 4; i++) {
      cv::line(image, vertices[i], vertices[(i + 1) % 4],
               randomColors[boxWithID.id % colorNums], 2);
    }
  }
  // cv::rectangle(this->image, box, cv::Scalar(0, 255, 0), 1, cv::LINE_8);
  // cv::rectangle(this->image, box, cv::Scalar(0, 0, 255), 1, cv::LINE_8);
}

void BoundingBoxVisualizer::clearBoundingBoxes() {
  this->boundingBoxes_in.clear();
  this->boundingBoxes_out.clear();
}

cv::Mat BoundingBoxVisualizer::getImage() const {
  return this->image.clone();  // Return a copy to avoid external modification
}

std::pair<cv::Rect, float> BoundingBoxVisualizer::Normalization(
    std::vector<double>& Inbox) {
  // Inbox:  {x-center, y-center, width, length, yaw}
  // Outbox: scaled {x-center, y-center, width, height} + theta in the center of
  // image
  cv::Rect outbox((Inbox.at(0) - xCenter) * this->Xscale,
                  (Inbox.at(1) - yCenter) * this->Yscale,
                  Inbox.at(3) * this->Xscale,
                  Inbox.at(2) * this->Yscale);  // length*width == width*height
  outbox.x += this->car_posX_inPixel;
  outbox.y += this->car_posY_inPixel;
  // cv::Rect outbox((Inbox.at(0) - Inbox.at(2) / 2.0) * this->Xscale,
  // (Inbox.at(1) - Inbox.at(3) / 2.0) * this->Yscale, (Inbox.at(0) +
  // Inbox.at(2) / 2.0) * this->Xscale, (Inbox.at(1) + Inbox.at(3) / 2.0) *
  // this->Yscale);
  return std::make_pair(outbox, (float)Inbox.at(4));
}

void BoundingBoxVisualizer::Addvehicle(float Vehicle_width,
                                       float vehicle_length, float vehicle_x,
                                       float vehicle_y, float yaw) {
  vehicle_x -= xCenter;
  vehicle_y -= yCenter;
  cv::Point pt1((-vehicle_length / 2.0 + vehicle_x) * this->Xscale,
                (-Vehicle_width / 2.0 + vehicle_y) * this->Yscale);  // Vertex 1
  cv::Point pt2((-vehicle_length / 2.0 + vehicle_x) * this->Xscale,
                (Vehicle_width / 2.0 + vehicle_y) * this->Yscale);  // Vertex 2
  cv::Point pt3((vehicle_length / 2.0 + vehicle_x) * this->Xscale,
                vehicle_y * this->Yscale);  // Vertex 3
  pt1.x += (this->imageLength) / 2.0;
  pt1.y += (this->imageWidth) / 2.0;
  pt2.x += (this->imageLength) / 2.0;
  pt2.y += (this->imageWidth) / 2.0;
  pt3.x += (this->imageLength) / 2.0;
  pt3.y += (this->imageWidth) / 2.0;
  std::vector<cv::Point> trianglePoints = {pt1, pt2, pt3};
  //        trianglePoints =
  //                Rotation_xy_plain(yaw, trianglePoints, cv::Point(vehicle_x,
  //                vehicle_y));
  std::vector<std::vector<cv::Point>> contours;
  contours.push_back(trianglePoints);
  cv::fillPoly(this->image, contours, this->ego_Veh_color,
               cv::LINE_8);  // Green filled triangle
}

void BoundingBoxVisualizer::Addlegend() {
  // Text properties
  std::string text = "Lidar Tracker";
  int fontFace = cv::FONT_HERSHEY_SIMPLEX;
  double fontScale = 1.0;
  int thickness = 4;
  // Calculate text size
  int baseline;
  cv::Size textSize =
      cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
  // Declaring the text position
  int textX = 0;  //(this->image.cols - textSize.width) / 2;
  int textY = textSize.height;
  cv::Point text_position(textX, textY);
  // Putting the text in the matrix
  putText(this->image, text, text_position, fontFace, fontScale,
          this->defaultColor, thickness);

  text = "Input Boxes: " + std::to_string(this->boundingBoxes_in.size());
  thickness = 2;
  textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
  textX = 0;
  textY += textSize.height + this->legend_linespace;  // 5 is bias
  text_position = cv::Point(textX, textY);
  putText(this->image, text, text_position, fontFace, fontScale,
          this->in_box_color, thickness);

  text = "Output Boxes: " + std::to_string(this->boundingBoxes_out.size());
  thickness = 2;
  textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
  textX = 0;
  textY += textSize.height + this->legend_linespace;
  text_position = cv::Point(textX, textY);
  putText(this->image, text, text_position, fontFace, fontScale,
          this->out_box_color, thickness);

  text = "Ego Vehicle: A Triangle Pointed Forward";
  thickness = 2;
  textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
  textX = 0;
  textY += textSize.height + this->legend_linespace;  // 5 is bias
  text_position = cv::Point(textX, textY);
  putText(this->image, text, text_position, fontFace, fontScale,
          this->ego_Veh_color, thickness);

  std::stringstream ss;
  ss << std::fixed << std::setprecision(1) << this->Velocity_show_gain;
  text = "Arrow: Velocity (scaled by " + ss.str() + ")";
  thickness = 2;
  textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
  textX = 0;
  textY += textSize.height + this->legend_linespace;
  text_position = cv::Point(textX, textY);
  putText(this->image, text, text_position, fontFace, fontScale,
          this->arrowColor, thickness);

  std::stringstream sss;
  ss.str("");
  ss << std::fixed << std::setprecision(1) << this->eclips_majorAxis_scale;
  sss << std::fixed << std::setprecision(1) << this->eclips_minorAxis_scale;
  text = "Eclipse: Position Uncertainty (scaled by (" + ss.str() + "," +
         sss.str() + "))";
  thickness = 2;
  textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
  textX = 0;
  textY += textSize.height + this->legend_linespace;
  text_position = cv::Point(textX, textY);
  putText(this->image, text, text_position, fontFace, fontScale,
          this->eclipsColor, thickness);
}

// Function to rotate a vector of cv::Point around a specified center.
std::vector<cv::Point> BoundingBoxVisualizer::Rotation_xy_plain(
    double angleRadians, const std::vector<cv::Point>& points,
    const cv::Point& center = cv::Point(0, 0)) {
  // Convert OpenCV Points to Eigen Vectors
  std::vector<Eigen::Vector2d> eigenPoints;
  eigenPoints.reserve(points.size());
  for (const cv::Point& pt : points) {
    eigenPoints.emplace_back(pt.x, pt.y);
  }

  // Create the rotation matrix
  Eigen::Matrix2d rotationMatrix;
  rotationMatrix(0, 0) = cos(angleRadians);
  rotationMatrix(0, 1) = -sin(angleRadians);
  rotationMatrix(1, 0) = sin(angleRadians);
  rotationMatrix(1, 1) = cos(angleRadians);

  // Apply rotation to each point (using Eigen's Vector2d)
  Eigen::Vector2d eigenCenter(center.x, center.y);
  std::vector<Eigen::Vector2d> rotatedEigenPoints;
  rotatedEigenPoints.reserve(eigenPoints.size());
  for (const Eigen::Vector2d& point : eigenPoints) {
    rotatedEigenPoints.push_back(rotationMatrix * (point - eigenCenter) +
                                 eigenCenter);
  }

  // Convert back to OpenCV Points
  std::vector<cv::Point> rotatedPoints;
  rotatedPoints.reserve(rotatedEigenPoints.size());
  for (const Eigen::Vector2d& pt : rotatedEigenPoints) {
    rotatedPoints.emplace_back(
        cv::Point(round(pt.x()),
                  round(pt.y())));  // Round to nearest integer for cv::Point
  }

  return rotatedPoints;
}

void BoundingBoxVisualizer::Add_Velocity(double velocityX, double velocityY,
                                         double arrowBaseX, double arrowBaseY) {
  // velocityX // Horizontal velocity (pixels per frame)
  // velocityY // Vertical velocity (pixels per frame)
  // Scaling inputs:
  arrowBaseX -= xCenter;
  arrowBaseY -= yCenter;
  velocityX *= this->Xscale * this->Velocity_show_gain;
  velocityY *= this->Yscale * this->Velocity_show_gain;
  arrowBaseX *= this->Xscale;
  arrowBaseY *= this->Yscale;

  // Point representing the base of the arrow
  cv::Point arrowBase(arrowBaseX + this->car_posX_inPixel,
                      arrowBaseY + this->car_posY_inPixel);

  // Calculate Arrow End Point
  cv::Point arrowTip(arrowBase.x + velocityX, arrowBase.y + velocityY);

  // Calculate Arrow Angle
  double angleRadians = atan2(velocityY, velocityX);
  double angleDegrees = angleRadians * 180.0 / CV_PI;

  // Draw Arrow
  arrowedLine(image, arrowBase, arrowTip, this->arrowColor, 2, cv::LINE_8, 0,
              0.2);

  // Add Arrowhead (Optional)
  int arrowHeadSize = 8;
  double tipX = arrowTip.x - arrowHeadSize * cos(angleRadians - CV_PI / 4);
  double tipY = arrowTip.y - arrowHeadSize * sin(angleRadians - CV_PI / 4);
  line(this->image, arrowTip, cv::Point(tipX, tipY), this->arrowColor, 2,
       cv::LINE_8, 0);
  tipX = arrowTip.x - arrowHeadSize * cos(angleRadians + CV_PI / 4);
  tipY = arrowTip.y - arrowHeadSize * sin(angleRadians + CV_PI / 4);
  line(this->image, arrowTip, cv::Point(tipX, tipY), this->arrowColor, 2,
       cv::LINE_8, 0);

  return;
}

// Guards the value of axis by ensuring it is within the acceptable range
// defined by min_value and max_value, and not NaN or infinity.
void BoundingBoxVisualizer::GuardAxis(double* axis, double min_value,
                                      double max_value) {
  if (axis == nullptr) {
    // If the axis pointer is null, there's nothing to guard.
    return;
  }

  if (std::isnan(*axis)) {
    // If axis is NaN, set it to the minimum value.
    *axis = min_value;
  } else if (std::isinf(*axis)) {
    if (*axis > 0) {
      // If axis is +inf, set it to the maximum value.
      *axis = max_value;
    } else {
      // If axis is -inf, set it to the minimum value.
      *axis = min_value;
    }
  } else if (*axis < min_value) {
    // If axis is below the minimum value, clamp it to the minimum.
    *axis = min_value;
  } else if (*axis > max_value) {
    // If axis is above the maximum value, clamp it to the maximum.
    *axis = max_value;
  }
}

void BoundingBoxVisualizer::Add_ID(double pos_x, double pos_y, uint ID,
                                   ObservationSensorType sensor_type,
                                   double Speed) {
  // Text properties
  pos_x -= xCenter;
  pos_y -= yCenter;
  std::string text = std::to_string(ID) + "  _";
  if ((sensor_type & ST_LIDAR) == ST_LIDAR) {
    text += "L";
  }
  if ((sensor_type & ST_CAMERA) == ST_CAMERA) {
    text += "C";
  }
  if ((sensor_type & ST_RADAR) == ST_RADAR) {
    text += "R";
  }
  if (((sensor_type & ST_LIDAR) != ST_LIDAR) &&
      ((sensor_type & ST_CAMERA) != ST_CAMERA) &&
      ((sensor_type & ST_RADAR) != ST_RADAR)) {
    text += "?";
    std::cout << "Unknown observation type in visualizer." << std::endl;
  }
  text += "  " + to_string_with_precision(Speed, 1);
  int fontFace = cv::FONT_HERSHEY_SIMPLEX;
  double fontScale = 0.8;
  int thickness = 2;
  // Calculate text size
  int baseline;
  cv::Size textSize =
      cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
  // Declaring the text position
  int textX =
      this->car_posX_inPixel + pos_x * this->Xscale - textSize.width / 2;
  int textY =
      this->car_posY_inPixel + pos_y * this->Yscale - textSize.height / 2;
  cv::Point text_position(textX, textY);
  // Putting the text in the matrix
  putText(this->image, text, text_position, fontFace, fontScale,
          randomColors[ID % colorNums], thickness);
  return;
}

void BoundingBoxVisualizer::Add_Eclipse(double pos_x, double pos_y,
                                        tracker::Matrix5d CVM) {
  pos_x -= xCenter;
  pos_y -= yCenter;
  // 1. Input Covariance Matrix (first two are x,y and the other two is vx,vy)
  cv::Mat covariance =
      (cv::Mat_<double>(2, 2) << CVM(0, 0), CVM(0, 1), CVM(1, 0), CVM(1, 1));

  // 2. Center Point (Assuming origin or known mean)
  cv::Point center(this->car_posX_inPixel + pos_x * this->Xscale,
                   this->car_posY_inPixel + pos_y * this->Yscale);

  cv::Size size;
  cv::RotatedRect ellipse;

  // Get eigenvalues and eigenvectors of the covariance matrix
  cv::Mat eigenvalues, eigenvectors;
  cv::eigen(covariance, eigenvalues, eigenvectors);

  // Calculate ellipse angle and size
  double angle =
      -atan2(eigenvectors.at<double>(0, 1), eigenvectors.at<double>(0, 0)) *
      180 / CV_PI;
  double majorAxis = sqrt(eigenvalues.at<double>(0));
  double minorAxis = sqrt(eigenvalues.at<double>(1));

  GuardAxis(&majorAxis, 0.01, 100);
  GuardAxis(&minorAxis, 0.01, 100);

#ifdef Constant_Eclipse_Area_Visualizer
  // Constant Area scale for better visualization
  float scale_up = (this->constantAreaRadius * this->constantAreaRadius) /
                   (majorAxis * minorAxis);
  this->eclips_majorAxis_scale = this->constantAreaRadius / majorAxis;
  this->eclips_minorAxis_scale = this->constantAreaRadius / minorAxis;
#endif

  // Scale axes
  majorAxis *= this->eclips_majorAxis_scale * this->Xscale;
  minorAxis *= this->eclips_minorAxis_scale * this->Yscale;

  size = cv::Size(majorAxis, minorAxis);
  ellipse = cv::RotatedRect(center, size, angle);

  // Draw the ellipse on the image
  cv::ellipse(this->image, ellipse, this->eclipsColor, cv::FILLED);
  return;
}

template <typename T>
std::string BoundingBoxVisualizer::to_string_with_precision(const T a_value,
                                                            const int n) {
  std::ostringstream out;
  out.precision(n);
  out << std::fixed << a_value;
  return std::move(out).str();
}
}  // namespace tracker
