#include <ros/ros.h>
#include <std_msgs/String.h>
#include <vector>
#include <time.h>
#include <algorithm>
#include <std_msgs/Int32.h>
#include "yolo_object/sjy_bind.h"
#include "ncnn_ros_cwb/srv_FaceInfo.h"
#include "ncnn_ros_cwb/srv_FoundPerson.h"

using namespace std;

static ros::ServiceClient cliBindPeople;
static ncnn_ros_cwb::srv_FaceInfo srvPeople;
static ros::Publisher chatter_pub;
static ros::Subscriber sub_next_person;

std::vector<std::string> InFO_Name;
string person="";
string drink = "";
string copy_person="";
string copy_drink = "";

#define STATE_BIND_PERSON 0
#define STATE_FIND_PERSON 1
static int nState = STATE_BIND_PERSON;

static string FindPeopleState;
static string NextPersonState;

static void Init_person()
{
    InFO_Name.push_back("Linda");
    InFO_Name.push_back("Jack");
    InFO_Name.push_back("Jone");
    InFO_Name.push_back("Mark");
    InFO_Name.push_back("Mary");
    InFO_Name.push_back("Michael");
    InFO_Name.push_back("James");
    InFO_Name.push_back("Lili");
    InFO_Name.push_back("dawei");
    InFO_Name.push_back("xiaoqiang");
}
void NextPersonCB(const std_msgs::String::ConstPtr& msg)
{
    NextPersonState = msg->data;
    if(NextPersonState == "STATE_NEXT_PERSON")
    {
        nState = STATE_BIND_PERSON;
    }
}
int main(int argc,char** argv)
{
    ros::init(argc,argv,"bind_person");
    ros::NodeHandle nh;
    Init_person();

    chatter_pub = nh.advertise<yolo_object::sjy_bind>("chatter",1000);
    cliBindPeople = nh.serviceClient<ncnn_ros_cwb::srv_FaceInfo>("ASF_BindNameFace");

    sub_next_person = nh.subscribe("/next_person",1000,NextPersonCB);
    ros::Rate r(10);
    while(ros::ok())
    {
        switch (nState)
        {
            case STATE_BIND_PERSON:
            {
                drink = "water";
                person = InFO_Name.front();
                // std::vector<std::string>::iterator del = InFO_Name.begin();
                // InFO_Name.erase(del);

                sleep(10);
                srvPeople.request.name = person;
                srvPeople.request.drink = drink;
                if(cliBindPeople.call(srvPeople))
                {
                    cout<<"-----------"<<endl;
                    copy_person = srvPeople.response.name;
                    copy_drink = srvPeople.response.drink;
                    int age = srvPeople.response.age;
                    int gender = srvPeople.response.gender;
                    ROS_INFO("face info haved binding");
                    cout<<"age is "<<age<<endl;
                    cout<<"gender is "<<gender<<endl;
                    nState = STATE_FIND_PERSON;
                }
                
                break;
            }

            case STATE_FIND_PERSON:
            {
                FindPeopleState = "ACT_FIND_PERSON";
                yolo_object::sjy_bind msgfindpeople;
                msgfindpeople.act = FindPeopleState;
                msgfindpeople.name = person;
                msgfindpeople.drink = drink; 
                chatter_pub.publish(msgfindpeople);

                // nState = STATE_BIND_PERSON;
                break;
            }
            
            default:
                break;
        }
        ros::spinOnce();
        r.sleep();
    }
}
