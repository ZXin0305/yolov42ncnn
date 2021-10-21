#include <ros/ros.h>
#include <iostream>
#include <std_msgs/String.h>
#include <vector>
#include <move_base_msgs/MoveBaseAction.h>
#include <time.h>
#include "yolo_object/sjy_bind.h"
#include "ncnn_ros_cwb/srv_FaceInfo.h"
#include "ncnn_ros_cwb/srv_FoundPerson.h"

using namespace std;

#define STATE_BIND_PERSON 0
#define STATE_FIND_PERSON 1
#define STATE_FOUND_PERSON 2

int nState = STATE_FIND_PERSON;

static string Sub_nState;  
static string Sub_Targets_1;
static string Sub_Targets_2;
static string Target_person;

static ros::Subscriber sub;
static ros::Publisher pub_next_person;

static ros::ServiceClient cliFindPeople;
static ncnn_ros_cwb::srv_FoundPerson srvFindPeople;

string name ="";
string drink ="";
string person="";
int positionError = 0;
float confidenceLevel = 0;

void chatterCallback(const yolo_object::sjy_bind msg)
{
    Sub_nState = msg.act.c_str();
    Sub_Targets_1 = msg.name.c_str();
    Sub_Targets_2 = msg.drink.c_str();
    if(Sub_nState =="ACT_FIND_PERSON")
    {
        if(Sub_Targets_1  !=""  && Sub_Targets_2 !="")
        {
            Target_person = Sub_Targets_1.c_str();
        }
    }
}

int main(int argc,char** argv)
{
    ros::init(argc,argv,"find_person");
    ros::NodeHandle n;

    cliFindPeople = n.serviceClient<ncnn_ros_cwb::srv_FoundPerson>("/ASF_FoundPersonByName");
    sub = n.subscribe("chatter",1000,chatterCallback);
    pub_next_person = n.advertise<std_msgs::String>("/next_person",1000);

    ros::Rate r(10);

    while (ros::ok())
    {
        switch (nState)
        {
            case STATE_FIND_PERSON:
            {
                if(Target_person.c_str() != "")     
                {
                    srvFindPeople.request.name = Target_person.c_str();
                    if(cliFindPeople.call(srvFindPeople))
                    {
                        name = srvFindPeople.response.name;
                        positionError = srvFindPeople.response.positionError;
                        confidenceLevel= srvFindPeople.response.confidenceLevel;    
                        if(confidenceLevel > 18)    
                        {
                            if(abs(positionError) <135)
                            {
                                nState = STATE_FOUND_PERSON;
                            }   
                        } 
                        else
                        {
                            ROS_INFO("failed find people");
                        }           
                    }
                }   
                break;        
            }
            case STATE_FOUND_PERSON:
            {
                ROS_INFO("Find person");
                cout<<"I have find :"<<name;
                sleep(10);
                
                std_msgs::String next_person_state;
                next_person_state.data = "STATE_NEXT_PERSON";
                pub_next_person.publish(next_person_state);
                nState = STATE_FIND_PERSON;
                break;
            }

        }
        ros::spinOnce();
        r.sleep();
    }
}