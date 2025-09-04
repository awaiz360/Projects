#include <Eigen/Dense>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

class BBox3DTo2DConverter {
 public:
  static const int YAW_AXIS = 1;

  static std::vector<Eigen::MatrixXf> corners(
      const Eigen::MatrixXf& bboxes_3d) {
    if (bboxes_3d.size() == 0) {
      // return std::vector<Eigen::MatrixXf> zeros(9);
    }

    Eigen::MatrixXf dims = bboxes_3d.block(
        0, 3, bboxes_3d.rows(), 3);  // Select all rows, columns 3 to 6

    Eigen::MatrixXf corners_norm(8, 3);
    corners_norm << -0.5, -1, -0.5, -0.5, -1, 0.5, -0.5, 0, 0.5, -0.5, 0, -0.5,
        0.5, -1, -0.5, 0.5, -1, 0.5, 0.5, 0, 0.5, 0.5, 0, -0.5;

    int num_corners = dims.rows();
    std::vector<Eigen::MatrixXf> corners(num_corners);

    for (int i = 0; i < num_corners; ++i) {
      corners[i] = (dims.row(i).array().replicate(corners_norm.rows(), 1) *
                    corners_norm.array())
                       .matrix();
    }

    Eigen::VectorXf yaw_angles = bboxes_3d.col(6);

    std::vector<Eigen::MatrixXf> corners_rotated =
        rotation_3d_in_axis(corners, yaw_angles, YAW_AXIS);

    // Select all rows, columns 1 to 3
    Eigen::MatrixXf selected_bboxes_3d =
        bboxes_3d.block(0, 0, bboxes_3d.rows(), 3);

    // Vector to store the results
    std::vector<Eigen::MatrixXf> corners_translated(num_corners);

    // Loop through each matrix in corners_rotated
    for (int i = 0; i < num_corners; ++i) {
      // Create a temporary matrix for the current result
      Eigen::MatrixXf current_translation = Eigen::MatrixXf::Zero(8, 3);

      // Perform element-wise addition of each row of selected_bboxes_3d with
      // corners_rotated[i]
      for (int j = 0; j < 8; ++j) {
        current_translation.row(j) =
            selected_bboxes_3d.row(i) + corners_rotated[i].row(j);
      }

      // Assign the current result to corners_translated[i]
      corners_translated[i] = current_translation;
    }

    return corners_translated;
  }

  static Eigen::MatrixXf points_cam2img(const Eigen::MatrixXf& points_3d,
                                        const Eigen::MatrixXf& proj_mat,
                                        bool with_depth = false) {
    // Create a new matrix of size (rowsMumOfPoints4d, 4) and set the first
    // three columns to be the same as points_3d and the fourth column to be all
    // ones
    int rows_of_poins_4d = points_3d.rows();
    Eigen::MatrixXf points_4d(rows_of_poins_4d, 4);
    points_4d << points_3d, Eigen::MatrixXf::Ones(rows_of_poins_4d, 1);

    Eigen::MatrixXf points_2d = points_4d * proj_mat.transpose();

    // Select all rows and first two columns
    Eigen::MatrixXf selected_points =
        points_2d.block(0, 0, points_2d.rows(), 2);

    // Select all rows and third column
    Eigen::MatrixXf selected_column =
        points_2d.block(0, 2, points_2d.rows(), 1);

    // Perform element-wise division for the first column
    Eigen::MatrixXf point_2d_res_col1 =
        selected_points.col(0).array() / selected_column.col(0).array();

    // Perform element-wise division for the second column
    Eigen::MatrixXf point_2d_res_col2 =
        selected_points.col(1).array() / selected_column.col(0).array();

    // Combine the results into a single matrix
    Eigen::MatrixXf point_2d_res(selected_points.rows(), 2);
    point_2d_res.col(0) = point_2d_res_col1;
    point_2d_res.col(1) = point_2d_res_col2;

    return point_2d_res;
  }

  static std::vector<Eigen::MatrixXf> rotation_3d_in_axis(
      std::vector<Eigen::MatrixXf>& points, Eigen::VectorXf& angles,
      int axis = 0, bool return_mat = false, bool clockwise = false) {
    bool batch_free = (points[0].cols() == 2);
    int size = points.size();

    // Initialize rotated_points to hold the result
    std::vector<Eigen::MatrixXf> rotated_points(size);

    int num_dims = points[0].cols();
    int num_points = angles.size();

    // Handle the case when yaw_angles is a scalar (float)
    if (angles.size() == 1) {
      angles.resize(num_points);
      angles.fill(angles(0));  // Fill all elements with the scalar value
    }

    Eigen::ArrayXf rot_sin = angles.array().sin();
    Eigen::ArrayXf rot_cos = angles.array().cos();

    std::vector<Eigen::MatrixXf> rot_mat_T(size);
    if (num_dims == 3) {
      if (axis == 1 || axis == -2) {
        for (int i = 0; i < size; ++i) {
          rot_mat_T[i].resize(3, 3);
          rot_mat_T[i] << rot_cos[i], 0, -rot_sin[i], 0, 1, 0, rot_sin[i], 0,
              rot_cos[i];
        }
      } else if (axis == 2 || axis == -1) {
        for (int i = 0; i < size; ++i) {
          rot_mat_T[i].resize(3, 3);
          rot_mat_T[i] << rot_cos[i], rot_sin[i], 0, -rot_sin[i], rot_cos[i], 0,
              0, 0, 1;
        }
      } else if (axis == 0 || axis == -3) {
        for (int i = 0; i < size; ++i) {
          rot_mat_T[i].resize(3, 3);
          rot_mat_T[i] << 1, 0, 0, 0, rot_cos[i], rot_sin[i], 0, -rot_sin[i],
              rot_cos[i];
        }
      } else {
        throw std::invalid_argument(
            "Invalid axis. Must be in [-3, -2, -1, 0, 1, 2].");
      }
    } else if (num_dims == 2) {
      for (int i = 0; i < size; ++i) {
        rot_mat_T[i].resize(2, 2);
        rot_mat_T[i] << rot_cos[i], rot_sin[i], -rot_sin[i], rot_cos[i];
      }
    }

    if (clockwise) {
      // rot_mat_T[i].transposeInPlace();
    }

    for (int i = 0; i < num_points; ++i) {
      rotated_points[i] =
          points[i] *
          rot_mat_T[i];  // Multiply each (8, 3) matrix by the transpose of the
                         // corresponding (3, 3) matrix
    }

    if (return_mat) {
      return rotated_points;
    } else {
      return rotated_points;
    }
  }

  static std::vector<Eigen::MatrixXf> proj_camera_bbox3d_to_img(
      std::vector<Eigen::MatrixXf>& corners_3d,
      const Eigen::MatrixXf& cam2img) {
    int num_corners_3d = corners_3d.size();
    const int rows_per_matrix = 8;
    const int cols = 3;
    int mult_num_corners_3d_cols = num_corners_3d * rows_per_matrix;
    // Concatenate the corners_3d into a single matrix of size
    // mult_num_corners_3d_cols x 3
    Eigen::MatrixXf points_3d(mult_num_corners_3d_cols, 3);
    int index = 0;
    for (int i = 0; i < num_corners_3d; ++i) {
      points_3d.block(index, 0, rows_per_matrix, cols) = corners_3d[i];
      index += rows_per_matrix;
    }

    // Project 3D points to image coordinates
    Eigen::MatrixXf uv_origin = points_cam2img(points_3d, cam2img);

    // Perform the operation uv_origin - 1 and round the result
    uv_origin = (uv_origin.array() - 1).round().matrix();

    // Reshape uv_origin into num_corners_3d(num of bboxes) matrices of size 8x2
    std::vector<Eigen::MatrixXf> imgfov_pts_2d;

    int rows_total = uv_origin.rows();

    for (int i = 0; i < num_corners_3d; ++i) {
      // Calculate the start and end indices for each matrix
      int start_row = i * rows_per_matrix;
      int end_row = std::min((i + 1) * rows_per_matrix, rows_total);

      // Extract the rows for this matrix
      Eigen::MatrixXf matrix_i =
          uv_origin.block(start_row, 0, end_row - start_row, 2);

      // Add this matrix to the vector of matrices
      imgfov_pts_2d.push_back(matrix_i);
    }

    return imgfov_pts_2d;
  }

  static Eigen::MatrixXf calculateMinMaxPlotRectangles(
      cv::Mat& image, std::vector<Eigen::MatrixXf>& corners_2d) {
    const int num_corners_2d = corners_2d.size();
    const int rows =
        corners_2d[0]
            .rows();  // Assuming all corners_2d have the same number of rows
    const int cols =
        corners_2d[0]
            .cols();  // Assuming all corners_2d have the same number of columns

    Eigen::MatrixXf min_max(num_corners_2d, 4);

    for (int i = 0; i < num_corners_2d; ++i) {
      Eigen::MatrixXf& matrix = corners_2d[i];

      float min_x = matrix.col(0).minCoeff();
      float max_x = matrix.col(0).maxCoeff();
      float min_y = matrix.col(1).minCoeff();
      float max_y = matrix.col(1).maxCoeff();

      min_max(i, 0) = min_x;
      min_max(i, 1) = max_x;
      min_max(i, 2) = min_y;
      min_max(i, 3) = max_y;

      // Create rectangle
      cv::Rect rect(min_x, min_y, max_x - min_x, max_y - min_y);

      // Draw rectangle on image
      cv::rectangle(image, rect, cv::Scalar(0, 255, 0), 2);
    }
    return min_max;
  }
};
