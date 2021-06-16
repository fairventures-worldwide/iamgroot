#ifndef CARDDETECTION_H
#define CARDDETECTION_H


#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <opencv2/features2d/features2d.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class CardDetection {
private:
    std::string TAG = "CardDetection";
    cv::Mat sourceImg;                  //image of tree and card
    cv::Mat cardImg;                    //image of card which should be found in sourceImg
    std::vector<cv::Point2f> points;    //vector of card points (upper left, upper right, bottom right, bottom left)
    float card_confidence;              //confidence that card was found

    std::vector<cv::Point2f> findCard();
    float confidence();


public:
    CardDetection(cv::Mat sourceImg, cv::Mat cardImage);
    cv::Mat getMarkedImage();
    std::vector<cv::Point2f> getPoints();
    float getConfidenceScore();

};

float angleBetween3Points(cv::Point2f a, cv::Point2f b, cv::Point2f c);

#endif //CARDDETECTION_H