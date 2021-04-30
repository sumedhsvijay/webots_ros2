// Copyright 1996-2021 Cyberbotics Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// webots_ros2_cpp
#include <webots_ros2_cpp/math.hpp>

// ros
#include <geometry_msgs/msg/quaternion.hpp>
#include <geometry_msgs/msg/vector3.hpp>

// std
#include <cmath>

// Copy from Webots-ROS1
namespace wb_ros2_interface {

void matrixToQuaternion(const double *matrix, 
    geometry_msgs::msg::Quaternion &q) {
  if (matrix[0] == matrix[4] && matrix[0] == matrix[8] && matrix[0] == 1.0) {
    // exception
    q.w = 1.0;
    q.x = 0.0;
    q.y = 0.0;
    q.z = 0.0;
    return;
  }

  double s = 2.0;
  double invS = 1.0;
  const double trace = matrix[0] + matrix[4] + matrix[8];
  if (trace >= 0.0) {
    s *= sqrt(trace + 1);
    invS = 1.0 / s;
    q.w = 0.25 * s;
    q.x = (matrix[7] - matrix[5]) * invS;
    q.y = (matrix[2] - matrix[6]) * invS;
    q.z = (matrix[3] - matrix[1]) * invS;
    return;
  }

  if (matrix[0] > matrix[4] && matrix[0] > matrix[8]) {
    // matrix[0] is larger than max(M(4), M(8))
    s *= sqrt(1.0 + matrix[0] - matrix[4] - matrix[8]);
    invS = 1.0 / s;
    q.w = (matrix[7] - matrix[5]) * invS;
    q.x = 0.25 * s;
    q.y = (matrix[1] + matrix[3]) * invS;
    q.z = (matrix[6] + matrix[2]) * invS;
    return;
  }

  if (matrix[4] > matrix[8]) {
    // matrix[4] is the largest
    s *= sqrt(1.0 + matrix[4] - matrix[8] - matrix[0]);  // s = 4y
    invS = 1.0 / s;
    q.w = (matrix[2] - matrix[6]) * invS;
    q.x = (matrix[1] + matrix[3]) * invS;
    q.y = 0.25 * s;
    q.z = (matrix[5] + matrix[7]) * invS;
    return;
  }

  // else matrix[8] is the largest
  s *= sqrt(1.0 + matrix[8] - matrix[0] - matrix[4]);  // s = 4z
  invS = 1.0 / s;
  q.w = (matrix[3] - matrix[1]) * invS;
  q.x = (matrix[6] + matrix[2]) * invS;
  q.y = (matrix[5] + matrix[7]) * invS;
  q.z = 0.25 * s;
}

void axisAngleToQuaternion(const double *axisAngle, 
    geometry_msgs::msg::Quaternion &q) {
  const double halfAngle = 0.5 * axisAngle[3];
  const double sinusHalfAngle = sin(halfAngle);
  q.w = cos(halfAngle);
  q.x = axisAngle[0] * sinusHalfAngle;
  q.y = axisAngle[1] * sinusHalfAngle;
  q.z = axisAngle[2] * sinusHalfAngle;
}

void quaternionToAxisAngle(const geometry_msgs::msg::Quaternion &q, 
    double *axisAngle) {
  // if q.w > 1, acos will return nan
  // if this actually happens we should normalize the quaternion here
  axisAngle[3] = 2.0 * acos(q.w);
  if (axisAngle[3] < 0.0001) {
    axisAngle[0] = 0.0;
    axisAngle[1] = 1.0;
    axisAngle[2] = 0.0;
    axisAngle[3] = 0.0;
  }
  // normalise axes
  const double inv = 1.0 / sqrt(q.x * q.x + q.y * q.y + q.z * q.z);
  axisAngle[0] = q.x * inv;
  axisAngle[1] = q.y * inv;
  axisAngle[2] = q.z * inv;
}

}  // wb_ros2_interface