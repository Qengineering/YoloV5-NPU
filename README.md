# YoloV5 NPU
![output image]( https://qengineering.eu/github/YoloV5_Parking_NPU.webp )
## YoloV5 for RK3566/68 and RK3588 NPU (Rock 5, Orange Pi 5, Radxa Zero 3). <br/>
[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)<br/><br/>
Paper: https://towardsdatascience.com/yolo-v5-is-here-b668ce2a4908<br/><br/>
Special made for the NPU, see [Q-engineering deep learning examples](https://qengineering.eu/deep-learning-examples-on-raspberry-32-64-os.html)

------------

## Model performance benchmark(FPS)

All models, with C++ examples can be found on the SD images.<br><br>
![output image]( https://qengineering.eu/github/RockPi5_Ubuntu_22.jpg ) [Rock 5 with **Ubuntu 22.04**, OpenCV, ncnn and **NPU**](https://github.com/Qengineering/Rock-5-Ubuntu-22-image)<br><br>
![output image]( https://qengineering.eu/github/RadxaZero3_Ubuntu_22.jpg ) [Radxa Zero 3 with **Ubuntu 22.04**, OpenCV, ncnn and **NPU**](https://github.com/Qengineering/Radxa-Zero-3-NPU-Ubuntu22)<br><br>



| demo             | model_name                   | inputs_shape            | dtype | RK3588  | RK3566/68  |
| ---------------- | ---------------------------- | ----------------------- | ----- | :-----: | :--------: |
| yolov5           | yolov5s_relu                 | [1, 3, 640, 640]        | INT8  | 50.0    | 14.8       |
|                  | yolov5n                      | [1, 3, 640, 640]        | INT8  | 58.8    | 19.5       |
|                  | yolov5s                      | [1, 3, 640, 640]        | INT8  | 37.7    | 11.7       |
|                  | yolov5m                      | [1, 3, 640, 640]        | INT8  | 16.2    | 5.7        |
| yolov6           | yolov6n                      | [1, 3, 640, 640]        | INT8  | 63.0    | 18.0       |
|                  | yolov6s                      | [1, 3, 640, 640]        | INT8  | 29.5    | 8.1        |
|                  | yolov6m                      | [1, 3, 640, 640]        | INT8  | 15.4    | 4.5        |
| yolov7           | yolov7-tiny                  | [1, 3, 640, 640]        | INT8  | 53.4    | 16.1       |
|                  | yolov7                       | [1, 3, 640, 640]        | INT8  | 9.4     | 3.4        |
| yolov8           | yolov8n                      | [1, 3, 640, 640]        | INT8  | 53.1    | 18.2       |
|                  | yolov8s                      | [1, 3, 640, 640]        | INT8  | 28.5    | 8.9        |
|                  | yolov8m                      | [1, 3, 640, 640]        | INT8  | 12.1    | 4.4        |
| yolox            | yolox_s                      | [1, 3, 640, 640]        | INT8  | 30.0    | 10.0       |
|                  | yolox_m                      | [1, 3, 640, 640]        | INT8  | 12.9    | 4.8        |
| ppyoloe          | ppyoloe_s                    | [1, 3, 640, 640]        | INT8  | 28.8    | 9.2        |
|                  | ppyoloe_m                    | [1, 3, 640, 640]        | INT8  | 13.1    | 5.04       |
| yolov5_seg       | yolov5n-seg                  | [1, 3, 640, 640]        | INT8  | 9.4     | 1.04       |
|                  | yolov5s-seg                  | [1, 3, 640, 640]        | INT8  | 7.8     | 0.87       |
|                  | yolov5m-seg                  | [1, 3, 640, 640]        | INT8  | 6.1     | 0.71       |
| yolov8_seg       | yolov8n-seg                  | [1, 3, 640, 640]        | INT8  | 8.9     | 0.91       |
|                  | yolov8s-seg                  | [1, 3, 640, 640]        | INT8  | 7.3     | 0.87       |
|                  | yolov8m-seg                  | [1, 3, 640, 640]        | INT8  | 4.5     | 0.7        |
| ppseg	           | ppseg_lite_1024x512          |	[1, 3, 512, 512]	      | INT8	| 27.5    | 2.4        |
| RetinaFace       | RetinaFace_mobile320         | [1, 3, 320, 320]        | INT8  | 243.6   | 88.5       |
|                  | RetinaFace_resnet50_320      | [1, 3, 320, 320]        | INT8  | 43.4    | 11.8       |
| PPOCR-Det        | ppocrv4_det                  | [1, 3, 480, 480]        | INT8  | 31.5    | 15.1       |
| PPOCR-Rec        | ppocrv4_rec                  | [1, 3, 48, 320]         | FP16  | 35.7    | 17.3       |

* Due to the pixelwise filling and drawing, segmentation models are relative slow

------------

## Dependencies.
To run the application, you have to:
- OpenCV 64-bit installed.
- Code::Blocks installed. (```$ sudo apt-get install codeblocks```)

### Installing the dependencies.
Start with the usual 
```
$ sudo apt-get update 
$ sudo apt-get upgrade
$ sudo apt-get install cmake wget curl
```
#### OpenCV
Follow the Raspberry Pi 4 [guide](https://qengineering.eu/install-opencv-on-raspberry-64-os.html).<br>

#### RKNPU2
```
$ git clone https://github.com/airockchip/rknn-toolkit2.git
```
We only use a few files.
```
rknn-toolkit2-master
│      
└── rknpu2
    │      
    └── runtime
        │       
        └── Linux
            │      
            └── librknn_api
                ├── aarch64
                │   └── librknnrt.so
                └── include
                    ├── rknn_api.h
                    ├── rknn_custom_op.h
                    └── rknn_matmul_api.h

$ cd ~/rknn-toolkit2-master/rknpu2/runtime/Linux/librknn_api/aarch64
$ sudo cp ./librknnrt.so /usr/local/lib
$ cd ~/rknn-toolkit2-master/rknpu2/runtime/Linux/librknn_api/include
$ sudo cp ./rknn_* /usr/local/include
```
Save 2 GB of disk space by removing the toolkit. We do not need it anymore.
```
$ cd ~
$ sudo rm -rf ./rknn-toolkit2-master
```

------------

## Installing the app.
To extract and run the network in Code::Blocks <br/>
```
$ mkdir *MyDir* <br/>
$ cd *MyDir* <br/>
$ git clone https://github.com/Qengineering/YoloV5-NPU.git <br/>
```

------------

## Running the app.
To run the application load the project file YoloV5.cbp in Code::Blocks.<br/><br>
Or use **Cmake**.
```
$ cd *MyDir*
$ mkdir build
$ cd build
$ cmake ..
$ make -j4
```
Make sure you use the model fitting your system.<br><br>

More info or if you want to connect a camera to the app, follow the instructions at [Hands-On](https://qengineering.eu/deep-learning-examples-on-raspberry-32-64-os.html#HandsOn).<br/><br/>
![output image]( https://qengineering.eu/github/YoloV5_Bus_NPU.webp )

------------

[![paypal](https://qengineering.eu/images/TipJarSmall4.png)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=CPZTM5BB3FCYL) 


