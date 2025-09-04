#ifndef IOU_H
#define IOU_H
#include "clipper2/clipper.h"

struct RectangleYaw {
  double center_x;
  double center_y;
  double width;
  double length;
  double yaw;  // In radians
};

// Function to calculate Intersection over Union (IoU)
double CalculateIoU(const RectangleYaw& rect1, const RectangleYaw& rect2);

#endif  // IOU_H
