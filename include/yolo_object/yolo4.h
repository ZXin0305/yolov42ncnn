#include "net.h"
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#if CV_MAJOR_VERSION >= 3
#include <opencv2/videoio/videoio.hpp>
#include <string>
#endif

#include <vector>
#include <stdio.h>

#define NCNN_PROFILING
#define YOLOV4_TINY //Using yolov4_tiny, if undef, using original yolov4

#ifdef NCNN_PROFILING
#include "benchmark.h"
#endif

using namespace std;
// build the object struct
struct Object
{
    cv::Rect_<float> rect;       //cornor
    int label;
    float prob;                  //probility
    std::string name;
};

class Yolo_Class
{
 public:
   Yolo_Class();
   
   int init_yolov4(ncnn::Net* yolov4, int* target_size);
   int detect_yolov4(const cv::Mat& bgr, std::vector<Object>& objects, int target_size, ncnn::Net* yolov4);
   void draw_objects(const cv::Mat& bgr, const std::vector<Object>& objects, int is_streaming);
private:
   // define path to model & weight params
   string yolov4_param = "/home/zx/robot_ws/src/ncnn/build/examples/yolov4-tiny-opt.param";
   string yolov4_model = "/home/zx/robot_ws/src/ncnn/build/examples/yolov4-tiny-opt.bin";
};