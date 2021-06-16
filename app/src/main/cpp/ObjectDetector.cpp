//treeo project class structure
#include "ObjectDetector.h"
#include "CardDetection.h"
#include "TreeDetection.h"
#include "TreeDiameter.h"
#include <android/log.h>

using namespace std;

//temporary
//std::string path_to_card = "../images/karta2.png";

int ObjectDetector::measureTree (cv::Mat input_image, double &diameter) {
    this->TreeInputImage = input_image;
    int ret_value = 0;

    // detect all
    ret_value = detectCard();
    if (ret_value > 0) return ret_value;

    ret_value = detectTree();
    if (ret_value > 0) return ret_value;

    //measure
    computeDiameter();
    //return vals
    diameter = this->diameter_value;

    return 0;
}

int ObjectDetector::measureTree (cv::Mat input_image, std::vector<cv::Point2f> &card, std::vector<cv::Point2f> &tree, double &diameter) {
    this->TreeInputImage = input_image;
    int ret_value = 0;

    // detect all
    ret_value = detectCard();
    if (ret_value > 0) return ret_value;

    ret_value = detectTree();
    if (ret_value > 0) return ret_value;

    //measure
    computeDiameter();
    //return vals
    card = this->card_polygon;
    tree = this->tree_polygon;
    diameter = this->diameter_value;

    return 0;
}


ObjectDetector::ObjectDetector () {//prázdný kontrsuktor
}

ObjectDetector::ObjectDetector (cv::Mat chosen_card_image) {//constructor with card file
    this->CardInputImage = chosen_card_image;
}

ObjectDetector::ObjectDetector (string path_to_card) {//constructor with card file
    CardInputImage = cv::imread(path_to_card);
    if (!CardInputImage.data) {
        std::cerr << "Error: Unable to read card image file" << std::endl;
        __android_log_print(ANDROID_LOG_ERROR, "STORMY", "Error: Unable to read card image file");
    }
}



int ObjectDetector::detectCard(){
    // Load ID card from image, later SIFT
    //image is in CardInputImage
    if (!CardInputImage.data) {
        std::cerr << "Error: Unable to read card image file" << std::endl;
        __android_log_print(ANDROID_LOG_ERROR, "STORMY", "Error: Unable to read card image file");
        return 1;
    }

    CardDetection cardDet = CardDetection(TreeInputImage, CardInputImage);
    card_polygon = cardDet.getPoints();

    //float confidence = cardDet.getConfidenceScore();
    //cout << "Confidence score: " << confidence << endl;
    if (card_polygon.empty()) {
        std::clog << "Card was not found." << std::endl;
        __android_log_print(ANDROID_LOG_ERROR, "STORMY", "Card was not found.");
        return 1;
    }
    return 0;
}


int ObjectDetector::detectTree(){

    TreeDetection tree = TreeDetection(TreeInputImage, card_polygon);
    int ret = tree.findTree(1);
    if (ret < 0) {
        std::clog << "Tree was not found. Another try" << std::endl;
        __android_log_print(ANDROID_LOG_ERROR, "STORMY", "Tree was not found. Another try");
        ret = tree.findTree(2);
        if (ret < 0) {
            std::clog << "Tree was not found." << std::endl;
            __android_log_print(ANDROID_LOG_ERROR, "STORMY", "Tree was not found.");
            return 2;
        }
    }
    tree_polygon = tree.getTreeLines();

    return 0;
}


int ObjectDetector::computeDiameter(){
    if (this->card_polygon.empty() || this->tree_polygon.empty()){
        std::cerr << "Error: Empty card or tree points" << std::endl;
        __android_log_print(ANDROID_LOG_ERROR, "STORMY", "Error: Empty card or tree points");
        return (-1);
    }
    //measure
    float tree_width_in_pixels = getTreeWidth(this->tree_polygon, this->card_polygon);
    float card_width_in_pixels = distBetweenPoints(this->card_polygon[0], this->card_polygon[1]);
    this->diameter_value = float((tree_width_in_pixels / card_width_in_pixels * 85.6));
    return 0;
}


// ./treeProject path/to/image

int main(int argc, char const* argv[]){

    std::string path_to_tree;

    // parse args
    if (argc == 2){
        path_to_tree = argv[1];
    }else{
        std::cerr << "Set path to image (./treeProject path/to/image)" << std::endl;
        return -1;
    }

    // Load Image of tree
    //std::clog << "Filename: " + path_to_tree << std::endl;
    cv::Mat input_image = cv::imread(path_to_tree);
    if (!input_image.data) {
        std::cerr << "Error: Unable to read tree image file" << std::endl;
        __android_log_print(ANDROID_LOG_ERROR, "STORMY", "Error: Unable to read tree image file");
        return -1;
    }

    //ObjectDetector detector; // detect all
    ObjectDetector detector;
    std::vector<cv::Point2f> card_polygon, tree_polygon;
    double diameter_value = 0.0;
    cout << "ObjectDetector::measureTree() returned code: " << detector.measureTree(input_image, card_polygon, tree_polygon, diameter_value) << endl;
    cout << "Diameter: " << diameter_value << endl;

    __android_log_print(ANDROID_LOG_DEBUG, "STORMY", "Diameter= %d",diameter_value);

    return 0;
}