#include "iou.h"

#include "Eigen/Eigen"
#include "clipper2/clipper.h"  // Note the change here

using namespace Clipper2Lib;

// Function to calculate IoU
double CalculateIoU(const RectangleYaw& rect1, const RectangleYaw& rect2) {
  PathD path1, path2;  // Use PathD for double precision
  Eigen::Rotation2Dd rot1(rect1.yaw), rot2(rect2.yaw);

  for (int i = 0; i < 4; ++i) {
    double x = ((i == 0 || i == 3) ? rect1.width : -rect1.width) / 2;
    double y = (i < 2 ? rect1.length : -rect1.length) / 2;
    Eigen::Vector2d point1 = Eigen::Vector2d(rect1.center_x, rect1.center_y) +
                             rot1 * Eigen::Vector2d(x, y);
    path1.push_back(PointD(point1.x(), point1.y()));  // Use PointD

    x = ((i == 0 || i == 3) ? rect2.width : -rect2.width) / 2;
    y = (i < 2 ? rect2.length : -rect2.length) / 2;
    Eigen::Vector2d point2 = Eigen::Vector2d(rect2.center_x, rect2.center_y) +
                             rot2 * Eigen::Vector2d(x, y);
    path2.push_back(PointD(point2.x(), point2.y()));  // Use PointD
  }

  // Create Clipper object and add paths (now within a separate scope)
  {
    Clipper2Lib::ClipperD clipper;
    Clipper2Lib::PathsD subjects{path1};
    Clipper2Lib::PathsD clips{path2};
    clipper.AddSubject(subjects);
    clipper.AddClip(clips);

    // Execute clipper to get the intersection
    Clipper2Lib::PathsD solution;
    clipper.Execute(ClipType::Intersection, FillRule::EvenOdd, solution);

    if (solution.empty()) {
      return 0.0;  // No intersection
    } else {
      double intersectionArea = 0.0;
      double unionArea = Area(path1) + Area(path2);

      for (const PathD& sol : solution) {
        intersectionArea += Area(sol);
      }

      unionArea -= intersectionArea;
      // Calculate and return IoU
      return intersectionArea / unionArea;
    }
  }
}
