#include "box2d.h"

namespace tracker {

void Box2d::InitCorners() {
  const double dx1 = cos_heading_ * half_length_;
  const double dy1 = sin_heading_ * half_length_;
  const double dx2 = sin_heading_ * half_width_;
  const double dy2 = -cos_heading_ * half_width_;
  corners_.clear();
  corners_.emplace_back(center_.x() + dx1 + dx2, center_.y() + dy1 + dy2);
  corners_.emplace_back(center_.x() + dx1 - dx2, center_.y() + dy1 - dy2);
  corners_.emplace_back(center_.x() - dx1 - dx2, center_.y() - dy1 - dy2);
  corners_.emplace_back(center_.x() - dx1 + dx2, center_.y() - dy1 + dy2);

  for (auto& corner : corners_) {
    max_x_ = std::fmax(corner.x(), max_x_);
    min_x_ = std::fmin(corner.x(), min_x_);
    max_y_ = std::fmax(corner.y(), max_y_);
    min_y_ = std::fmin(corner.y(), min_y_);
  }
}

Box2d::Box2d(const Vec2d& center, const double heading, const double length,
             const double width)
    : center_(center),
      length_(length),
      width_(width),
      half_length_(length / 2.0),
      half_width_(width / 2.0),
      heading_(heading),
      cos_heading_(cos(heading)),
      sin_heading_(sin(heading)) {
  InitCorners();
}

Box2d::Box2d(const Vec2d& point, double heading, double front_length,
             double back_length, double width)
    : length_(front_length + back_length),
      width_(width),
      half_length_(length_ / 2.0),
      half_width_(width / 2.0),
      heading_(heading),
      cos_heading_(cos(heading)),
      sin_heading_(sin(heading)) {
  double delta_length = (front_length - back_length) / 2.0;
  center_ = Vec2d(point.x() + cos_heading_ * delta_length,
                  point.y() + sin_heading_ * delta_length);
  InitCorners();
}

void Box2d::GetAllCorners(std::vector<Vec2d>* const corners) const {
  if (corners == nullptr) {
    return;
  }
  *corners = corners_;
}

const std::vector<Vec2d>& Box2d::GetAllCorners() const { return corners_; }

}  // namespace tracker
