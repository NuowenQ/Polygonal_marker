#include <ros/ros.h>
#include <mavconn/interface.h>
#include <mavros_msgs/CommandHome.h>
#include <mavros_msgs/State.h>
#include <mavros_msgs/Mavlink.h>
#include <mavros_msgs/mavlink_convert.h>
#include <mavlink/v2.0/mavlink_helpers.h>
#include <sensor_msgs/NavSatFix.h>
#include <std_msgs/Float64.h>
#include "NE_utilities.h"

mavros_msgs::State current_state;
sensor_msgs::NavSatFix NavSatFixed;
float compass_hdg, GYM_OFFSET; //rad

uint8_t system_id = 255;
uint8_t component_id = 1;
uint8_t target_system = 1;

bool packMavlinkMessage(const mavlink::Message& mavMsg, mavros_msgs::Mavlink &rosMsg)
{
	mavlink::mavlink_message_t msg;
	mavlink::MsgMap map(msg);
	mavMsg.serialize(map);
	auto mi = mavMsg.get_message_info();
	
	mavlink::mavlink_status_t *status = mavlink::mavlink_get_channel_status(mavlink::MAVLINK_COMM_0);
	status->flags |= MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
	mavlink::mavlink_finalize_message_buffer(&msg, system_id, component_id, status, mi.min_length, mi.length, mi.crc_extra);
	
	return mavros_msgs::mavlink::convert(msg, rosMsg);
}

void state_cb(const mavros_msgs::State::ConstPtr& msg)
{
	current_state = *msg;
}

//get current GPS position of drone
void GPS_cb(const sensor_msgs::NavSatFix::ConstPtr& NavSatFix_)
{
	NavSatFixed=*NavSatFix_;
}

//get current compass heading of drone
void compass_cb(const std_msgs::Float64ConstPtr& compass_hdg_)
{
	compass_hdg=deg2rad(compass_hdg_->data);
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "set_home_node");
	ros::NodeHandle home_handle;
	
	ros::Subscriber state_sub = home_handle.subscribe<mavros_msgs::State>("mavros/state", 1, state_cb);
	ros::ServiceClient home_set = home_handle.serviceClient<mavros_msgs::CommandHome>("/mavros/cmd/set_home",1);
	ros::Publisher origin_pub = home_handle.advertise<mavros_msgs::Mavlink>("mavlink/to", 1000);
	//ros::Subscriber gps_sub = home_handle.subscribe<sensor_msgs::NavSatFix>("mavros/global_position/global", 1, GPS_cb);
	//ros::Subscriber compass_sub = home_handle.subscribe<std_msgs::Float64>("mavros/global_position/compass_hdg", 1, compass_cb);
	ros::Rate rate(20.0);
	
	while(ros::ok() && current_state.connected)
	{
		ros::spinOnce();
		rate.sleep();
	}
	
	ros::Duration(2.0).sleep();
	
	//if(NavSatFixed.status.status<=0){
	//  ROS_ERROR("No NavSatFix. Exiting....");
	//  return -1;
	//}
	
	double latitude = 24.5253658;
	double longitude = 54.4377712;
	double altitude = 0.0;
	
	mavlink::common::msg::SET_GPS_GLOBAL_ORIGIN originMsg;
	originMsg.latitude = (int32_t)(latitude * 10000000);
	originMsg.longitude = (int32_t)(longitude * 10000000);
	originMsg.altitude = (int32_t)(altitude * 1000);
	originMsg.target_system = target_system;
	
	mavros_msgs::Mavlink packedMsg;
	if(packMavlinkMessage(originMsg, packedMsg)){
		printf("Pack mavlink message SUCCEEDED\n");
		origin_pub.publish(packedMsg);
		ros::spinOnce();
		rate.sleep();
		ros::Duration(2.0).sleep();
	}
	else{
		printf("GPS Pack mavlink message FAILED\n");
	}
	
	
	// ------------------------------------------------------------------------------------------------
	// -----------------------------------  OLD CODE --------------------------------------------------
	//	mavros_msgs::CommandHome set_home_req;
	//	set_home_req.request.current_gps = false; //if set to true probably FCU will get current GPS coords for this
	//	set_home_req.request.latitude = latitude;
	//	set_home_req.request.longitude = longitude;
	//	set_home_req.request.altitude = altitude;
	//	From MavLink --> Yaw angle. NaN to use default heading! Caution here.
	//	set_home_req.request.yaw = GYM_OFFSET;
	
	//	set the orientation of the gym
	//	GYM_OFFSET = 0;
	//	for (int i = 1; i <= 30; ++i) {
	//  	ros::spinOnce();
	//  	rate.sleep();
	//  	GYM_OFFSET += compass_hdg;
	//  	ROS_INFO("current heading%d: %f", i, GYM_OFFSET/i);
	//	}
	//	GYM_OFFSET /= 30;
	//	ROS_INFO("the N' axis offest is facing: %f", rad2deg(GYM_OFFSET));
	//	ros::service::call("/mavros/cmd/set_home", set_home_req);
	//	printf("Result was %d\n", set_home_req.response.result);
	//	------------------------------------------------------------------------------------------------
	//	------------------------------------------------------------------------------------------------
	
	mavlink::common::msg::SET_HOME_POSITION homePosition;
	homePosition.target_system = target_system;
	
	homePosition.latitude = (int32_t)(latitude * 10000000);
	homePosition.longitude = (int32_t)(longitude * 10000000);
	homePosition.altitude = (int32_t)(altitude * 1000);
	homePosition.x = homePosition.y = homePosition.z = 0;
	homePosition.q = {1,0,0,0};
	homePosition.approach_x = homePosition.approach_y = 0;
	homePosition.approach_z = 1;
	
	mavros_msgs::Mavlink packedMsg2;
	if(packMavlinkMessage(homePosition, packedMsg2)){
		printf("Pack mavlink message SUCCEEDED\n");
		origin_pub.publish(packedMsg2);
		ros::spinOnce();
		rate.sleep();
		ros::Duration(2.0).sleep();
	}
	else{
		printf("GPS Pack mavlink message FAILED\n");
	}
	return 0;
}
