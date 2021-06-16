//treeo project class structure
#include <iostream>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

using namespace std;

class ObjectDetector {
private:
    cv::Mat TreeInputImage;
    cv::Mat CardInputImage;

    //pair<int, int>* card_polygon; //nebo std::vector<cv::Point2f>
    std::vector<cv::Point2f> card_polygon;
    double card_confidence = 0;//0 to 1
    //pair<int, int>* tree_polygon; //nebo std::vector<cv::Point2f>
    std::vector<cv::Point2f> tree_polygon;
    double tree_confidence = 0; //0 to 1
    double diameter_value;
    double diameter_cofidence = 0; //0 to 1

    //the following functions only return error codes
    int setImage(string);
    int detectCard();
    int refineCardPosition();
    int detectTree();
    int computeDiameter();
    //void reset();//internal structures/data
public:
    ObjectDetector ();
    ObjectDetector (cv::Mat);
    ObjectDetector (string);

    //Getters
    std::vector<cv::Point2f> getCardPolygon(){return card_polygon;}
    std::vector<cv::Point2f> getTreePolygon(){return tree_polygon;}
    double getDiameterValue(){return diameter_value;}

    int measureTree(cv::Mat input_image, double &diameter); //&confidence
    int measureTree(cv::Mat input_image, std::vector<cv::Point2f> &card, std::vector<cv::Point2f> &tree, double &diameter); //&confidence
    /*return value is error type:
    0 OK
    1 card failed
    2 tree failed
    3 diameter failed
    */
};

