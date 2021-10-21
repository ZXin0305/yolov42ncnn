#include "yolo4.h"
#include <iostream>
#include <ros/ros.h>
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/PointStamped.h>
#include <image_transport/image_transport.h>  
#include <cv_bridge/cv_bridge.h>  
// #include <sound_play/SoundRequest.h>
#include <sensor_msgs/image_encodings.h> 
#include <string>
#include <sensor_msgs/Image.h>
#include <std_msgs/String.h>
#include<time.h>

using namespace std;

geometry_msgs::PointStamped two_pose;
ncnn::Net yolov4;
Yolo_Class yolo_type;
std::vector<Object> objects;
int target_size = 0;
int is_streaming = 1;
cv::Mat color_img;
int object_x=0;
int object_y=0;

int object_z = 0;

int object_width=0;
int object_height=0;

// callback function to subscribe
// color image, transform it to opencv
// finally process it
void colorCallback(const sensor_msgs::Image::ConstPtr& msg)
{
    try{
        // double t1 = static_cast<double>(cv::getTickCount());
        cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
        cv_ptr->image.copyTo(color_img);
        // yolo_type.detect_yolov4(color_img, objects, target_size, &yolov4);
        // yolo_type.draw_objects(color_img, objects, is_streaming);
        // double t2 = static_cast<double>(cv::getTickCount());
        // cout<<(t2-t1) / cv::getTickFrequency()<<endl;
        
        cv::imshow("image", color_img);
        cv::waitKey(1);
    }
    catch(cv_bridge::Exception& ex){
        ROS_ERROR("cv_bridge exception: %s",ex.what());
        return; 
    }
}



int main(int argc, char** argv)
{

    ros::init(argc,argv,"yolo_ros");
    ros::NodeHandle n;
    ros::Subscriber color_picture=n.subscribe("/camera/color/image_raw",10,colorCallback);
    // ros::Subscriber color_picture=n.subscribe("/camera/depth/image_rect_raw",10,colorCallback);
    ros::Publisher two_pose_pub=n.advertise<geometry_msgs::PointStamped>("/two_pose", 1000);
    
    // the class-function to load model parameters
    int ret = yolo_type.init_yolov4(&yolov4, &target_size);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to load model or param, error %d", ret);
        return -1;
    }
    ros::Rate loop_rate(100);
    int count_num=0;
    std::vector<int> object_container;
    while(ros::ok())
    {
        for(int i=0;i<objects.size();i++)
        {

            if (objects[i].name=="chips")
            {
                object_x=objects[i].rect.x;
                object_y=objects[i].rect.y;


                object_width=objects[i].rect.width;
                object_height=objects[i].rect.height;
                std::cout<<"x:"<<object_x<<" "<<"y:"<<object_y<<" "<<"width:"<<object_width<<" "<<"height:"<<object_height<<" "<<std::endl;
                two_pose.point.x=object_x+int((1.0*object_width)/2);
                two_pose.point.y=object_y+int((1.0*object_height)/2);


                two_pose_pub.publish(two_pose);
            }
        }
        
         loop_rate.sleep();
         ros::spinOnce();
    }

    return 0;
}
