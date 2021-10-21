#include "yolo4.h"

Yolo_Class::Yolo_Class(){

}

// ncnn::Net:实例化网络
int Yolo_Class::init_yolov4(ncnn::Net* yolov4, int* target_size)
{
    /* --> Set the params you need for the ncnn inference <-- */

    yolov4->opt.num_threads = 4; //You need to compile with libgomp for multi thread support

    yolov4->opt.use_vulkan_compute = true; //You need to compile with libvulkan for gpu support

    yolov4->opt.use_winograd_convolution = true;
    yolov4->opt.use_sgemm_convolution = true;
    yolov4->opt.use_fp16_packed = true;
    yolov4->opt.use_fp16_storage = true;
    yolov4->opt.use_fp16_arithmetic = true;
    yolov4->opt.use_packing_layout = true;
    yolov4->opt.use_shader_pack8 = false;
    yolov4->opt.use_image_storage = false;

    /* --> End of setting params <-- */
    int ret = 0;

    // original pretrained model from https://github.com/AlexeyAB/darknet
    // the ncnn model https://drive.google.com/drive/folders/1YzILvh0SKQPS_lrb33dmGNq7aVTKPWS0?usp=sharing
    // the ncnn model https://github.com/nihui/ncnn-assets/tree/master/models
// if set YOLOV4_TINY defination, to load the params from the transformed weights ..
#ifdef YOLOV4_TINY
    // std::string yolo4_1 = this->yolov4_param;
    // std::string yolo4_2 = this->yolov4_model;
    const char* yolov4_param = this->yolov4_param.c_str();
    const char* yolov4_model = this->yolov4_model.c_str();
    *target_size = 416;          // set the original img shape to (416, 416)
#else
    const char* yolov4_param = "yolov4-opt.param";
    const char* yolov4_model = "yolov4-opt.bin";
    *target_size = 608;
#endif

    ret = yolov4->load_param(yolov4_param);
    if (ret != 0)
    {
        return ret;
    }

    ret = yolov4->load_model(yolov4_model);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int Yolo_Class::detect_yolov4(const cv::Mat& bgr, std::vector<Object>& objects, int target_size, ncnn::Net* yolov4)
{
    int img_w = bgr.cols;
    int img_h = bgr.rows;
    //利用ncnn的from_pixels_resize函数将opencv格式的图片转为ncnn格式的图片，用于网络的前向推理，
    //from_pixels_resize函数还会对图片进行缩放，
    //如果只需要转换图片可以使用from_pixels函数
    ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR2RGB, bgr.cols, bgr.rows, target_size, target_size);

    const float mean_vals[3] = {0, 0, 0};
    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    //对图片进行归一化,将像素归一化到-1~1之间
    in.substract_mean_normalize(mean_vals, norm_vals);  //normlize to the img

    ncnn::Extractor ex = yolov4->create_extractor();    //实例化Extractor, 就是模拟了网络的前向传播

    ex.input("data", in);   //注意把"data"换成数据层中的名字

    ncnn::Mat out; //note:ncnn没有提供可以直接输出Mat数据的函数，所以想要输出Mat数据时，只能利用for循环进行遍历
    ex.extract("output", out); //将output换成最后一层的名字
    // std::cout<<out.c<<" "<<out.h<<std::endl;
    objects.clear();        //将上一次的清空
    for (int i = 0; i < out.h; i++)
    {
        const float* values = out.row(i);
        
        Object object;
        object.label = values[0];
        switch (object.label)
        {
            case 1:
            {
                object.name="chips";
                break;
            }
            case 2:
            {
                object.name="tea";
            }
        }
        
        object.prob = values[1];
        object.rect.x = values[2] * img_w;
        object.rect.y = values[3] * img_h;
        object.rect.width = values[4] * img_w - object.rect.x;
        object.rect.height = values[5] * img_h - object.rect.y;

        objects.push_back(object);
    }

    return 0;
}

void Yolo_Class::draw_objects(const cv::Mat& bgr, const std::vector<Object>& objects, int is_streaming)
{
    static const char* class_names[] = {"null", "chips","tea"
                                       };

    cv::Mat image = bgr.clone();

    for (size_t i = 0; i < objects.size(); i++)
    {
        const Object& obj = objects[i];

        fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.prob,
                obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);

        cv::rectangle(image, obj.rect, cv::Scalar(255, 0, 0));

        char text[256];
        sprintf(text, "%s %.1f%%", class_names[obj.label], obj.prob * 100);

        int baseLine = 0;
        cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

        int x = obj.rect.x;
        int y = obj.rect.y - label_size.height - baseLine;
        if (y < 0)
            y = 0;
        if (x + label_size.width > image.cols)
            x = image.cols - label_size.width;

        cv::rectangle(image, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                      cv::Scalar(255, 255, 255), -1);

        cv::putText(image, text, cv::Point(x, y + label_size.height),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
    }

    cv::imshow("image", image);

    if (is_streaming)
    {
        cv::waitKey(1);
    }
    else
    {
        cv::waitKey(0);
    }

    // return 0;
}
