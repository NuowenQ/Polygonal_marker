#include <iostream>
#include <fstream>
#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Pose.h>
#include <nav_msgs/Odometry.h>
#include "std_msgs/Float64.h"
#include "std_msgs/Bool.h"
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>
#include <mavros_msgs/PositionTarget.h>
#include <geometry_msgs/Pose2D.h>
#include <mavros_msgs/CommandTOL.h>
#include <time.h>
#include <ros/duration.h>
#include "tf/transform_datatypes.h"
#include "tf_conversions/tf_eigen.h"
#include <tf/LinearMath/Quaternion.h>
#include "NE_utilities.h"

using namespace std;

#define takeoff_alt 2.0
#define rect_size 1.5

#define NUM_POINTS 8

#define RATE 25

//Set global variables
mavros_msgs::State current_state;
geometry_msgs::PoseStamped initial_pose, current_pose, target_pose;
double current_heading; //rad
bool stop_exec=false;
float GYM_OFFSET;

float tollerance = .10;

std::ofstream outfile;

float points[NUM_POINTS][4]={{0.0, rect_size, takeoff_alt, -90},
						{0.0, rect_size, takeoff_alt/2.0, 0},
							{rect_size, rect_size, takeoff_alt/2.0, 0},
								{rect_size, rect_size, takeoff_alt, 90},
									{rect_size, 0.0, takeoff_alt, 90},
										{rect_size, 0.0, takeoff_alt/2.0, -180},
											{0.0, 0.0, takeoff_alt/2.0, -180},
												{0.0, 0.0, takeoff_alt, 0}};

//get orientation of the drone from
void getHeading(geometry_msgs::Pose pose_)
{
  tf::Quaternion q(
    pose_.orientation.x,
    pose_.orientation.y,
    pose_.orientation.z,
    pose_.orientation.w);
    tf::Matrix3x3 m(q);
    double roll, pitch, yaw;
    m.getRPY(roll, pitch, yaw);
    current_heading=yaw;
}

//set orientation of the drone (drone should always be level)
void setHeading(float heading)
{
  //heading = -heading + 90 - GYM_OFFSET;
  float yaw = -heading*M_PI/180 - GYM_OFFSET;
  float pitch = 0;
  float roll = 0;

  float cy = cos(yaw * 0.5);
  float sy = sin(yaw * 0.5);
  float cr = cos(roll * 0.5);
  float sr = sin(roll * 0.5);
  float cp = cos(pitch * 0.5);
  float sp = sin(pitch * 0.5);

  float qw = cy * cr * cp + sy * sr * sp;
  float qx = cy * sr * cp - sy * cr * sp;
  float qy = cy * cr * sp + sy * sr * cp;
  float qz = sy * cr * cp - cy * sr * sp;

  target_pose.pose.orientation.w = qw;
  target_pose.pose.orientation.x = qx;
  target_pose.pose.orientation.y = qy;
  target_pose.pose.orientation.z = qz;
}

// set position to fly to in the gym frame
void setDestination(float x, float y, float z)
{
  float X = x*cos(-GYM_OFFSET) - y*sin(-GYM_OFFSET);
  float Y = x*sin(-GYM_OFFSET) + y*cos(-GYM_OFFSET);
  float Z = z;
  target_pose.pose.position.x = X;
  target_pose.pose.position.y = Y;
  target_pose.pose.position.z = Z;
  ROS_INFO("Destination set to x: %f y: %f z %f", X, Y, Z);
}

//stop execution
void stop_cb(const std_msgs::Bool input)
{
  ROS_INFO("Start stop data received");
  //stop_exec = input.data;
  stop_exec = true;
}

//get armed state
void state_cb(const mavros_msgs::State::ConstPtr& msg)
{
  current_state = *msg;
  bool connected = current_state.connected;
  bool armed = current_state.armed;
}

//get current position of drone
void pose_cb(const geometry_msgs::PoseStamped::ConstPtr& msg)
{
  current_pose = *msg;
  getHeading(current_pose.pose);
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "staple_rect_indoor");
  ros::NodeHandle nh;
  ROS_INFO("Start");
  // the setpoint publishing rate MUST be faster than 2Hz
  ros::Rate rate(RATE);
  ros::ServiceClient set_mode_client = nh.serviceClient<mavros_msgs::SetMode>("/mavros/set_mode");
  ros::Subscriber state_sub = nh.subscribe<mavros_msgs::State>("/mavros/state", 1, state_cb);
  ros::Publisher stop_execution_pub = nh.advertise<std_msgs::Bool>("/stop_execution", 1);
  ros::Publisher set_vel_pub = nh.advertise<mavros_msgs::PositionTarget>("/mavros/setpoint_raw/local", 1);
  ros::Publisher local_pos_pub = nh.advertise<geometry_msgs::PoseStamped>("/mavros/setpoint_position/local", 1);
  ros::Subscriber currentPos = nh.subscribe<geometry_msgs::PoseStamped>("/mavros/local_position/pose", 1, pose_cb);
  ros::Subscriber stop_execution = nh.subscribe<std_msgs::Bool>("/startstop_rectangle", 1, stop_cb);

  // allow the subscribers to initialize
  ROS_INFO("INITIALIZING...");
  for(int i=0; i<100; i++)
  {
    ros::spinOnce();
    ros::Duration(0.01).sleep();
  }
  
  ROS_INFO("CHECKING FCU CONNECTION...");
  // wait for FCU connection
  while (ros::ok() && !current_state.connected)
  {
    ros::spinOnce();
    rate.sleep();
  }

    //  Initialization - Log file
	std::string usr=get_username();
	std::string fileName;
	if(usr!="?") fileName="/home/"+usr+"/projects/nevangeliou_GapterUAV/logs/"+date_filename(); //try to get username, if not save to home folder
	else fileName=date_filename();
	outfile.open(fileName, std::ios::out | std::ios::app);
	if (outfile.fail()){
		throw std::ios_base::failure(std::strerror(errno));
		return -1;
	}
	
	// Make sure write fails with exception if something is wrong
	outfile.exceptions(outfile.exceptions() | std::ios::failbit | std::ifstream::badbit);
	outfile << "time_elapsed(Sec)"<< ";" 
				<< "Gapter waypoint (X)" << ";" << "Gapter.waypoint (Y)" << ";" << "Gapter.waypoint (Z)" << ";" 
						<< "Gapter Position(x)" << ";" <<  "y" << ";" <<  "z" << std::endl;
  
  //Check for and enable guided mode
  mavros_msgs::SetMode guided_set_mode;
  guided_set_mode.request.custom_mode = "GUIDED";
  //while(current_state.mode != "GUIDED" && (ros::Time::now() - last_request > ros::Duration(5.0)))
  while(current_state.mode != "GUIDED")
  {
	if( set_mode_client.call(guided_set_mode) && guided_set_mode.response.mode_sent){ ROS_INFO("GUIDED requested");}
	else{ROS_ERROR("Failed to set GUIDED mode");}
    ros::spinOnce();
    ros::Duration(0.05).sleep();
  }

  //set the orientation of the gym
  GYM_OFFSET = 0;
  int i=0;
  while (ros::ok() && i<30) {
  //for (int i = 1; i <= 30; ++i) {
    ros::spinOnce();
    ros::Duration(0.1).sleep();
    GYM_OFFSET += current_heading;
    i++;
  }
  GYM_OFFSET /= i;
  ROS_INFO("the N' axis is facing: %f", GYM_OFFSET);

  // arming
  ros::ServiceClient arming_client_i = nh.serviceClient<mavros_msgs::CommandBool>("/mavros/cmd/arming");
  mavros_msgs::CommandBool srv_arm_i;
  srv_arm_i.request.value = true;
  if (arming_client_i.call(srv_arm_i) && srv_arm_i.response.success)
    ROS_INFO("ARM sent %d", srv_arm_i.response.success);
  else
  {
    ROS_ERROR("Failed arming");
    return -1;
  }
  for(int i=0; i<200; i++)
  {
    ros::spinOnce();
    ros::Duration(0.01).sleep();
  }

  //request takeoff
  ros::ServiceClient takeoff_cl = nh.serviceClient<mavros_msgs::CommandTOL>("/mavros/cmd/takeoff");
  mavros_msgs::CommandTOL srv_takeoff;
  srv_takeoff.request.altitude = takeoff_alt;
  int stop=0;
  while (!srv_takeoff.response.success && stop<5){
	  ros::spinOnce();
	  rate.sleep();
	  if(takeoff_cl.call(srv_takeoff)){
		  ROS_INFO("Take off service called successfully");
		  ros::spinOnce();
		  rate.sleep();
		  if(srv_takeoff.response.success){
				ROS_INFO("takeoff success");
				stop=10;
		  }
		  else{	
				ROS_INFO("takeoff fail. Send success: %d", srv_takeoff.response.success);
				stop++;
		  }
	  }
	  else{
				ROS_INFO("Could not make the takeoff service call"); stop++;
	  }
	  ros::spinOnce();
	  ros::Duration(1).sleep();
  }

  // Hover for 5 seconds
  ROS_INFO("HOVERING...");
  for(int i=0; i<500; i++)
  {
    ros::spinOnce();
    ros::Duration(0.01).sleep();
  }
  initial_pose = current_pose;
  //Switch positions
  int wp=0;
  double waypoint_time_init = ros::Time::now().toSec();
  double dMag = 0;
  ROS_INFO("Waypoint %d",wp);
  stop_exec=true;
  while(ros::ok() && !stop_exec && kbhit()==0 && wp<NUM_POINTS){
        setHeading(points[wp][3]);
        setDestination(points[wp][0], points[wp][1], points[wp][2]);
        for (int i = 20*RATE; ros::ok() && i > 0 ; --i){
        local_pos_pub.publish(target_pose);
        dMag = get_distance3D(target_pose.pose.position.x,target_pose.pose.position.y,target_pose.pose.position.z,current_pose.pose.position.x,current_pose.pose.position.y,current_pose.pose.position.z);
        //cout << "Mag: " <<  dMag << endl;
        outfile << ros::Time::now().toSec()-waypoint_time_init << ";" 
            << target_pose.pose.position.x << ";" << target_pose.pose.position.y << ";" << target_pose.pose.position.z << ";" 
                    << current_pose.pose.position.x << ";" <<  current_pose.pose.position.y << ";" <<  current_pose.pose.position.z << std::endl;
        if( dMag < tollerance) break;
        ros::spinOnce();
        rate.sleep();
        if(i == 1) ROS_INFO("Failed to reach destination. Stepping to next task.");
        }
        wp++;
        ROS_INFO("Waypoint %d",wp);
    }
  outfile.close();
  //while(ros::ok() && dMag<tollerance){
  //      ROS_INFO("Waypoint landing");
  //      setHeading(points[NUM_POINTS-1][3]);
  //      setDestination(points[NUM_POINTS-1][0], points[NUM_POINTS-1][1], points[NUM_POINTS-1][2]);
  //      for (int i = 20*RATE; ros::ok() && i > 0 ; --i){
  //      local_pos_pub.publish(target_pose);
  //      dMag = get_distance3D(target_pose.pose.position.x,target_pose.pose.position.y,target_pose.pose.position.z,current_pose.pose.position.x,current_pose.pose.position.y,current_pose.pose.position.z);
  //      if( dMag < tollerance) break;
 //       ros::spinOnce();
   //     rate.sleep();
  //      if(i == 1) ROS_INFO("Failed to reach destination. Stepping to next task.");
  //      }
 // }
  //land
  ros::ServiceClient land_client = nh.serviceClient<mavros_msgs::CommandTOL>("/mavros/cmd/land");
  mavros_msgs::CommandTOL srv_land;
  if (land_client.call(srv_land) && srv_land.response.success) ROS_INFO("land sent %d", srv_land.response.success);
  else ROS_ERROR("Landing failed");
  
  ROS_INFO("FINISHING...");
  for(int i=0; i<1000; i++)
  {
    ros::spinOnce();
    ros::Duration(0.01).sleep();
  }
  return 0;
}
