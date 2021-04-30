#include <webots_ros2_cpp/devices/Ros2Lidar.hpp>

#include <sensor_msgs/msg/point_field.hpp>

namespace webots_ros2
{
  Ros2Lidar::Ros2Lidar(webots_ros2::WebotsNode *node, std::map<std::string, std::string> &parameters) : mNode(node)
  {
    mLidar = mNode->robot()->getLidar(parameters["name"]);

    // Parameters
    mTopicName = parameters.count("topicName") ? parameters["topicName"] : "/" + mLidar->getName();
    mPublishTimestep = parameters.count("updateRate") ? 1.0 / atof(parameters["updateRate"].c_str()) : 0;
    mAlwaysOn = parameters.count("alwaysOn") ? (parameters["alwaysOn"] == "true") : false;
    mFrameName = parameters.count("frameName") ? parameters["frameName"] : mLidar->getName();

    // Calcualte timestep
    mPublishTimestepSyncedMs = mNode->robot()->getBasicTimeStep();
    while (mPublishTimestepSyncedMs / 1000.0 <= mPublishTimestep)
      mPublishTimestepSyncedMs *= 2;

    // Laser publisher
    if (mLidar->getNumberOfLayers() == 1)
    {
      mLaserPublisher = mNode->create_publisher<sensor_msgs::msg::LaserScan>(mTopicName, rclcpp::SensorDataQoS().reliable());
      const int resolution = mLidar->getHorizontalResolution();
      mLaserMessage.header.frame_id = mFrameName;
      mLaserMessage.angle_min = -mLidar->getFov() / 2.0;
      mLaserMessage.angle_max = mLidar->getFov() / 2.0;
      mLaserMessage.angle_increment = mLidar->getFov() / resolution;
      mLaserMessage.time_increment = (double)mLidar->getSamplingPeriod() / (1000.0 * resolution);
      mLaserMessage.scan_time = (double)mLidar->getSamplingPeriod() / 1000.0;
      mLaserMessage.range_min = mLidar->getMinRange();
      mLaserMessage.range_max = mLidar->getMaxRange();
      mLaserMessage.ranges.resize(resolution);
    }

    // Point cloud publisher
    mPointCloudPublisher = mNode->create_publisher<sensor_msgs::msg::PointCloud2>(mTopicName + "/point_cloud", rclcpp::SensorDataQoS().reliable());
    mPointCloudMessage.header.frame_id = mFrameName;
    mPointCloudMessage.height = 1;
    mPointCloudMessage.point_step = 20;
    mPointCloudMessage.is_dense = false;
    mPointCloudMessage.fields.resize(3);
    mPointCloudMessage.fields[0].name = "x";
    mPointCloudMessage.fields[0].datatype = sensor_msgs::msg::PointField::FLOAT32;
    mPointCloudMessage.fields[0].count = 1;
    mPointCloudMessage.fields[0].offset = 0;
    mPointCloudMessage.fields[1].name = "y";
    mPointCloudMessage.fields[1].datatype = sensor_msgs::msg::PointField::FLOAT32;
    mPointCloudMessage.fields[1].count = 1;
    mPointCloudMessage.fields[1].offset = 4;
    mPointCloudMessage.fields[2].name = "z";
    mPointCloudMessage.fields[2].datatype = sensor_msgs::msg::PointField::FLOAT32;
    mPointCloudMessage.fields[2].count = 1;
    mPointCloudMessage.fields[2].offset = 8;
    mPointCloudMessage.is_bigendian = false;

    mLastUpdate = mNode->robot()->getTime();
    mIsEnabled = false;
  }

  void Ros2Lidar::step()
  {
    // Update only if needed
    if (mNode->robot()->getTime() - mLastUpdate < mPublishTimestep)
      return;
    mLastUpdate = mNode->robot()->getTime();

    // Enable/Disable sensor
    const bool shouldBeEnabled = mAlwaysOn || mLaserPublisher->get_subscription_count() > 0 || mPointCloudPublisher->get_subscription_count() > 0;
    if (shouldBeEnabled != mIsEnabled)
    {
      if (shouldBeEnabled)
        mLidar->enable(mPublishTimestepSyncedMs);
      else
        mLidar->disable();
      mIsEnabled = shouldBeEnabled;
    }

    // Publish data
    if (mLaserPublisher != nullptr && (mLaserPublisher->get_subscription_count() > 0 || mAlwaysOn))
      publishLaserScan();
    if (mPointCloudPublisher->get_subscription_count() > 0 || mAlwaysOn)
    {
      mLidar->enablePointCloud();
      publishPointCloud();
    }
    else
      mLidar->disablePointCloud();
  }

  void Ros2Lidar::publishPointCloud()
  {
    auto data = mLidar->getPointCloud();
    if (data)
    {
      mPointCloudMessage.header.stamp = rclcpp::Clock().now();

      mPointCloudMessage.width = mLidar->getNumberOfPoints();
      mPointCloudMessage.row_step = 20 * mLidar->getNumberOfPoints();
      if (mPointCloudMessage.data.size() != mPointCloudMessage.row_step * mPointCloudMessage.height)
        mPointCloudMessage.data.resize(mPointCloudMessage.row_step * mPointCloudMessage.height);

      memcpy(mPointCloudMessage.data.data(), data, mPointCloudMessage.row_step * mPointCloudMessage.height);
      mPointCloudPublisher->publish(mPointCloudMessage);
    }
  }

  void Ros2Lidar::publishLaserScan()
  {
    auto rangeImage = mLidar->getLayerRangeImage(0);
    if (rangeImage)
    {
      memcpy(mLaserMessage.ranges.data(), rangeImage, mLaserMessage.ranges.size() * sizeof(float));
      mLaserMessage.header.stamp = rclcpp::Clock().now();
      mLaserPublisher->publish(mLaserMessage);
    }
  }

} // end namespace webots_ros2
