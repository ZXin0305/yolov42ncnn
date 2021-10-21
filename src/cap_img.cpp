#include <iostream>
#include <ros/ros.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <image_transport/image_transport.h>  
#include <cv_bridge/cv_bridge.h>  
#include <sensor_msgs/image_encodings.h> 
#include <string>
#include <sensor_msgs/Image.h>
#include <std_msgs/String.h>
#include<time.h>

using namespace std;
cv::Mat color_img;
int count_num=0;
void colorCallback(const sensor_msgs::Image::ConstPtr& msg)
{
    try{
        cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
        cv_ptr->image.copyTo(color_img);
	
        cv::imshow("image", color_img);
        cv::imwrite("/home/zx/robot_ws/src/yolo_object/src/imgs/"+to_string(count_num)+".jpg", color_img);
        cv::waitKey(33);
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

    ros::Rate loop_rate(100);
    while(ros::ok())
    {    count_num++;
         loop_rate.sleep();
         ros::spinOnce();
    }

    return 0;
}
