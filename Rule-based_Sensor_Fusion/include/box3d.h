#ifndef TRACKER_BOX3D_H_
#define TRACKER_BOX3D_H_

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "Eigen/Core"
#include "vecXd.h"

namespace tracker {

class Box3D {
 public:
  Box3D() = default;

  // Constructor using center, heading, pitch, length, width, and height
  Box3D(const Vec3d& center, double heading, double pitch, double length,
        double width, double height)
      : center_(center),
        length_(length),
        width_(width),
        height_(height),
        half_length_(length / 2.0),
        half_width_(width / 2.0),
        half_height_(height / 2.0),
        heading_(heading),
        pitch_(pitch),
        cos_heading_(std::cos(heading)),
        sin_heading_(std::sin(heading)),
        cos_pitch_(std::cos(pitch)),
        sin_pitch_(std::sin(pitch)) {
    InitCorners();
  }

  // Constructor using a point, heading, pitch, front/back lengths, width, and
  // height
  Box3D(const Vec3d& point, double heading, double pitch, double front_length,
        double back_length, double width, double height)
      : length_(front_length + back_length),
        width_(width),
        height_(height),
        half_length_(length_ / 2.0),
        half_width_(width / 2.0),
        half_height_(height / 2.0),
        heading_(heading),
        pitch_(pitch),
        cos_heading_(std::cos(heading)),
        sin_heading_(std::sin(heading)),
        cos_pitch_(std::cos(pitch)),
        sin_pitch_(std::sin(pitch)) {
    double delta_length = (front_length - back_length) / 2.0;
    center_ = Vec3d(point.x() + cos_heading_ * delta_length,
                    point.y() + sin_heading_ * delta_length,
                    point.z());  // Assuming the point is in the x-y plane
    InitCorners();
  }

  const Vec3d& center() const { return center_; }
  double center_x() const { return center_.x(); }
  double center_y() const { return center_.y(); }
  double center_z() const { return center_.z(); }
  double length() const { return length_; }
  double width() const { return width_; }
  double height() const { return height_; }
  double half_length() const { return half_length_; }
  double half_width() const { return half_width_; }
  double half_height() const { return half_height_; }
  double heading() const { return heading_; }
  double pitch() const { return pitch_; }
  double volume() const { return length_ * width_ * height_; }  // 3D volume

  // Trigonometric getters (you might add more based on your rotation needs)
  double cos_heading() const { return cos_heading_; }
  double sin_heading() const { return sin_heading_; }
  double cos_pitch() const { return cos_pitch_; }
  double sin_pitch() const { return sin_pitch_; }

  // Getters for corners and dimensions
  void GetAllCorners(std::vector<Vec3d>* const corners) const;
  const std::vector<Vec3d>& GetAllCorners() const;
  double max_x() const { return max_x_; }
  double min_x() const { return min_x_; }
  double max_y() const { return max_y_; }
  double min_y() const { return min_y_; }
  double max_z() const { return max_z_; }
  double min_z() const { return min_z_; }

 private:
  void InitCorners() {
    corners_.clear();
    for (int i = -1; i <= 1; i += 2) {
      for (int j = -1; j <= 1; j += 2) {
        for (int k = -1; k <= 1; k += 2) {
          Vec3d corner(i * half_length_, j * half_width_, k * half_height_);

          // Apply heading rotation (around z-axis)
          corner = Vec3d(corner.x() * cos_heading_ - corner.y() * sin_heading_,
                         corner.x() * sin_heading_ + corner.y() * cos_heading_,
                         corner.z());

          // Apply pitch rotation (around y-axis)
          corner = Vec3d(corner.x() * cos_pitch_ + corner.z() * sin_pitch_,
                         corner.y(),
                         -corner.x() * sin_pitch_ + corner.z() * cos_pitch_);

          // Translate to the center of the box
          corner += center_;

          corners_.push_back(corner);

          // Update dimensions
          max_x_ = std::max(corner.x(), max_x_);
          min_x_ = std::min(corner.x(), min_x_);
          max_y_ = std::max(corner.y(), max_y_);
          min_y_ = std::min(corner.y(), min_y_);
          max_z_ = std::max(corner.z(), max_z_);
          min_z_ = std::min(corner.z(), min_z_);
        }
      }
    }
  }

  // Member variables
  Vec3d center_;
  double length_, width_, height_;
  double half_length_, half_width_, half_height_;
  double heading_, pitch_;
  double cos_heading_, sin_heading_, cos_pitch_, sin_pitch_;
  std::vector<Vec3d> corners_;
  double max_x_ = -std::numeric_limits<double>::infinity();
  double min_x_ = std::numeric_limits<double>::infinity();
  double max_y_ = -std::numeric_limits<double>::infinity();
  double min_y_ = std::numeric_limits<double>::infinity();
  double max_z_ = -std::numeric_limits<double>::infinity();
  double min_z_ = std::numeric_limits<double>::infinity();
};

}  // namespace tracker

#endif  // TRACKER_BOX3D_H_
