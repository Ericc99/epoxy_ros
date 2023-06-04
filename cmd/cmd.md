# Gmapping ROS Commands

## Mapping

```
terminator -l map
```

```
cd ~/Documents/ros_code
source devel/setup.bash
rosrun tracer_bringup bringup_can2usb.bash
roslaunch tracer_bringup tracer_robot_base.launch
```

```
cd ~/Documents/ros_code
source devel/setup.bash
ls -l /dev/ttyUSB*
sudo chmod 666 /dev/ttyUSB0
roslaunch lslidar_x10_driver lslidar_x10_serial.launch
```

```
cd ~/Documents/ros_code
source devel/setup.bash
ls -l /dev/ttyUSB*
sudo chmod 666 /dev/ttyUSB1
roslaunch fdilink_ahrs ahrs_data.launch
```

```
cd ~/Documents/ros_code
source devel/setup.bash
roslaunch nav_stack amcl_plus_map_server.launch
```

## Navigation

```
terminator -l nav
```

```
cd ~/Documents/ros_code
source devel/setup.bash
rosrun tracer_bringup bringup_can2usb.bash
roslaunch tracer_bringup tracer_robot_base.launch
```

```
cd ~/Documents/ros_code
source devel/setup.bash
ls -l /dev/ttyUSB*
sudo chmod 666 /dev/ttyUSB0
roslaunch lslidar_x10_driver lslidar_x10_serial.launch
```

```
cd ~/Documents/ros_code
source devel/setup.bash
ls -l /dev/ttyUSB*
sudo chmod 666 /dev/ttyUSB1
roslaunch fdilink_ahrs ahrs_data.launch
```

```
cd ~/Documents/ros_code
source devel/setup.bash
roslaunch nav_stack navigation_main.launch
```


## ESP-IDF Commands

```
idf.py set-target esp32s3
idf.py build
ls -l /dev/ttyUSB*
idf.py -p /dev/ttyUSB0 flash
```