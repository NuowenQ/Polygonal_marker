/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2010, UC Regents
 *  Copyright (c) 2011, Markus Achtelik, ETH Zurich, Autonomous Systems Lab (modifications)
 * 	Copyright (C) 2018, Nikolaos Evangeliou, NYU Abu Dhabi, Robotics Lab (modifications)
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the University of California nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>
#include <cstring>
#include <chrono> //ctime is included
#include <vector>
#include <stdlib.h>
#include <inttypes.h>
#include <sstream>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <fstream>
#include "NE_utilities.h"
#include <mavros_msgs/RCOut.h>

#define RATE 25 //Hz

std::string object_name ("gapter1");
std::ofstream outfile;
ros::Time exp_init;
double elapsed = 0.0;

geometry_msgs::TwistStamped FCUvel;
geometry_msgs::PoseStamped  FCUPose;

mavros_msgs::RCOut RCout;

const uint8_t offset_hours = 8;
const uint16_t seconds_in_hour = 3600;

void vrpnPoseFeedback(geometry_msgs::PoseStampedConstPtr vrpnPose_){
    //Matlab style in orintation requires w first, in ROS w is last. Here we use Matlab
    //elapsed = ros::Time::now().toSec()-exp_init.toSec();
	if(RCout.channels.size()!=0){
    outfile << vrpnPose_->header.stamp.toSec() -  offset_hours*seconds_in_hour << ";"
        << vrpnPose_->pose.position.x << ";" << vrpnPose_->pose.position.y << ";" << vrpnPose_->pose.position.z << ";" 
        << vrpnPose_->pose.orientation.w << ";" <<  vrpnPose_->pose.orientation.x << ";" <<  vrpnPose_->pose.orientation.y << ";" <<  vrpnPose_->pose.orientation.z << ";" 
		
		<< FCUvel.header.stamp.toSec() << ";" 
		<< FCUvel.twist.linear.x << ";" << FCUvel.twist.linear.y << ";" << FCUvel.twist.linear.z << ";"
		<< FCUvel.twist.angular.x << ";" << FCUvel.twist.angular.y << ";" << FCUvel.twist.angular.z << ";"
		
		<< FCUPose.header.stamp.toSec() << ";"
        << FCUPose.pose.position.x << ";" << FCUPose.pose.position.y << ";" << FCUPose.pose.position.z << ";"
        << FCUPose.pose.orientation.w << ";" << FCUPose.pose.orientation.x << ";" << FCUPose.pose.orientation.y << ";" << FCUPose.pose.orientation.z << ";" 
		
        <<RCout.header.stamp.toSec() << ";" ;
		for (uint8_t i=0; i<4; i++){
			if(i<3) outfile << int (RCout.channels[i]) << ";" ;
			else outfile << int (RCout.channels[i]) << std::endl;
		}
	}
}

void FCUvelocity_cb(geometry_msgs::TwistStampedConstPtr FCUVel_){
    //Matlab style in orintation requires w first, in ROS w is last. Here we use Matlab
    FCUvel = *FCUVel_;
}

void FCUPose_cb(geometry_msgs::PoseStampedConstPtr FCUPose_){
    //Matlab style in orintation requires w first, in ROS w is last. Here we use Matlab
    FCUPose = *FCUPose_;
}

void RCout_cb(mavros_msgs::RCOutConstPtr RCout_){
	RCout = *RCout_;
}

int main(int argc, char** argv)
{
	ros::init(argc, argv, "vrpn_log");
	ros::NodeHandle nh("~");
	ros::Subscriber vrpn_measurements = nh.subscribe<geometry_msgs::PoseStamped>("/vrpn_client_node/"+ object_name + "/pose", 1, &vrpnPoseFeedback);
	ros::Subscriber currentVel = nh.subscribe<geometry_msgs::TwistStamped>("/mavros/local_position/velocity_local", 1, &FCUvelocity_cb);
	ros::Subscriber currentPos = nh.subscribe<geometry_msgs::PoseStamped>("/mavros/local_position/pose", 1, &FCUPose_cb);
	ros::Subscriber rcout = nh.subscribe<mavros_msgs::RCOut>("/mavros/rc/out", 1 , &RCout_cb);
	
	ros::Rate loop_rate(RATE);
    
    //   Initialization - Log file
    std::string usr=get_username();
    std::string fileName;
    //try to get username, if not save to home folder
    if(usr!="?") fileName="/home/"+usr+"/projects/NE_UAScodes/logs/INERTIALlog_"+date_filename();
    else fileName=date_filename();
    outfile.open(fileName, std::ios::out | std::ios::app);
    if (outfile.fail()){
        throw std::ios_base::failure(std::strerror(errno));
        return -1;
      }
    // 	make sure write fails with exception if something is wrong
    outfile.exceptions(outfile.exceptions() | std::ios::failbit | std::ifstream::badbit);
	
	outfile << std::fixed << std::setprecision(10);
	
    outfile << "time_elapsed(Sec)"<< ";" 
    << "Vicon position (x)" << ";" << "y" << ";" << " z" << ";" 
    << "Vicon orientation (o_w)" << ";" <<  "o_x" << ";" <<  "o_y" << ";" <<  "o_z" << ";"
	
	<< "FCU timestamp vel" << ";" 
    << "FCU linear velocity (v_x)" << ";" <<  "v_y" << ";" <<  "v_z" << ";"
	<< "FCU angular velocity (w_x)" << ";" <<  "w_y" << ";" <<  "w_z" << ";"
	
	<< "FCU timestamp position" << ";" 
    << "FCU local position (x)" << ";" <<  "y" << ";" <<  "z" << ";"
    << "FCU local orientation (o_w)" << ";" <<  "o_x" << ";" <<  "o_y" << ";" <<  "o_z" << ";"
	
	<< "FCU timestamp RCout" << ";"
	<<  "RC1" << ";" <<  "RC2" << ";" <<  "RC3" << ";" <<  "RC4" << std::endl;
	
    exp_init= ros::Time::now();
	
    //MAIN Loop
    while(kbhit()==0 && ros::ok())
	{
		//Spin until user stops program
		ros::spinOnce();
		loop_rate.sleep();
	}
    ROS_INFO("Saved data to: %s", fileName.c_str());
    outfile.close();
	return 0;
}
