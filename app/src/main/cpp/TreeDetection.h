#ifndef TREEDETECTION_H
#define TREEDETECTION_H

#include <stdio.h>
#include <string>
#include <iostream>
#include <map>
#include <iterator>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define TREE_SHOW_IMAGES 0


class TreeDetection {
private:
    std::string TAG = "TreeDetection";

    cv::Mat image;      /**< Input image resized to 'resize_to_width' */
    cv::Mat image_roi;  /**< Cropped resized image */   
    cv::Rect2f roi;     /**< Region of interest above or under card */  
    float resize_to_width = 600;    /**< The width to which the input image is resized */
    float ratio;        /**< Ratio of resized width and original width */

    cv::Mat tree_mask_roi;  /**< Binary mask of the tree */
    std::map<std::string, cv::Point2f> card_points; /**< Ordered card points in map. Top left point = 'tl', bottom right = 'br' */
    std::tuple<cv::Point2f, cv::Point2f> left_tree_line, right_tree_line;   /**< The edge of tree represented by a line. Tuple points, top point first */
    //std::tuple<cv::Point2f, cv::Point2f> left_tree_line2, right_tree_line2;


    void doGrabcut(cv::Point2f card_middle);
    int findLines();
    int lines_intersect(cv::Vec2f line1, cv::Vec2f line2);

    void linePoints(cv::Vec2f line, cv::Point2f& pt1, cv::Point2f& pt2);
    cv::Point2f intersection(std::tuple<cv::Point2f, cv::Point2f> image_line, std::tuple<cv::Point2f, cv::Point2f> line);
public:
    TreeDetection(cv::Mat source_img, std::vector<cv::Point2f> card_points);
    ~TreeDetection(){};

    int findTree(int position);

    cv::Mat getOutputImage();
    cv::Mat getTreeMask(){return tree_mask_roi;}
    std::vector<cv::Point2f> getTreeLines();
    //std::tuple<cv::Point2f, cv::Point2f> getLeftTreeLine(){return this->left_tree_line;};
    //std::tuple<cv::Point2f, cv::Point2f> getRightTreeLine(){return this->right_tree_line;};
};


double pointsDistance(cv::Point2f p1, cv::Point2f p2);
double distanceToLine(cv::Point2f line_start, cv::Point2f line_end, cv::Point2f point);
std::map<std::string, cv::Point2f> orderCardPoints(std::vector<cv::Point2f> points);
cv::Mat maskCard(std::map<std::string, cv::Point2f> points, cv::Mat input, int margin = 0);


#endif //TREEDETECTION_H