#! /usr/bin/env python3
import sys
sys.path.remove('/opt/ros/kinetic/lib/python2.7/dist-packages')
import cv2
import rospy
from sensor_msgs import Image
from cv_bridge import CvBridge

class CameraTopic():
    def __init__(self, cam_topic):
        self.img = None
        self.cam_topic = cam_topic
        self.cv_bridge = CvBridge()
        self.img_sub = rospy.Subscriber(self.cam_topic, Image, self.imgCallBack)
    
    def imgCallBack(self, msg):
        try:
            self.img = self.cv_bridge.imgmsg_to_cv2(msg, "bgr8")
            size = self.img.shape
            #cv2.imshow("img", self.img)
            #cv2.waitKey(33)
        except CvBridgeError as e:
            ROS_INFO("cv_bridge error ..")

def main():
    rospy.init('test_node', anonymous=True)
    cam_topic = "/camera/color/image_raw"
    camCapture = CameraTopic()

    while not rospy.is_shutdown():
        rospy.spin()

if __name__ == "__main__":
    main()
