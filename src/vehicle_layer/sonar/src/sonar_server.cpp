#include "ros/ros.h"
#include "include/sensorapi.h"

bool sonar(vehicle_layer::sonar_data::Request &req, vehicle_layer::sonar_data::Response &res){
  struct sonarData sd;
  int fd = openSensorBoard("/dev/ttyUSB0");

  if(!strcmp(req.req,"data")){
    getSonarData(fd, &sd);
    res.vectorXYZ[0] = sd.vectorX;
    res.vectorXYZ[1] = sd.vectorY;
    res.vectorXYZ[2] = sd.vectorZ;
    res.status = sd.status;
    res.range = sd.range;
    res.timestamp = sd.timeStampSec;
    res.sample_number = sd.timeStampUSec;
  }
  else{
    ROS_ERROR("Request contents wrong");
    return false;
  }
  return true;
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "sonar_server_node");
  ros::NodeHandle n;

  ros::ServiceServer service = n.advertiseService("sonar_data", sonar);
  ROS_INFO("Ready to get sonar data.");
  ros::spin();

  return 0;
}