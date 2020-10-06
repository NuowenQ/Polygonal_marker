// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <iomanip> // setprecision
#include <sstream> // stringstream
#include <time.h>

#include <ros/ros.h>
#include <diagnostic_updater/diagnostic_updater.h>
#include <diagnostic_updater/update_functions.h>
#include <tf/tf.h>
#include <tf/transform_broadcaster.h>
#include <geometry_msgs/TransformStamped.h>
#include <geometry_msgs/PoseStamped.h>
#include <iostream>
#include <tf/transform_listener.h>
#include <std_msgs/UInt32.h>
#include <fstream>
#include <thread>
#include <ios>
#include <cstring>
#include <chrono> //ctime is included
#include <vector>
#include <stdlib.h>
#include <inttypes.h>
#include <sstream>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

#define PORT     9400 
#define MAXLINE 1024 
#define num_objects 1

double transX = 0;
double transY = 0;
double transZ = 0;
double quatX = 0;
double quatY = 0;
double quatZ = 0;
double quatW = 0;

geometry_msgs::Pose Gapter_1; //other drone

std::string object_names[num_objects]={"Gapter_1"};

std::stringstream stream;

void GapterPoseFeedback(geometry_msgs::PoseStamped VulcanPose_){
    Gapter_1=VulcanPose_.pose;
    transX = Gapter_1.position.x; 
    transY = Gapter_1.position.y; 
    transZ = Gapter_1.position.z;
    quatW = Gapter_1.orientation.w; 
    quatX = Gapter_1.orientation.x;
    quatY = Gapter_1.orientation.y;
    quatZ = Gapter_1.orientation.z;

    stream << std::fixed << std::setprecision(16) << transX << ", " << transY << ", " << transZ << ", " << quatX << ", " << quatY << ", " << quatZ << ", " << quatW;

    printf("%.16lf\n", transX);
    printf("%.16lf\n", transY);
    printf("%.16lf\n", transZ);
    printf("%.16lf\n", quatW);
    printf("%.16lf\n", quatX);
    printf("%.16lf\n", quatY);
    printf("%.16lf\n", quatZ);

}

double randfrom(double min, double max)
{
  double range = (max - min);
  double div = RAND_MAX / range;
  return min + (rand() / div);
}

// Driver code 
int main(int argc, char **argv) { 
  if((sizeof(object_names)/sizeof(std::string))!=num_objects){
    ROS_INFO("Number of object names not equal to predefined num_objects. Exiting....");
    return -1;
  }
    //srand (time (NULL));
    ros::init(argc, argv, "client");
    int sockfd; 
    char buffer[MAXLINE]; 

    ros::NodeHandle n("~");

    ros::Subscriber Gapter_1_measurements = n.subscribe("/vrpn_client_node/"+ object_names[0] + "/pose", 1, &GapterPoseFeedback);

    ros::Rate loop_rate(2);

    clock_t start = 0, end = 0;
    double roundtt;


    /*
    transX = -3.1546934314123493;
    transY = 1.684092431234211;
    transZ = -9.6840431432439211;

    quatX = -1.18404334343429211;
    quatY = 5.2844233243209211;
    quatZ = -3.58404313413449211;
    quatW = 8.9847646776709211;
    */

    unsigned int len; 

    long long msg_counter = 0;

    const char delim[2] = ",";

    int other_pose_num = 7;

    double other_pose[other_pose_num]; 

    int counter = 0;

    char *token;

    char *tmp_ptr;

    stream << std::fixed << std::setprecision(16) << transX << ", " << transY << ", " << transZ << ", " << quatX << ", " << quatY << ", " << quatZ << ", " << quatW;

    struct sockaddr_in servaddr; 

    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
      perror("socket creation failed"); 
      exit(EXIT_FAILURE); 
    } 

    memset(&servaddr, 0, sizeof(servaddr)); 

    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    //servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.100"); 

    len = sizeof(servaddr);

    while (ros::ok())
    {

      std::string s = stream.str();

      int n_size = s.length(); 

      char message[n_size+1];

      strcpy(message, s.c_str()); 

      bzero(buffer, MAXLINE); 
      bzero(other_pose, other_pose_num); 

      sendto(sockfd, (const char *)message, strlen(message), 
          MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
              sizeof(servaddr)); 

      printf("Message %lld sent.\n", msg_counter);  
            
      start = clock();

      n_size = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                  MSG_DONTWAIT, (struct sockaddr *) &servaddr, 
                  &len); 


      if(n_size < 0)
      {
        printf("error: %s\n", strerror(errno));
        n_size = 0;
        continue;
      }

      msg_counter++;

      end = clock();

      buffer[n_size] = '\0'; 

      token = strtok(buffer, delim);

      while( token != NULL ) 
      {
        other_pose[counter] = strtod(token, &tmp_ptr);
        counter++;
        token = strtok(NULL, delim);
      }

      for(int i = 0; i < other_pose_num; i++)
      {
        //printf("%.16lf\n", other_pose[i]);
      }

      printf("\nMessage %lld received.\n", msg_counter);  

      roundtt = ((double) (end - start)) / CLOCKS_PER_SEC;

      printf("\nTime between pose datagram sent and response received: %f\n", roundtt*1000);
      printf("\n-----------------------------------------------------------\n\n");

      counter = 0;

      ros::spinOnce();

      loop_rate.sleep();

    }

    printf("\nEnd datagram send\n\n");

    close(sockfd); 

    return 0; 
} 
