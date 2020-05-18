# Copyright 1996-2020 Cyberbotics Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""LED plugin."""

from std_msgs.msg import Int32
from rclpy.time import Time
from rclpy.qos import qos_profile_sensor_data
from .plugin import Plugin


class CameraPluginParams:
    def __init__(
        self,
        timestep=None,
        topic_name=None,
        always_publish=False,
        disable=False
    ):
        self.timestep = timestep
        self.topic_name = topic_name
        self.always_publish = always_publish
        self.disable = disable


class LEDPlugin(Plugin):
    """Webots + ROS2 camera wrapper."""

    def __init__(self, node, device, params=None):
        self._node = node
        self._device = device
        self._last_update = -1
        self._camera_info_plugin = None
        self._image_plugin = None
        self.params = params or CameraPluginParams()

        # Determine default params
        self.params.timestep = self.params.timestep or int(node.robot.getBasicTimeStep())
        self.params.topic_name = self.params.topic_name or self._create_topic_name(device)

        self._led_subscriber = node.create_subscription(
            Int32,
            self.params.topic_name,
            self._callback,
            10
        )

    def _callback(self, msg):
        self._device.set(msg.data)

    def step(self):
        pass
