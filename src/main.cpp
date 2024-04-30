// Copyright (c) 2021 by Rockchip Electronics Co., Ltd. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//
// Modified by Q-engineering 4-6-2024
//

/*-------------------------------------------
                Includes
-------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cstdlib>                  // for malloc and free
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "postprocess.h"
#include "rknn_api.h"

/*-------------------------------------------
                  Functions
-------------------------------------------*/

static void dump_tensor_attr(rknn_tensor_attr* attr)
{
  printf("\tindex=%d, name=%s, \n\t\tn_dims=%d, dims=[%d, %d, %d, %d], \n\t\tn_elems=%d, size=%d, fmt=%s, \n\t\ttype=%s, qnt_type=%s, "
         "zp=%d, scale=%f\n",
         attr->index, attr->name, attr->n_dims, attr->dims[0], attr->dims[1], attr->dims[2], attr->dims[3],
         attr->n_elems, attr->size, get_format_string(attr->fmt), get_type_string(attr->type),
         get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
}

// Function to read binary file into a buffer allocated in memory
static unsigned char* load_model(const char* filename, int& fileSize) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate); // Open file in binary mode and seek to the end

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return nullptr;
    }

    fileSize = (int) file.tellg(); // Get the file size
    file.seekg(0, std::ios::beg); // Seek back to the beginning

    char* buffer = (char*)malloc(fileSize); // Allocate memory for the buffer

    if (!buffer) {
        std::cerr << "Memory allocation failed." << std::endl;
        return nullptr;
    }

    file.read(buffer, fileSize); // Read the entire file into the buffer
    file.close(); // Close the file

    return (unsigned char*) buffer;
}

/*-------------------------------------------
                  Main Functions
-------------------------------------------*/
int main(int argc, char** argv)
{
    char*          model_name = NULL;
    const char*    imagepath = argv[1];
    rknn_context   ctx;
    int            img_width          = 0;
    int            img_height         = 0;
    const float    nms_threshold      = NMS_THRESH;
    const float    box_conf_threshold = BOX_THRESH;
    int            ret;

    float f;
    float FPS[16];
    int i, Fcnt=0;
    std::chrono::steady_clock::time_point Tbegin, Tend;

    for(i=0;i<16;i++) FPS[i]=0.0;

    if (argc < 3) {
        fprintf(stderr,"Usage: %s [imagepath] [model]\n", argv[0]);
        return -1;
    }

    printf("post process config: box_conf_threshold = %.2f, nms_threshold = %.2f\n", box_conf_threshold, nms_threshold);

    model_name = argv[2];

    // Create the neural network
    printf("Loading mode...\n");
    int            model_data_size = 0;
    unsigned char* model_data      = load_model(model_name, model_data_size);

    ret = rknn_init(&ctx, model_data, model_data_size, 0, NULL);
    //no need to hold the model in the buffer. Get some memory back
    free(model_data);

    if (ret < 0) {
        printf("rknn_init error ret=%d\n", ret);
        return -1;
    }

    rknn_sdk_version version;
    ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
    if (ret < 0) {
        printf("rknn_init error ret=%d\n", ret);
        return -1;
    }
    printf("sdk version: %s driver version: %s\n", version.api_version, version.drv_version);

    rknn_input_output_num io_num;
    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret < 0) {
        printf("rknn_init error ret=%d\n", ret);
        return -1;
    }
    printf("\nmodel input num: %d\n", io_num.n_input);
    rknn_tensor_attr input_attrs[io_num.n_input];
    memset(input_attrs, 0, sizeof(input_attrs));
    for(uint32_t i = 0; i < io_num.n_input; i++) {
        input_attrs[i].index = i;
        ret                  = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret < 0) {
            printf("rknn_init error ret=%d\n", ret);
            return -1;
        }
        dump_tensor_attr(&(input_attrs[i]));
    }

    printf("\nmodel output num: %d\n", io_num.n_output);
    rknn_tensor_attr output_attrs[io_num.n_output];
    memset(output_attrs, 0, sizeof(output_attrs));
    for (uint32_t i = 0; i < io_num.n_output; i++) {
        output_attrs[i].index = i;
        ret                   = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
        dump_tensor_attr(&(output_attrs[i]));
    }

    int channel = 3;
    int width   = 0;
    int height  = 0;
    if (input_attrs[0].fmt == RKNN_TENSOR_NCHW) {
        printf("\nmodel input is NCHW\n");
        channel = input_attrs[0].dims[1];
        height  = input_attrs[0].dims[2];
        width   = input_attrs[0].dims[3];
    }
    else {
        printf("\nmodel input is NHWC\n");
        height  = input_attrs[0].dims[1];
        width   = input_attrs[0].dims[2];
        channel = input_attrs[0].dims[3];
    }

    printf("model input height=%d, width=%d, channel=%d\n\n", height, width, channel);

    rknn_input inputs[1];
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index        = 0;
    inputs[0].type         = RKNN_TENSOR_UINT8;
    inputs[0].size         = width * height * channel;
    inputs[0].fmt          = RKNN_TENSOR_NHWC;
    inputs[0].pass_through = 0;

    // You may not need resize when src resulotion equals to dst resulotion
    cv::Mat orig_img;
    cv::Mat img;
    cv::Mat resized_img;

    printf("Start grabbing, press ESC on Live window to terminated...\n");
    while(1){

        orig_img=cv::imread(imagepath, 1);
        if(orig_img.empty()) {
            printf("Error grabbing\n");
            break;
        }

        Tbegin = std::chrono::steady_clock::now();

        cv::cvtColor(orig_img, img, cv::COLOR_BGR2RGB);
        img_width  = img.cols;
        img_height = img.rows;

        //check sizes
        if (img_width != width || img_height != height) {
            cv::resize(img,resized_img,cv::Size(width,height));
            inputs[0].buf = (void*)resized_img.data;
        } else {
            inputs[0].buf = (void*)img.data;
        }

        rknn_inputs_set(ctx, io_num.n_input, inputs);

        rknn_output outputs[io_num.n_output];
        memset(outputs, 0, sizeof(outputs));
        for (uint32_t i = 0; i < io_num.n_output; i++) {
            outputs[i].index = i;
            outputs[i].want_float = 0;
        }

        rknn_run(ctx, NULL);
        rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);

        // post process
        float scale_w = (float)width / img_width;
        float scale_h = (float)height / img_height;

        detect_result_group_t detect_result_group;
        std::vector<float> out_scales;
        std::vector<int32_t> out_zps;
        for(uint32_t i = 0; i < io_num.n_output; ++i){
            out_scales.push_back(output_attrs[i].scale);
            out_zps.push_back(output_attrs[i].zp);
        }

        post_process((int8_t *)outputs[0].buf, (int8_t *)outputs[1].buf, (int8_t *)outputs[2].buf, height, width,
                   box_conf_threshold, nms_threshold, scale_w, scale_h, out_zps, out_scales, &detect_result_group);


        // Draw Objects
        char text[256];
        for (int i = 0; i < detect_result_group.count; i++) {
            detect_result_t* det_result = &(detect_result_group.results[i]);

            int x1 = det_result->box.left;
            int y1 = det_result->box.top;
            int x2 = det_result->box.right;
            int y2 = det_result->box.bottom;
            cv::rectangle(orig_img, cv::Point(x1, y1), cv::Point(x2, y2),cv::Scalar(255, 0, 0));

//            printf("%s @ (%d %d %d %d) %f\n", det_result->name, det_result->box.left, det_result->box.top,
//                   det_result->box.right, det_result->box.bottom, det_result->prop);

            //put some text
            sprintf(text, "%s %.1f%%", det_result->name, det_result->prop * 100);

            int baseLine = 0;
            cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

            int x = det_result->box.left;
            int y = det_result->box.top - label_size.height - baseLine;
            if (y < 0) y = 0;
            if (x + label_size.width > orig_img.cols) x = orig_img.cols - label_size.width;

            cv::rectangle(orig_img, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)), cv::Scalar(255, 255, 255), -1);

            cv::putText(orig_img, text, cv::Point(x, y + label_size.height), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
        }

        ret = rknn_outputs_release(ctx, io_num.n_output, outputs);

        Tend = std::chrono::steady_clock::now();
        //calculate frame rate
        f = std::chrono::duration_cast <std::chrono::milliseconds> (Tend - Tbegin).count();
        if(f>0.0) FPS[((Fcnt++)&0x0F)]=1000.0/f;
        for(f=0.0, i=0;i<16;i++){ f+=FPS[i]; }
        putText(orig_img, cv::format("FPS %0.2f", f/16),cv::Point(10,20),cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(0, 0, 255));

        //show output
//        std::cout << "FPS" << f/16 << std::endl;
        imshow("Radxa zero 3W - 1,8 GHz - 4 Mb RAM", orig_img);
        char esc = cv::waitKey(5);
        if(esc == 27) break;

//      imwrite("./out.jpg", orig_img);
    }

    // release
    ret = rknn_destroy(ctx);

    return 0;
}
