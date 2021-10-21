#include "yolo4.h"
#include <iostream>

Yolo_Class yolo_type;

int main(int argc, char** argv)
{
     
    cv::Mat frame;
    std::vector<Object> objects;

    cv::VideoCapture cap;

    ncnn::Net yolov4;
    
   
    const char* devicepath;

    int target_size = 0;
    int is_streaming = 0;


    int ret=yolo_type.init_yolov4(&yolov4, &target_size);

    if (ret != 0)
    {
        fprintf(stderr, "Failed to load model or param, error %d", ret);
        return -1;
    }

    std::string picture_path="/home/sjy/yolo_ncnn/src/ncnn/build/examples/000021.jpg";
    devicepath=picture_path.c_str();
    frame = cv::imread(picture_path, 1);
   
    yolo_type.detect_yolov4(frame, objects, target_size, &yolov4); //Create an extractor and run detection

    yolo_type.draw_objects(frame, objects, is_streaming); //Draw detection results on opencv image

    return 0;
}
