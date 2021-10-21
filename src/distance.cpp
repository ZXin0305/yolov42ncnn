#include<iostream>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<librealsense2/rs.hpp>
#include<librealsense2/rsutil.h>
#include <sensor_msgs/image_encodings.h> 
#include <string>
#include <sensor_msgs/Image.h>
#include <std_msgs/String.h>
#include <ros/ros.h>
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/PointStamped.h>
#include <image_transport/image_transport.h>  
#include <cv_bridge/cv_bridge.h>  
#include <Eigen/Geometry>

#include<vector>
#include<algorithm>
#include<numeric>

using namespace cv;
using namespace std;
using namespace rs2;

double fx = 605.3;
double fy = 605.2;
double cx = 326.8;
double cy = 245.2;
double depthScale = 1000.0;

#define ready 0
#define start 1
#define result 2
#define reset 3
int nstate=0;


vector<double>distance_temp;

geometry_msgs::PointStamped two_pose;
void pose_CB(const geometry_msgs::PointStampedConstPtr& msg)
{
    two_pose.point.x = msg->point.x;
    two_pose.point.y = msg->point.y;
    std::cout<<"------------"<<two_pose.point.x<<endl;
    std::cout<<"------------"<<two_pose.point.y<<endl;
}
cv::Mat depth_img;
void depthCallback(const sensor_msgs::Image::ConstPtr& depth_msg)
{
    try
    {
        cv::imshow("depth_view", cv_bridge::toCvShare(depth_msg, sensor_msgs::image_encodings::TYPE_16UC1)->image);
        cv_bridge::CvImagePtr cv_depth_ptr = cv_bridge::toCvCopy(depth_msg,sensor_msgs::image_encodings::TYPE_16UC1);
        depth_img = cv_depth_ptr->image;


        // double distance = depth_img.at<uint16_t>(two_pose.point.x,two_pose.point.y) *0.001;  //距离有问题

        cv::waitKey(1);
        // int pic_type = cv_depth_ptr->image.type();
        // std::cout << "the element type of depth_pic is " << pic_type << std::endl;    
    }
    catch (cv_bridge::Exception& e)
    {
        ROS_ERROR("Could not convert from '%s' to '32fc1'.", depth_msg->encoding.c_str());
    }
}

template<typename T>
T SumVector(vector<T>& vec)
{
    T res = 0;
    for (size_t i=0; i<vec.size(); i++)
    {
        res += vec[i];
    }
    return res;
}

int main(int argc,char** argv)
{
    ros::init(argc,argv,"distance");
    ros::NodeHandle nh;
    ros::Subscriber depth_picture = nh.subscribe("/camera/aligned_depth_to_color/image_raw",10,depthCallback);
    ros::Subscriber two_pose_sub = nh.subscribe("/two_pose",10,pose_CB);
    ros::Rate loop_rate(10);

    while(ros::ok())
    {
        double x= two_pose.point.x;
        double y = two_pose.point.y;
        double distance_sum,distance;
        switch (nstate)
        {
            case 0:
            {
                if(x==0 || y==0)
                    break;
                else
                {
                    distance_temp.push_back(depth_img.at<uint16_t>(x,y)*0.001);
                    nstate =1;
                }
                break;
            }
            case 1:
            {
                cout<<"xxxxxx:"<<x<<endl;
                cout<<"yyyyyy:"<<y<<endl;
                for(int i=1;i<10;i++)
                {
                    double dis_up = 0.0,dis_down=0.0;
                    dis_up = depth_img.at<uint16_t>(x,y+i) *0.001;
                    dis_down = depth_img.at<uint16_t>(x,y-i) *0.001;
                    distance_temp.push_back(dis_up);
                    distance_temp.push_back(dis_down);
                }
                nstate =2;
                break;
            }  
            case 2:
            {
                sort(distance_temp.begin(),distance_temp.end());
                distance_temp.pop_back();
                vector<double>::iterator k = distance_temp.begin();
                distance_temp.erase(k);      
                for(int i=0;i<distance_temp.size();i++)
                {
                    cout<<"distance_temp:"<<distance_temp[i]<<endl;
                }
                vector<double>::iterator it;
                for(it = distance_temp.begin(); it!=distance_temp.end();)
                {
                    if(*it ==0)
                    {
                        it= distance_temp.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
                cout<<"删除0以后:"<<endl;
                for(int j=0;j<distance_temp.size();j++)
                {
                    cout<<"distance_temp:"<<distance_temp[j]<<endl;
                }
                // distance_sum = accumulate(distance_temp.begin(),distance_temp.end(),0);
                distance_sum = SumVector(distance_temp);    //vector求和    
                // 找出现次数最多且不是零
                cout<<"distance_sum:"<<distance_sum<<endl;
                distance = distance_sum/distance_temp.size();
                cout<<"distance"<<distance<<endl;
                Eigen::Vector3d point;
                point[2] = distance;
                point[0] = (two_pose.point.x - cx)*point[2]/fx;
                point[1] = (two_pose.point.y - cy)*point[2]/fy;
                cout<<"相机坐标系下x= "<<point[0]<<endl;
                cout<<"相机坐标系下y= "<<point[1]<<endl;
                cout<<"相机坐标系下z= "<<point[2]<<endl; 

                nstate =3;
                break;
            }
            case 3:
            {
                nstate =0;
                // distance_temp.swap(vector<double>());
                vector<double>().swap(distance_temp);
                break;
            }
            
            default:
                break;
        }     

        loop_rate.sleep();
        ros::spinOnce();
    }
    return 0;
}