This repository comprises some example sources to illustrate fundamental techniques in ROS and robotics programming in general. Please follow the wiki for more details.

### urdf

execute following commands to run the scara in rviz

terminal 1: \
`roscore`

terminal 2: \
`cd urdf` \
`xacro scara.urdf.xacro > scara.urdf` \
`rosparam load scara.urdf robot_description`

terminal 3: \
`rosrun joint_state_publisher_gui joint_state_publisher_gui`

terminal 4: \
`rosrun robot_state_publisher robot_state_publisher`

terminal 5 (or use 2 again): \
`rosrun rviz rviz`
