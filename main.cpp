//
//  main.cpp
//  cpp-surf-example
//
//  Created by Joe on 19/11/2016.
//  Copyright © 2016 joemcalister. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/xfeatures2d/nonfree.hpp"

#define filepath "image-filepath-here"

// declare namespaces
using namespace cv;
using namespace std;
using namespace xfeatures2d;

int main(int argc, const char * argv[]) {
    
    // object to search for
    Mat imgObject = imread(filepath, CV_LOAD_IMAGE_GRAYSCALE);
    
    // create a vector to store previous coordinates for comparison
    vector<vector<Point2f>> coords;
    
    // standard video capture
    VideoCapture cap(0);
    
    // create variable to store the most recent frame
    Mat frame;
    
    // loop through frames -- while loop prevents end of application
    while (cap.read(frame)) {
        
        // we have the frame variable which contains the current view
        Mat imgScene = frame;

        
        // surf feature detector -- this esentially finds points that are the most likely to be interesting and unique within an image
        Ptr<SurfFeatureDetector> detector = SURF::create(400);
        std::vector<KeyPoint> keypointObject, keypointScene;
        detector->detect(imgObject, keypointObject);
        detector->detect(imgScene, keypointScene);
        
        
        // feature descriptors -- this provides vector correspondence to the keypoints, used for the flann
        Ptr<SurfDescriptorExtractor> extractor = SURF::create();

        
        // create descriptors for the object and scene
        Mat objectDescriptors, sceneDescriptors;
        extractor->compute(imgObject, keypointObject, objectDescriptors);
        extractor->compute(imgScene, keypointScene, sceneDescriptors);
        
        
        // Match descriptor vectors using FLANN Matcher -- FLANN provides incredibly quick matching between the keypoints we determined in the two images (ref image and webcam frame) using the surf feature detector
        FlannBasedMatcher matcher;
        vector <DMatch> matches;
        matcher.match(objectDescriptors, sceneDescriptors, matches);
        
        
        // set max and min distances for the points, see below
        float maximumDistance = 0;
        float minimumDistance = 100;
        
        
        // calculate the max and min distances between points
        for (int i = 0; i < objectDescriptors.rows; i++)
        {
            float dist = matches[i].distance;
            if (dist < minimumDistance)
            {
                minimumDistance = dist;
                
            }else if (dist > maximumDistance)
            {
                maximumDistance = dist;
            }
        }
        
        
        // only draw matches that are 4x as long as the smallest match -- these should be the 'good ones'
        vector <DMatch> goodMatches;
        for (int i=0; i<objectDescriptors.rows; i++)
        {
            if (matches[i].distance < 4*minimumDistance)
            {
                goodMatches.push_back(matches[i]);
            }
        }
        
        // create vectors of points for both the object and scene
        vector<Point2f> object;
        vector<Point2f> scene;
        
        for (int i = 0; i < goodMatches.size(); i++)
        {
            // get keypoints but only from the good matches
            object.push_back(keypointObject[goodMatches[i].queryIdx].pt);
            scene.push_back(keypointScene[goodMatches[i].trainIdx].pt);
        }
        
        
        // find the homography of the scene, this allows the box drawn to perfectly fit the object, no matter what rotation on the x, y or z axis
        Mat homography = findHomography(object, scene, CV_RANSAC);
        
        
        // get the corners for the image
        vector<Point2f> objectCorners(4);
        objectCorners[0] = cvPoint(0,0); objectCorners[1] = cvPoint(imgObject.cols, 0);
        objectCorners[2] = cvPoint(imgObject.cols, imgObject.rows); objectCorners[3] = cvPoint(0, imgObject.rows);
        vector<Point2f> sceneCorners(4);
        
        
        // transform the coordinates so they match the homography
        perspectiveTransform(objectCorners, sceneCorners, homography);
        
        
        
        // determine if should display, is the last set of points simmiar to the new? -- 2 pass system, are the last coordinates roughly the same as the last frames? This prevents sporatic incorrect matching, also slows detection speed slightly so should be looked into a bit more
        if (coords.size() > 0)
        {
            vector<Point2f> lastResVec = coords.back();
            bool flag = true;
            int count = 0;
            for (auto &pts:lastResVec)
            {
                Point2f thisPnts = sceneCorners[count];
                
                if (!(fabs(thisPnts.x - pts.x) < 50))
                {
                    flag = false;
                }
                
                count++;
            }
            
            if (flag == true)
            {
                // draw the lines across all the corners
                for (int i=0; i<sceneCorners.size(); i++)
                {
                    Point2f first = sceneCorners[i];
                    Point2f second;
                    
                    if (i < sceneCorners.size()-1)
                    {
                        second = sceneCorners[i+1];
                    }else {
                        second = sceneCorners[0];
                    }
                    
                    line(imgScene, first, second, Scalar(0, 255, 0), 4);
                }
            }
        }
        
        
        // push coords
        coords.push_back(sceneCorners);
        
        
        // show the matches on the screen
        imshow("cpp-surf-example", imgScene);

        
        // allow a break key to end program
        if (waitKey(1) == 'e') {
            break;
        }
    }
    
    return 0;
}

