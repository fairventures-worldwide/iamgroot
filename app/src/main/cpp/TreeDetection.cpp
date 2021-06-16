#include "TreeDetection.h"

/**
 * Constructor. Resize original image to defined width. Resize and order card points.
 * @param source_img original input image with tree and card
 * @param card_pts vector of card points, corresponds to the original image
 */
TreeDetection::TreeDetection(cv::Mat source_img, std::vector<cv::Point2f> card_pts) {
    // resize image
    ratio = float(resize_to_width / source_img.cols);
    int new_height = round(ratio * source_img.rows);
    cv::resize(source_img, image, cv::Size(resize_to_width, new_height), cv::INTER_LINEAR);

    cv::GaussianBlur(image, image, cv::Size(3, 3), 0);

    // resize card points to image
    std::vector<cv::Point2f> pts;
    for (int i = 0; i < int(card_pts.size()); i++) {
        pts.push_back(cv::Point2f(card_pts[i].x * ratio, card_pts[i].y * ratio));
    }
    // order card points
    this->card_points = orderCardPoints(pts);

}


/**
 * Run the tree detection
 * @param position set 1 to search above card, 2 under card
 * @return 0 if detection is successful, -1 otherwise
 */
int TreeDetection::findTree(int position) {
    // crop image above or under card
    if(position == 1){
        roi = cv::Rect2f(cv::Point2f(0,0), cv::Point2f(resize_to_width, card_points["tl"].y));
    }else{
        roi = cv::Rect2f(cv::Point2f(0,card_points["br"].y), cv::Point2f(resize_to_width, image.rows-1));
    }

    // init tree mask
    image_roi = image(roi);
    tree_mask_roi = cv::Mat::zeros(image_roi.rows, image_roi.cols, CV_8U);

    // tree segmentation
    cv::Point2f center = cv::Point2f(
            (this->card_points["tl"].x + this->card_points["br"].x) / 2,
            (this->card_points["tl"].y + this->card_points["br"].y) / 2);

    doGrabcut(center);

    // fit lines to segmentation
    int ret = findLines();

    return ret;
}


/**
 * Create input mask for graph cut algorithm (green background, foreground behind card).
 * Run grabcut, save output binary tree mask.
 * @param card_center center of the detected card
 */
void TreeDetection::doGrabcut(cv::Point2f card_center) {
    cv::Mat mask(image_roi.rows, image_roi.cols, CV_8U, cv::Scalar::all(cv::GC_PR_BGD));
    cv::Mat bgd_model = cv::Mat();
    cv::Mat fgd_model = cv::Mat();

    // draw wider GC_PR_FGD vertical line in the center of the card
    int line_width = round(0.11 * image_roi.cols);
    for (int i = 0; i < roi.height; i++) {
        for (int j = -line_width; j < line_width; j++) {
            mask.at<uchar>(i, card_center.x + j) = cv::GC_PR_FGD;
        }
    }

    // mask green color as background GC_BGD
    cv::Mat hsv;
    cv::Mat green;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::cvtColor(image_roi, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(38, 55, 55), cv::Scalar(95, 255, 255), green);
    cv::morphologyEx(green, green, cv::MORPH_OPEN, kernel);
    mask.setTo(cv::Scalar::all(cv::GC_BGD), green);

    grabCut(image_roi, mask, cv::Rect(0, 0, image_roi.cols-1, image_roi.rows-1), bgd_model, fgd_model, 5, cv::GC_INIT_WITH_MASK );


    // create a binary mask from the segmentation
    tree_mask_roi = (mask == cv::GC_FGD) | (mask == cv::GC_PR_FGD);
    kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 7));
    cv::morphologyEx(tree_mask_roi, tree_mask_roi, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), 1);
    cv::morphologyEx(tree_mask_roi, tree_mask_roi, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), 1);


    if (TREE_SHOW_IMAGES) {
        // mask foreground pixels of the original image
        cv::Mat masked_foreground_image = cv::Mat();
        cv::bitwise_and(image_roi, image_roi, masked_foreground_image, tree_mask_roi);

        cv::imshow("Grabcut input mask", mask);
        cv::imshow("masked_foreground_image", masked_foreground_image);
        cv::imshow("tree_mask", tree_mask_roi);
        cv::imshow("green", green);
        cv::waitKey(0);
    }

}


/**
 * Check if two lines intersect in the image.
 * @param line1 first line in polar coordinates (Hough output)
 * @param line2 second line in polar coordinates
 * @return 1 if intersect, 0 otherwise
 */
int TreeDetection::lines_intersect(cv::Vec2f line1, cv::Vec2f line2){
    //moves the points to their original position in the uncropped image
    cv::Point2f shift_p(0, roi.y);
    cv::Point2f pt1, pt2, pt3, pt4;
    linePoints(line1, pt1, pt2);
    linePoints(line2, pt3, pt4);
    pt1 += shift_p;
    pt2 += shift_p;
    pt3 += shift_p;
    pt4 += shift_p;

    cv::Point2f x = pt3 - pt1;
    cv::Point2f d1 = pt2 - pt1;
    cv::Point2f d2 = pt4 - pt3;

    float cross = d1.x*d2.y - d1.y*d2.x;
    //if (std::abs(cross) < 1e-8)
    //   return false;

    double t1 = (x.x * d2.y - x.y * d2.x)/cross;
    cv::Point2f r = pt1 + d1 * t1;

    // check if intersect is in the image region
    if (r.x > 0 && r.x < image.cols && r.y > 0 && r.y < image.rows){
        return 1;
    }

    return 0;
}



/**
 * Find lines that respresent tree edges.
 * Uses Canny edge detecor and Hough transformation on tree mask to find lines.
 * @return 0 if lines were found, -1 otherwise
 */
int TreeDetection::findLines(){

    cv::Mat canny_out = cv::Mat::zeros(this->image_roi.size(), CV_8UC1);

    // use canny edge detector on tree mask
    cv::Canny(tree_mask_roi, canny_out, 50, 200, 3);

    // use hough transform on Canny edges to find lines
    std::vector<cv::Vec2f> lines;
    cv::HoughLines(canny_out, lines, 1, CV_PI/180, 50, 0, 0, -1, 1 );
    if (lines.size() <= 1) {
        std::cerr << TAG << ": Couldn't detect tree lines with Hough" << std::endl;
        return -1;
    }

    //cv::imshow("canny_out", canny_out);
    cv::Point2f pt1, pt2, pt3, pt4;
    linePoints(lines[0], pt1, pt2);
    int intersect = 0;
    for (size_t i = 1; i < lines.size(); i++) {
        intersect = lines_intersect(lines[0], lines[i]);
        if (intersect) {
            continue;
        }else{
            linePoints(lines[i], pt3, pt4);
            break;
        }
    }

    if(intersect) {
        std::cerr << TAG << ": Couldn't detect tree lines with Hough. Lines intersect" << std::endl;
        return -1;
    }

    // left line first
    if (pt1.x > pt3.x){
        std::swap(pt1, pt3);
        std::swap(pt2, pt4);
    }
    // move points to original uncropped image position
    cv::Point2f shift_p(0, roi.y);
    left_tree_line = std::make_tuple(pt1+shift_p, pt2+shift_p);
    right_tree_line = std::make_tuple(pt3+shift_p, pt4+shift_p);


    if (TREE_SHOW_IMAGES) {
        cv::imshow("canny_out", canny_out);

        cv::Mat hough_lines = image.clone();
        for (size_t i = 0; i < lines.size(); i++) {
            cv::Point2f pt1, pt2;
            linePoints(lines[i], pt1, pt2);
            cv::line(hough_lines, pt1, pt2, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
        }
        cv::imshow("hough_lines", hough_lines);

        cv::Mat line_output = image.clone();
        cv::line(line_output, std::get<0>(left_tree_line), std::get<1>(left_tree_line), cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
        cv::line(line_output, std::get<0>(right_tree_line), std::get<1>(right_tree_line), cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
        cv::imshow("line_output", line_output);

        cv::waitKey(0);
    }

    return 0;
}



/**
 * Draw points and lines into output image (resized)
 * @return output image
 */
cv::Mat TreeDetection::getOutputImage() {
    cv::Mat output_image = image.clone();
    // draw card points
    cv::circle(output_image, card_points["tl"], 2, cv::Scalar(0, 0, 255), -1);
    cv::circle(output_image, card_points["tr"], 2, cv::Scalar(0, 0, 255), -1);
    cv::circle(output_image, card_points["bl"], 2, cv::Scalar(0, 255, 0), -1);
    cv::circle(output_image, card_points["br"], 2, cv::Scalar(0, 255, 0), -1);
    // draw tree lines
    cv::line(output_image, std::get<0>(this->left_tree_line), std::get<1>(this->left_tree_line), cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    cv::line(output_image, std::get<0>(this->right_tree_line), std::get<1>(this->right_tree_line), cv::Scalar(0, 0, 255), 1, cv::LINE_AA);

    return output_image;
}


/**
 * Creates vector of 4 points representing two tree lines, that correspond to the original image size.
 * Left line first. Top points first.
 * @return tree lines (4 points)
 */
std::vector<cv::Point2f> TreeDetection::getTreeLines() {
    std::vector<cv::Point2f> out;
    out.push_back(std::get<0>(left_tree_line) / ratio);
    out.push_back(std::get<1>(left_tree_line) / ratio);
    out.push_back(std::get<0>(right_tree_line) / ratio);
    out.push_back(std::get<1>(right_tree_line) / ratio);

    return out;
};


/**
 * Compute line points in image (Cartesian coordinate system) from Hough transformation. Mostly for drawing.
 * @param line line in polar coordinate system (Hough transform output)
 * @param pt1 first point of the line (output)
 * @param pt2 secont point of the line (output)
 */
void TreeDetection::linePoints(cv::Vec2f line, cv::Point2f& pt1, cv::Point2f& pt2) {

    float rho = line[0], theta = line[1];
    double a = cos(theta), b = sin(theta);
    double x0 = a * rho, y0 = b * rho;
    pt1.x = cvRound(x0 + 1000 * (-b));
    pt1.y = cvRound(y0 + 1000 * (a));
    pt2.x = cvRound(x0 - 1000 * (-b));
    pt2.y = cvRound(y0 - 1000 * (a));

    if (pt1.y > pt2.y){
        std::swap(pt1,pt2);
    }

    std::tuple<cv::Point2f, cv::Point2f> tree_line(pt1, pt2);

    std::tuple<cv::Point2f, cv::Point2f> top_image_line( cv::Point2f(0,0), cv::Point2f(image.cols-1,0));
    std::tuple<cv::Point2f, cv::Point2f> bottom_image_line( cv::Point2f(0,image.rows-1), cv::Point2f(image.cols-1,image.rows-1));

    cv::Point2f top_intersect = intersection(top_image_line, tree_line);
    cv::Point2f bot_intersect = intersection(bottom_image_line, tree_line);

    pt1 = top_intersect;
    pt2 = bot_intersect;

}

cv::Point2f TreeDetection::intersection(std::tuple<cv::Point2f, cv::Point2f> image_line, std::tuple<cv::Point2f, cv::Point2f> line){
    // Line AB represented as a1x + b1y = c1
    double a1 = std::get<1>(image_line).y - std::get<0>(image_line).y;
    double b1 = std::get<0>(image_line).x - std::get<1>(image_line).x;
    double c1 = a1*(std::get<0>(image_line).x) + b1*(std::get<0>(image_line).y);

    // Line CD represented as a2x + b2y = c2
    double a2 = std::get<1>(line).y - std::get<0>(line).y;
    double b2 = std::get<0>(line).x - std::get<1>(line).x;
    double c2 = a2*(std::get<0>(line).x)+ b2*(std::get<0>(line).y);

    double determinant = a1*b2 - a2*b1;

    double x = (b2*c1 - b1*c2)/determinant;
    double y = (a1*c2 - a2*c1)/determinant;

    return cv::Point2f(x, y);
}


/*************************************************************/



/**
 * Order card points based on distance from origin.
 * @param c_points unordered card points
 * @return dictionary with ordered points, top-left = 'tl', bottom-right = 'br', tr, bl
 */
std::map<std::string, cv::Point2f> orderCardPoints(std::vector<cv::Point2f> c_points) {

    std::vector<float> distances;
    std::map<std::string, cv::Point2f> card_pts;

    // compute distances
    for (cv::Point2f p : c_points) {
        float dist = (p.x * p.x) + (p.y * p.y);
        distances.push_back(dist);
    }
    //sort
    std::vector<float> sorted_distances(distances);
    std::sort(sorted_distances.begin(), sorted_distances.end());
    //save points
    auto it = find(distances.begin(), distances.end(), sorted_distances.at(0));
    card_pts["tl"] = c_points.at(it - distances.begin());

    it = find(distances.begin(), distances.end(), sorted_distances.at(1));
    card_pts["bl"] = c_points.at(it - distances.begin());

    it = find(distances.begin(), distances.end(), sorted_distances.at(2));
    card_pts["tr"] = c_points.at(it - distances.begin());

    it = find(distances.begin(), distances.end(), sorted_distances.at(3));
    card_pts["br"] = c_points.at(it - distances.begin());

    if (card_pts["bl"].x > card_pts["tr"].x) {
        std::swap(card_pts["bl"], card_pts["tr"]);
    }

    return card_pts;
}


/**
 * Distance between two points.
 * @param p1 first point
 * @param p2 second point
 * @return distance
 */
double pointsDistance(cv::Point2f p1, cv::Point2f p2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    return sqrt(dx * dx + dy * dy);
}


/**
 * Calculate the distance between the point and the line.
 * @param line_start first point of the line
 * @param line_end second point of the line
 * @param point point
 * @return computed distance
 */
double distanceToLine(cv::Point2f line_start, cv::Point2f line_end, cv::Point2f point) {

    cv::Point2f line = line_end - line_start;

    float t = (point - line_start).dot(line_end - line_start) /
              (line.x * line.x + line.y * line.y);

    cv::Point2f closest = cv::Point2f(line_start.x + t * line.x, line_start.y + t * line.y);
    line.x = point.x - closest.x;
    line.y = point.y - closest.y;

    return sqrt(line.x * line.x + line.y * line.y);

}


/**
 * Mask card area in input image. You can set margin to enlarge the area.
 * @param points ordered card points
 * @param margin the number of pixels by which the area on each side increases
 * @param input input image to mask
 * @return image with white mask of card
 */
cv::Mat maskCard(std::map<std::string, cv::Point2f> points, cv::Mat input, int margin) {

    std::vector<cv::Point> pts;
    pts.push_back(cv::Point(points["tl"].x - margin, points["tl"].y - margin));
    pts.push_back(cv::Point(points["tr"].x + margin, points["tr"].y - margin));
    pts.push_back(cv::Point(points["br"].x + margin, points["br"].y + margin));
    pts.push_back(cv::Point(points["bl"].x - margin, points["bl"].y + margin));

    cv::fillPoly(input, std::vector<cv::Point>{pts}, cv::Scalar(255));

    return input;
}