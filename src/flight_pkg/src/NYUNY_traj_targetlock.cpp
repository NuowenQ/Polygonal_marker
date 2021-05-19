#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
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

#define takeoff_alt 1.6
#define motion_dist 1.5

//Set global variables
mavros_msgs::State current_state;
geometry_msgs::PoseStamped initial_pose, current_pose, target_pose; //PoseOnGround, Current, Target respectively
double current_heading, target_heading, GYM_OFFSET; //rad

bool experiment_runs = false;
std_msgs::Bool logger_control, grabbing_control;

float pos_tollerance = .20; //m
float yaw_tollerance = 5; //degrees
const float time_tolerance = 15; //seconds to reach every waypoint

//get orientation of the drone from
void getHeading(geometry_msgs::Pose pose_)
{
	tf::Pose posed;
	tf::poseMsgToTF(pose_, posed);
	current_heading = tf::getYaw(posed.getRotation());
}

//set orientation of the drone (drone should always be level) - input is rad
void setHeading(double heading)
{
	target_heading = -(heading+GYM_OFFSET);
	tf::quaternionTFToMsg(tf::createQuaternionFromYaw(target_heading), target_pose.pose.orientation);
}

// set position to fly to in the gym frame
void setDestination(double x, double y, double z)
{
	target_pose.pose.position.x = initial_pose.pose.position.x + x*cos(-GYM_OFFSET) - y*sin(-GYM_OFFSET);
	target_pose.pose.position.y = initial_pose.pose.position.y + x*sin(-GYM_OFFSET) + y*cos(-GYM_OFFSET);
	target_pose.pose.position.z = initial_pose.pose.position.z + z;
	//ROS_INFO("Destination set to x: %f y: %f z %f", target_pose.pose.position.x, target_pose.pose.position.y, target_pose.pose.position.z);
}

//stop execution
void start_stop_cb(const std_msgs::Bool input)
{
	ROS_INFO("Start stop data received");
	experiment_runs = input.data;
	//stop_exec = true;
}

//get armed state
void state_cb(const mavros_msgs::State::ConstPtr& msg)
{
	current_state = *msg;
}

//get current position of drone
void pose_cb(const geometry_msgs::PoseStamped::ConstPtr& msg)
{
	current_pose = *msg;
	getHeading(current_pose.pose);
}

// Check if position and orientation defined by target are reached
bool check_wp_reached()
{	
	return ((get_distance3D(target_pose.pose.position.x,target_pose.pose.position.y,target_pose.pose.position.z,
					current_pose.pose.position.x,current_pose.pose.position.y,current_pose.pose.position.z) < pos_tollerance));
						//&&	(abs(rad2deg(target_heading-current_heading)) < yaw_tollerance)
}

int main(int argc, char** argv)
{
	ros::init(argc, argv, "NYU_traj_targetlock");
	ros::NodeHandle nh("~");
	//Initialization - Rate
	int RATE = 10;
	nh.getParam("trajectory_rate", RATE);
	ros::Rate loop_rate(RATE);
	
	//Initialization - Object name
	std::string object_name = "~";
	nh.getParam("object_name", object_name);
	
	// Initialization - Publishers/Subscribers
	ros::ServiceClient set_mode_client = nh.serviceClient<mavros_msgs::SetMode>("/mavros/set_mode");
	ros::Subscriber state_sub = nh.subscribe<mavros_msgs::State>("/mavros/state", 1, state_cb);
	ros::Publisher local_pos_pub = nh.advertise<geometry_msgs::PoseStamped>("/mavros/setpoint_position/local", 1);
	ros::Subscriber currentPos = nh.subscribe<geometry_msgs::PoseStamped>("/mavros/local_position/pose", 1, pose_cb);
	ros::Subscriber experiment_control = nh.subscribe<std_msgs::Bool>("/experiment_control", 1, start_stop_cb);
	ros::Publisher logger_control_ = nh.advertise<std_msgs::Bool>("/logger_control",1);
	ros::Publisher grabbing_control_ = nh.advertise<std_msgs::Bool>("/grabbing_control",1);
	
	ROS_INFO("INITIALIZING...");
	for(int i=0; i<10; i++)
	{
		ros::spinOnce();
		ros::Duration(0.01).sleep();
	}
	
	// Initialization - Check FCU connection
	while(ros::ok() && !current_state.connected)
	{
		ros::spinOnce();
		loop_rate.sleep();
	}
	
	//  Initialization - Path file
	std::ifstream pathfile;
	std::string fileName;
    nh.getParam("target_path", fileName);
	if(fileName.empty()){
		ROS_ERROR("Filename %s not found. Exiting...",fileName.c_str());
		ros::shutdown();
		return -1;
	}
	else{
		pathfile.open(fileName, std::ios::in);
		ROS_INFO("Filename %s ",fileName.c_str());
		if (pathfile.fail()){
			throw std::ios_base::failure(std::strerror(errno));
			return -1;
		}
		else{
			ROS_INFO("Reading pathFile...");
		}
	}
	std::vector<std::vector<double>> path_points;
	std::string line, item;
	std::vector<double> nums;
	while (std::getline(pathfile, line))
    {
		std::istringstream ss;
		ss.str(line);
		while (std::getline(ss, item, '\t')) {
			nums.push_back(std::stod(item));
		}
		path_points.push_back(nums);
		nums.clear();
	}
	
	ROS_INFO("Configuring YAW offsets");
	//Read distance from target and offset distance from target w.r.t drone frame (X forward Z up)
	geometry_msgs::PoseStampedConstPtr CCD_Pos_sub= ros::topic::waitForMessage<geometry_msgs::PoseStamped>("/vrpn_client_node/"+ object_name + "/pose", nh);
	geometry_msgs::PoseStampedConstPtr TV_Pos_sub= ros::topic::waitForMessage<geometry_msgs::PoseStamped>("/vrpn_client_node/Monitor/pose", nh);
	double target_distance = get_distance3D(0,CCD_Pos_sub->pose.position.y,0, 0, TV_Pos_sub->pose.position.y, 0); //Get obj dist from Vicon
	double offset_drift= 0.0;
	nh.getParam("offset_drift", offset_drift);
	ROS_INFO("YAW offsets configured for offset %lf", offset_drift);
	
	// Pre-flight - Get the initial orientation of the drone
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
	GYM_OFFSET = 0;
	ROS_INFO("the N' axis is facing: %f degrees", rad2deg(GYM_OFFSET));
	
	// Pre-flight - Check for and enable guided mode
	mavros_msgs::SetMode guided_set_mode;
	guided_set_mode.request.custom_mode = "GUIDED";
	while(current_state.mode != "GUIDED")
	{
		if( set_mode_client.call(guided_set_mode) && guided_set_mode.response.mode_sent){ ROS_INFO("GUIDED requested");}
		else{ROS_ERROR("Failed to set GUIDED mode");}
		ros::spinOnce();
		ros::Duration(0.05).sleep();
	}
	
	// Pre-flight - Arming
	ros::ServiceClient arming_client_i = nh.serviceClient<mavros_msgs::CommandBool>("/mavros/cmd/arming");
	mavros_msgs::CommandBool srv_arm_i;
	srv_arm_i.request.value = true;
	if (arming_client_i.call(srv_arm_i) && srv_arm_i.response.success){
		ROS_INFO("ARM sent %d", srv_arm_i.response.success);
		for(int i=0; i<200; i++)
		{
			ros::spinOnce();
			ros::Duration(0.01).sleep();
		}
	}
	else
	{
		ROS_ERROR("Failed arming");
		return -1;
	}
	initial_pose = current_pose;
	target_heading = current_heading;
	
	// Pre-flight - Request takeoff
	ros::ServiceClient takeoff_cl = nh.serviceClient<mavros_msgs::CommandTOL>("/mavros/cmd/takeoff");
	mavros_msgs::CommandTOL srv_takeoff;
	srv_takeoff.request.altitude = takeoff_alt;
	int stop=0;
	while (!srv_takeoff.response.success && stop<5){
		ros::spinOnce();
		loop_rate.sleep();
		if(takeoff_cl.call(srv_takeoff)){
			ROS_INFO("Take off service called successfully");
			ros::spinOnce();
			loop_rate.sleep();
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

	// Flight - Waiting to reach takeoff alt
	while(current_pose.pose.position.z < (initial_pose.pose.position.z + takeoff_alt-0.1))
	{
		ros::spinOnce();
		loop_rate.sleep();
	}
	
	// Initialization - Check FCU connection
	while(ros::ok() && !experiment_runs)
	{
		ros::spinOnce();
		loop_rate.sleep();
	}

	// Flight - Moving between WP
	int wp=0;
	double waypoint_time_init = ros::Time::now().toSec();	
	setHeading(deg2rad(path_points[wp][3]));
	setDestination(path_points[wp][0],path_points[wp][1],path_points[wp][2]-0.2);
	ROS_INFO("New waypoint %d, %lf, %lf, %lf, %lf", wp, target_pose.pose.position.x,target_pose.pose.position.y,target_pose.pose.position.z,-rad2deg(target_heading));
	while(ros::ok() && experiment_runs && wp<path_points.size()){
		while (experiment_runs && !check_wp_reached() && (ros::Time::now().toSec()-waypoint_time_init) < time_tolerance){
			double X_for_yaw = target_distance - abs(current_pose.pose.position.x); //always positive
			double Y_for_yaw = current_pose.pose.position.y - offset_drift;
			setHeading(atan2(Y_for_yaw, X_for_yaw)); //x,y are inversed in my case because drone X forward
			local_pos_pub.publish(target_pose);
			ros::spinOnce();
			loop_rate.sleep();
		}
		if((ros::Time::now().toSec()-waypoint_time_init)>time_tolerance){
			ROS_INFO("Waypoint %d time exceeded!Distance: %lf",wp, get_distance3D(
					target_pose.pose.position.x,target_pose.pose.position.y,target_pose.pose.position.z,
					current_pose.pose.position.x, current_pose.pose.position.y, current_pose.pose.position.z));
		}
		waypoint_time_init = ros::Time::now().toSec();
		wp++;
		if(wp<path_points.size()){
		//setHeading(deg2rad(path_points[wp][3]));
		double X_for_yaw = target_distance - abs(current_pose.pose.position.x); //always positive
		double Y_for_yaw = current_pose.pose.position.y - offset_drift;
		double the_heading = rad2deg(atan2(Y_for_yaw, X_for_yaw));
		setHeading(deg2rad(the_heading)); //x,y are inversed in my case because drone X forward
		ROS_INFO("Current X: %lf, target_distance: %lf, Current Y: %lf, offset_drift: %lf",
				current_pose.pose.position.x, target_distance, current_pose.pose.position.y, offset_drift);
		ROS_INFO("Heading degrees: %lf", the_heading);
		setDestination(path_points[wp][0],path_points[wp][1],path_points[wp][2]-0.2);
		ROS_INFO("New waypoint %d, %lf, %lf, %lf, %lf", wp, target_pose.pose.position.x,target_pose.pose.position.y,target_pose.pose.position.z,-rad2deg(target_heading));
		}
	}
	
	// Flight - ending logger
	ROS_INFO("Publish end all");
	logger_control.data = false;
	grabbing_control.data = false;
	grabbing_control_.publish(grabbing_control);
	ros::spinOnce();
	ros::Duration(1.0).sleep();
	logger_control_.publish(logger_control);
	ros::spinOnce();
	loop_rate.sleep();
	ROS_INFO("Done end all");
	
	// Finish - Land
	ros::ServiceClient land_client = nh.serviceClient<mavros_msgs::CommandTOL>("/mavros/cmd/land");
	mavros_msgs::CommandTOL srv_land;
	if (land_client.call(srv_land) && srv_land.response.success) ROS_INFO("Land commensing %d", srv_land.response.success);
	else ROS_ERROR("Landing failed");

	while((current_pose.pose.position.z - initial_pose.pose.position.z)>pos_tollerance)
	{
		ros::spinOnce();
		ros::Duration(0.01).sleep();
	}
	return 0;
}
