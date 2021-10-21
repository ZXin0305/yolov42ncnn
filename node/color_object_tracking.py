#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import cv_bridge
import geometry_msgs
import rospy
import numpy as np
import time
import cv2
from cv_bridge import CvBridge,CvBridgeError
from sensor_msgs.msg import Image
from geometry_msgs.msg import PointStamped

class ColorObjectTracking:
    def __init__(self):
        #初始化，创建cv_bridge
        self.bridge = CvBridge()
        self.pose_pub = rospy.Publisher("/two_pose",PointStamped,queue_size=1)   #发布者发布获得的坐标值
        #设置颜色阈值
        self.yellow_lower=np.array([26,43,46])
        self.yellow_upper=np.array([34,255,255])
        #接收者接收图像信息
        self.image_sub = rospy.Subscriber("/camera/color/image_raw",Image,self.colorCallback,queue_size=1)

        #图像回调函数
    def colorCallback(self,data):
        try:
            cv_image = self.bridge.imgmsg_to_cv2(data,"bgr8")
            frame = np.array(cv_image,dtype=np.uint8)

            frame=cv2.GaussianBlur(frame,(5,5),0)                #高斯模糊    
            hsv=cv2.cvtColor(frame,cv2.COLOR_BGR2HSV)           #转HSV     
            mask=cv2.inRange(hsv,self.yellow_lower,self.yellow_upper)   

            #形态学运算
            mask=cv2.erode(mask,None,iterations=2)
            mask=cv2.dilate(mask,None,iterations=2)
            mask=cv2.GaussianBlur(mask,(3,3),0)
            res=cv2.bitwise_and(frame,frame,mask=mask)               #与运算
            cnts=cv2.findContours(mask.copy(),cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)[-2]

            two_pose = PointStamped()
            if len(cnts)>0:
                cnt = max (cnts,key=cv2.contourArea)
                (x,y),radius=cv2.minEnclosingCircle(cnt)
                cv2.circle(frame,(int(x),int(y)),int(radius)*2,(255,0,255),2)      
                two_pose.point.x = x
                two_pose.point.y = y

            
            cv2.imshow('capture',frame)
            cv2.waitKey(3)
            rospy.loginfo("黄色物体坐标：")
            print(two_pose.point.x)
            print(two_pose.point.x)
            self.pose_pub.publish(two_pose)   

        except CvBridgeError as e:
                pass
        

                   

if __name__ =="__main__":
    try:
        rospy.init_node("color_object",anonymous=True)
        ColorObjectTracking()

        rospy.spin()
    except rospy.ROSInterruptException:
        pass 