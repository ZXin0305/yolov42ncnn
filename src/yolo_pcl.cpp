//#include "yolo4.h"
#include <iostream>
#include <ros/ros.h>
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/PointStamped.h>
#include <image_transport/image_transport.h>  
#include <cv_bridge/cv_bridge.h>  
#include <sound_play/SoundRequest.h>
#include <sensor_msgs/image_encodings.h> 
#include <string>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/PointCloud2.h>
#include <std_msgs/String.h>
#include "MergeSG.h"

#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/passthrough.h>
#include <pcl/conversions.h>

#include <pcl/search/search.h>
#include <pcl/search/kdtree.h>
#include <pcl/segmentation/extract_clusters.h>

using namespace std;

geometry_msgs::PointStamped two_pose;
ncnn::Net yolov4;
Yolo_Class yolo_type;
std::vector<Object> objects;
std::vector<Cluster> cluster;
MergeSG mergesg;
int target_size = 0;
int is_streaming = 1;
cv::Mat color_img;
int object_x=0;
int object_y=0;
int object_width=0;
int object_height=0;


void colorCallback(const sensor_msgs::Image::ConstPtr& msg)
{
    cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    color_img = cv_ptr -> image;
    yolo_type.detect_yolov4(color_img, objects, target_size, &yolov4);
    yolo_type.draw_objects(color_img, objects, is_streaming);
    cv::imshow("image", color_img);
    cv::waitKey(1);
}

void pointCallback(const sensor_msgs::PointCloud2::ConstPtr& cloud_msg)
{
   

    // pcl::PointCloud<pcl::PointXYZRGB> cloud;
    // pcl::PointCloud<pcl::PointXYZRGB> new_cloud;
    // pcl::fromROSMsg(*cloud_msg, cloud);
    // mergesg.extract(objects,cloud.makeShared(),cluster);


    pcl::PCLPointCloud2* cloud = new pcl::PCLPointCloud2; 
    pcl::PCLPointCloud2ConstPtr cloudPtr(cloud);
    pcl::PCLPointCloud2 cloud_filtered;
    pcl_conversions::toPCL(*cloud_msg, *cloud);

    pcl::PassThrough<pcl::PCLPointCloud2> pass;
    // build the filter
    pass.setInputCloud (cloudPtr);
    pass.setFilterFieldName ("z");
    pass.setFilterLimits (0.0, 1.3);
    pass.setKeepOrganized(true);
    // apply filter
    pass.filter (cloud_filtered);

    pcl::PointCloud<pcl::PointXYZRGB> point_cloud;
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr point_cloudPtr(new pcl::PointCloud<pcl::PointXYZRGB>);
    pcl::fromPCLPointCloud2( cloud_filtered, point_cloud);
    pcl::copyPointCloud(point_cloud, *point_cloudPtr);

    mergesg.extract(objects,point_cloudPtr,cluster);
}




int main(int argc, char** argv)
{

    ros::init(argc,argv,"yolo_ros");
    ros::NodeHandle n;
    ros::Subscriber color_picture=n.subscribe("/kinect2/qhd/image_color_rect",10,colorCallback);
    ros::Publisher two_pose_pub=n.advertise<geometry_msgs::PointStamped>("/two_pose", 1000);
    ros::Subscriber point_cloud=n.subscribe("/kinect2/sd/points",10,pointCallback);
    int ret=yolo_type.init_yolov4(&yolov4, &target_size);
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
        // for(int i=0;i<objects.size();i++)
        // {

        //     if (objects[i].name=="chips")
        //     {
        //         object_x=objects[i].rect.x;
        //         object_y=objects[i].rect.y;
        //         object_width=objects[i].rect.width;
        //         object_height=objects[i].rect.height;
        //         std::cout<<"x:"<<object_x<<" "<<"y:"<<object_y<<" "<<"width:"<<object_width<<" "<<"height:"<<object_height<<" "<<std::endl;
        //         two_pose.point.x=object_x+int((1.0*object_width)/2);
        //         two_pose.point.y=object_y+int((1.0*object_height)/2);
        //         two_pose_pub.publish(two_pose);
        //     }
        // }
        
         loop_rate.sleep();
         ros::spinOnce();
    }

    return 0;
}
