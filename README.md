# SURF object detection
Object detection using the non-free SURF Algorithm in C++, using opencv 3.0.

## How to use
Simply replace *filepath define* with the filepath of an image containing solely the object you wish to detect. It is important nothing else is included in the image, cropping may be neccessary.
```C++
#define filepath "/Users/Joe/Desktop/object.jpg"
```

OpenCV 3.0 is required, as 'non-free' alorithms have since been removed from 3.0 you have to build it using the [opencv_contrib](https://github.com/itseez/opencv_contrib) package. I suggesting using [CMake](https://cmake.org) to speed things up.

## Linker flags
-lopencv_calib3d, -lopencv_core, -lopencv_features2d, -lopencv_flann, -lopencv_highgui, -lopencv_imgproc, -lopencv_xfeatures2d, -lopencv_videoio, -lopencv_imgcodecs
