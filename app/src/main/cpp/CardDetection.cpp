#include "CardDetection.h"


/**
 * Constructor. Localize card and compute confidence score
 * @param sourceImg original input image with tree and card
 * @param cardImg image of card
 */
CardDetection::CardDetection(cv::Mat sourceImg, cv::Mat cardImg ){
    this->sourceImg = sourceImg.clone();
    this->cardImg = cardImg.clone();

    this->points = findCard();
    this->card_confidence = confidence();
}

/**
 * Getter. 
 * @return vector of card points (upper left, upper right, bottom right, bottom left)
 */
std::vector<cv::Point2f> CardDetection::getPoints() {
    return this->points;
}

float CardDetection::getConfidenceScore() {
    return this->card_confidence;
}

/**
 * Compute card confidence score.
 * How much inner angles of rectangle differs from 90degrees. Each of 4 angles can differ up to 90degrees. Sum all four differences together. 0 is minimum error, 360 is maximum error. Then map it to interval <0,1>.
 * @return confidence score, interval <0,1> where 1 is perfect rectangle
 */
float CardDetection::confidence() {

    if (points.empty()) {     //card was not found
        return 0.0;
    }
    else {
        float angle1 = float(abs(abs(fmod(angleBetween3Points(points[0], points[1], points[2]), 180)) - 90));
        float angle2 = float(abs(abs(fmod(angleBetween3Points(points[1], points[2], points[3]), 180)) - 90));
        float angle3 = float(abs(abs(fmod(angleBetween3Points(points[2], points[3], points[0]), 180)) - 90));
        float angle4 = float(abs(abs(fmod(angleBetween3Points(points[3], points[0], points[1]), 180)) - 90));

        return ((360 - (angle1 + angle2 + angle3 + angle4)) / 360);
    }

}

/**
 * Compute angle between 3 points
 * @return angle between 3 points
 */
float angleBetween3Points(cv::Point2f pointA, cv::Point2f pointB, cv::Point2f pointC) {
    float a = pointB.x - pointA.x;
    float b = pointB.y - pointA.y;
    float c = pointB.x - pointC.x;
    float d = pointB.y - pointC.y;

    float atanA = atan2(a, b);
    float atanB = atan2(c, d);

    return float(((atanB - atanA) * 180.0) / CV_PI);
}



/**
 * Find card in the image with SIFT.
 * SIFT tutorial: https://docs.opencv.org/3.4/d7/dff/tutorial_feature_homography.html
 * @return 4 points of card or empty vector if card was not found
 */
std::vector<cv::Point2f> CardDetection::findCard()
{

    cv::Mat image, card;

    // convert to grey
    cvtColor(this->sourceImg, image, cv::COLOR_BGR2GRAY);
    cvtColor(this->cardImg, card, cv::COLOR_BGR2GRAY);

    // resize image
    int resizeToWidth = 1000;
    float ratio = float(resizeToWidth) / float(image.cols);
    int newHeight = int(round(ratio * image.rows));
    cv::resize(image, image, cv::Size(resizeToWidth, newHeight), cv::INTER_LINEAR);

    // blur tree image and card
    cv::GaussianBlur(image, image, cv::Size(5, 5), 0);
    cv::GaussianBlur(card, card, cv::Size(3, 3), 0);

    // initialize SIFT detector 
    cv::Ptr<cv::SiftFeatureDetector> detectorS = cv::SiftFeatureDetector::create();
    std::vector<cv::KeyPoint> keypoints_image, keypoints_card;
    Mat descriptors_image, descriptors_card;

    // detect keypoints and compute descriptors
    detectorS->detectAndCompute(image, noArray(), keypoints_image, descriptors_image);
    detectorS->detectAndCompute(card, noArray(), keypoints_card, descriptors_card);

    //show keypoints in tree image 
    /*Mat outimg;
    drawKeypoints(image, keypoints_image, outimg, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
    imshow("SIFT", outimg);
    Mat outimg22;
    drawKeypoints(card, keypoints_card, outimg22, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
    imshow("SIFT_card", outimg22);*/

    //Matching descriptor vectors with a FLANN based matcher
    //Since SURF is a floating-point descriptor NORM_L2 must be used
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    std::vector< std::vector<DMatch> > knn_matches;
    matcher->knnMatch(descriptors_card, descriptors_image, knn_matches, 2);

    //-- Filter matches using the Lowe's ratio test
    // higher ratio -> more points
    const float ratio_thresh = 0.5f;
    std::vector<DMatch> good_matches;
    for (size_t i = 0; i < knn_matches.size(); i++)
    {
        if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
        {
            good_matches.push_back(knn_matches[i][0]);
        }
    }

    //-- Draw good matches
    /*Mat img_matches;
    drawMatches(card, keypoints_card, image, keypoints_image,  good_matches, img_matches, Scalar::all(-1),
        Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    imshow("Matches", img_matches);*/

    //at least 5 good matches to find a card
    if (good_matches.size()<5) {
        std::vector<Point2f> empty_corners;
        return empty_corners;
    }

    //-- Localize  card
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;
    for (size_t i = 0; i < good_matches.size(); i++)
    {
        //-- Get the keypoints from the good matches
        obj.push_back(keypoints_card[good_matches[i].queryIdx].pt);
        scene.push_back(keypoints_image[good_matches[i].trainIdx].pt);
    }

    Mat H = findHomography(obj, scene, RANSAC);

    //-- Get the corners from the image
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = Point2f(0, 0);
    obj_corners[1] = Point2f((float)card.cols, 0);
    obj_corners[2] = Point2f((float)card.cols, (float)card.rows);
    obj_corners[3] = Point2f(0, (float)card.rows);

    // transform objects corners to the scene
    std::vector<Point2f> scene_corners(4);
    perspectiveTransform(obj_corners, scene_corners, H);

    /*
    //-- Draw lines between the corners (the mapped object in the scene)
    line(img_matches, scene_corners[0] + Point2f((float)card.cols, 0),
        scene_corners[1] + Point2f((float)card.cols, 0), Scalar(0, 255, 0), 2);
    line(img_matches, scene_corners[1] + Point2f((float)card.cols, 0),
        scene_corners[2] + Point2f((float)card.cols, 0), Scalar(0, 255, 0), 2);
    line(img_matches, scene_corners[2] + Point2f((float)card.cols, 0),
        scene_corners[3] + Point2f((float)card.cols, 0), Scalar(0, 255, 0), 2);
    line(img_matches, scene_corners[3] + Point2f((float)card.cols, 0),
        scene_corners[0] + Point2f((float)card.cols, 0), Scalar(0, 255, 0), 2);
 
    imshow("Good Matches & Object detection", img_matches);
    */

    // adapt points to original size
    for (int i = 0; i < scene_corners.size(); i++)
        scene_corners[i] /= ratio;

    return scene_corners;
}



/**
 * Show results on downsized image. Draw borders of card which was found.
 * @return image with drawn card
 */
cv::Mat CardDetection::getMarkedImage()
{
    Mat image = sourceImg.clone();

    // resize image
    int resizeToWidth = 600;
    float ratio = float(resizeToWidth) / float(image.cols);
    int newHeight = int(round(ratio * image.rows));
    cv::resize(sourceImg, image, cv::Size(resizeToWidth, newHeight), cv::INTER_LINEAR);

    // adapt points to new size
    std::vector<cv::Point2f> pts = points;
    for (int i = 0; i < pts.size(); i++)
        pts[i] *= ratio;

    //-- Draw lines between the corners (the mapped object in the scene)
    line(image, pts[0], pts[1], Scalar(255, 0, 0), 2);
    line(image, pts[1], pts[2], Scalar(255, 0, 0), 2);
    line(image, pts[2], pts[3], Scalar(255, 0, 0), 2);
    line(image, pts[3], pts[0], Scalar(255, 0, 0), 2);

    // draw points in corners
    circle(image, pts[0], 6, Scalar(0, 255, 0), FILLED, LINE_8);
    circle(image, pts[1], 6, Scalar(0, 255, 0), FILLED, LINE_8);
    circle(image, pts[2], 6, Scalar(0, 255, 0), FILLED, LINE_8);
    circle(image, pts[3], 6, Scalar(0, 255, 0), FILLED, LINE_8);

    return image;
}