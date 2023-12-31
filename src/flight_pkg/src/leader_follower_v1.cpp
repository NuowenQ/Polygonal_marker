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
#include <mavros_msgs/RCIn.h>
#include <mavros_msgs/PositionTarget.h>
#include <geometry_msgs/PoseArray.h>
#include <mavros_msgs/CommandTOL.h>
#include <time.h>
#include <ros/duration.h>
#include "tf/transform_datatypes.h"
#include "tf_conversions/tf_eigen.h"
#include <tf/LinearMath/Quaternion.h>
#include "NE_utilities.h"

using namespace std;

#define takeoff_alt 2.0

//Set global variables
mavros_msgs::State current_state;
mavros_msgs::RCIn RC_input;
geometry_msgs::PoseStamped Rhombi_input, initial_pose, current_pose, target_pose; //PoseOnGround, Current, Target respectively
double initial_heading, current_heading, target_heading, GYM_OFFSET; //rad

bool experiment_runs;
std_msgs::Bool logger_control, grabbing_control;

double follower_safe_dist = 2.0; //less than this you might be in trouble
double pos_tolerance = 0.25; //when position is considered reached and also value when distances to rhombi are green checked
double rhombi_tolerance = 0.1;
double angle_tolerance = 15; //angular tolerance in degress!!! where no correction from rhombi needed

uint32_t sequence = 0;

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
	tf::quaternionTFToMsg(tf::createQuaternionFromYaw(heading+GYM_OFFSET), target_pose.pose.orientation);
}

// set position to fly to in the gym frame
void setDestination(geometry_msgs::Pose dist_)
{
	target_pose.pose.position.x = current_pose.pose.position.x + dist_.position.x - follower_safe_dist;
	target_pose.pose.position.y = current_pose.pose.position.y + dist_.position.y;
	target_pose.pose.position.z = current_pose.pose.position.z + dist_.position.z;
	/* ROS_INFO("Sequence: %d, From X, Y, Z : %lf, %lf, %lf, to X_T, Y_T, Z_T : %lf, %lf, %lf", sequence, current_pose.pose.position.x,current_pose.pose.position.y,current_pose.pose.position.z, 
	target_pose.pose.position.x,target_pose.pose.position.y, target_pose.pose.position.z); */
}

//stop execution
void start_stop_cb(const std_msgs::Bool input)
{
	ROS_INFO("Start stop data received");
	experiment_runs = input.data;
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
					current_pose.pose.position.x,current_pose.pose.position.y,current_pose.pose.position.z) < pos_tolerance));
}

void rhombi_detect_cb(geometry_msgs::PoseArrayConstPtr input_poses)
{
	//Rotation from Vicon follower to Camera follower: [0 0 1 ; -1 0 0 ; 0 -1 0] converted to quaternion
	tf::Transform V_to_A(tf::Quaternion(-0.5, 0.5, -0.5, 0.5), tf::Vector3(0,0,0));
	//From Camera follower to Marker leader
	tf::Transform A_f_l(tf::Quaternion(0,0,0,1), tf::Vector3(input_poses->poses[0].position.x, input_poses->poses[0].position.y, input_poses->poses[0].position.z));
	//From Vicon follower to Marker leader
	tf::Transform V_to_l = V_to_A * A_f_l;
	tf::poseTFToMsg(V_to_l,Rhombi_input.pose);
	Rhombi_input.header = input_poses->header;
}

void rc_feedback(mavros_msgs::RCInConstPtr RC_input_){
	RC_input = *RC_input_;
}

int main(int argc, char** argv)
{
	ros::init(argc, argv, "leader_follower_v1");
	ros::NodeHandle nh("~");
	//Initialization - Rate
	int RATE = 10;
	nh.getParam("trajectory_rate", RATE);
	ros::Rate loop_rate(RATE);
	
	//Initialization - Object name
	std::string object_name = "~";
	nh.getParam("object_name", object_name);
	
	//Initilization get safe distance
	nh.getParam("follower_distance", follower_safe_dist);
	
	
	// Initialization - Publishers/Subscribers
	ros::ServiceClient set_mode_client = nh.serviceClient<mavros_msgs::SetMode>("/mavros/set_mode");
	ros::Subscriber state_sub = nh.subscribe<mavros_msgs::State>("/mavros/state", 1, state_cb);
	ros::Publisher local_pos_pub = nh.advertise<geometry_msgs::PoseStamped>("/mavros/setpoint_position/local", 1);
	ros::Subscriber currentPos = nh.subscribe<geometry_msgs::PoseStamped>("/mavros/local_position/pose", 1, pose_cb);
	ros::Subscriber experiment_control = nh.subscribe<std_msgs::Bool>("/experiment_control", 1, start_stop_cb);
	ros::Publisher logger_control_ = nh.advertise<std_msgs::Bool>("/logger_control",1);
	ros::Publisher grabbing_control_ = nh.advertise<std_msgs::Bool>("/grabbing_control",1);
	ros::Subscriber rc_input_ = nh.subscribe<mavros_msgs::RCIn>("/mavros/rc/in",1, rc_feedback); 
	
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
	
	if(RC_input.channels[8] > 1000){
		ROS_WARN("SET RC9 to low to continue");
		while(ros::ok() &&  RC_input.channels[8] > 1000){
			ros::spinOnce();
			ros::Duration(0.1).sleep();
		}
	}
	
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
	initial_heading = current_heading;
	
	
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
	
	// Waiting to start experiment
	experiment_runs = false;
	ROS_WARN("Flip the RC switch to start experiment!!!!");
	while(ros::ok() && !experiment_runs){
		if(RC_input.channels[8] > 1000) experiment_runs = true;
		ros::spinOnce();
		loop_rate.sleep();
	}
	target_pose = current_pose;
	target_heading = current_heading;
	
	//Starting all other executables
	ROS_INFO("Publish start all");
	logger_control.data = true;
	grabbing_control.data = true;
	grabbing_control_.publish(grabbing_control);
	logger_control_.publish(logger_control);
	ros::spinOnce();
	loop_rate.sleep();
	ROS_INFO("Done start all");
	
	//Subscribe to start getting reference
	ros::Subscriber rhombi_pose_ = nh.subscribe<geometry_msgs::PoseArray>("/"+ object_name +"/pose_stamped",1, rhombi_detect_cb);
	ros::spinOnce();
	loop_rate.sleep();
	sequence = Rhombi_input.header.seq;
	
	ROS_INFO("Starting following");
	// Flight - Moving between WP
	while(ros::ok() && experiment_runs){
		//if you saw a new rhombi
		if(Rhombi_input.header.seq!=sequence){
			if(abs(Rhombi_input.pose.position.x-follower_safe_dist)>rhombi_tolerance || abs(Rhombi_input.pose.position.y)>rhombi_tolerance ||
			abs(Rhombi_input.pose.position.z)>rhombi_tolerance){
				setDestination(Rhombi_input.pose);
				setHeading(initial_heading);
				target_pose.header = Rhombi_input.header;
				sequence++;
			}
		}
		local_pos_pub.publish(target_pose);
		ros::spinOnce();
		loop_rate.sleep();
		if(RC_input.channels[8] < 1000) experiment_runs = false;
	}
	
	//Stoping all other executables
	ROS_INFO("Publish end all");
	logger_control.data = false;
	grabbing_control.data = false;
	grabbing_control_.publish(grabbing_control);
	logger_control_.publish(logger_control);
	ros::spinOnce();
	loop_rate.sleep();
	ROS_INFO("Done end all");
	
	// Finish - Land
	ros::ServiceClient land_client = nh.serviceClient<mavros_msgs::CommandTOL>("/mavros/cmd/land");
	mavros_msgs::CommandTOL srv_land;
	if (land_client.call(srv_land) && srv_land.response.success) ROS_INFO("Land commensing %d", srv_land.response.success);
	else ROS_ERROR("Landing failed");

	while((current_pose.pose.position.z - initial_pose.pose.position.z)>pos_tolerance)
	{
		ros::spinOnce();
		ros::Duration(0.01).sleep();
	}
	return 0;
}
