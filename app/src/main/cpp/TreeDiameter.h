#ifndef TREEDIAMETER_H
#define TREEDIAMETER_H

//#include "TreeDetection.h"

#include <stdio.h>
#include <string>

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

float getTreeWidth(std::vector<cv::Point2f> treePts, std::vector<cv::Point2f> cardPts);
//float getTreeWidth(cv::Mat image, std::vector<cv::Point2f> treePts, std::vector<cv::Point2f> cardPts);
cv::Point2f line_intersection(cv::Point2f A, cv::Point2f B, cv::Point2f C, cv::Point2f D);
std::vector<cv::Point2f> extendLine(cv::Point2f l1, cv::Point2f l2, int h, int w);
float toRadians(float degree);
float toDegrees(float radian);
std::vector<cv::Point2f> getPerpendicularInInterSc(cv::Point2f startPt, cv::Point2f inSc);
float distBetweenPoints(cv::Point2f p1, cv::Point2f p2);

#endif //TREEDIAMETER_H
