#include "TreeDiameter.h"

/**
 * Compute tree diameter.
 * @param treePts 4 tree points which represent 2 lines above card
 * @param cardPts 4 card points
 * @return diameter of tree
 */
float getTreeWidth( std::vector<cv::Point2f> treePts, std::vector<cv::Point2f> cardPts) {

    // treePts[0] - top coordinate of left line
    // treePts[1] - bottom coordinate of left line
    // treePts[2] - top coordinate of right line
    // treePts[3] - bottom coordinate of right line

    //check if left line is really left
    if (treePts[0].x > treePts[1].y) {  //if not flip them
        cv::Point2f temp0 = treePts[0];
        cv::Point2f temp1 = treePts[1];
        treePts[0] = treePts[2];
        treePts[1] = treePts[3];
        treePts[2] = temp0;
        treePts[3] = temp1;
    }

    // both tree lines must point from top to bottom
    if (treePts[0].y > treePts[1].y) {
        cv::Point2f temp = treePts[0];
        treePts[0] = treePts[1];
        treePts[1] = temp;
    }
    if (treePts[2].y > treePts[3].y) {
        cv::Point2f temp = treePts[2];
        treePts[2] = treePts[3];
        treePts[3] = temp;
    }

    // compute angle in the middle of tree lines
    float angle1 = atan2(treePts[0].y - treePts[1].y, treePts[0].x - treePts[1].x);
    float angle2 = atan2(treePts[2].y - treePts[3].y, treePts[2].x - treePts[3].x);
    float angle_middle_line = std::min(angle1, angle2) + (abs(angle1 - angle2) / 2);


    // compute first point of line in the middle
    cv::Point2f startPt, endPt;
    if (int(toDegrees(angle1)) == int(toDegrees(angle2))) {   // tree lines are parallel
        std::vector<cv::Point2f> perp = getPerpendicularInInterSc(treePts[2], treePts[3]);
        cv::Point2f i1 = line_intersection(perp[0], perp[1], treePts[2], treePts[3]);
        cv::Point2f i2 = line_intersection(perp[0], perp[1], treePts[0], treePts[1]);
        startPt = cv::Point2f(((i2.x + i1.x) / 2), ((i2.y + i1.y) / 2));
    }
    else {
        startPt = line_intersection(treePts[0], treePts[1], treePts[2], treePts[3]);
    }

    //compute second point of line in the middle
    endPt.y = startPt.y - (50000 * sin(angle_middle_line));
    endPt.x = startPt.x - (50000 * cos(angle_middle_line));

    // intersection of middle line and card
    cv::Point2f inSc = line_intersection(cardPts[0], cardPts[1], startPt, endPt);

    // find perpendicular line to middle line in intersection
    std::vector<cv::Point2f> perp = getPerpendicularInInterSc(startPt, inSc);

    // find final points. they are intersections between tree lines and perpendicular line
    cv::Point2f final1 = line_intersection(perp[0], perp[1], treePts[0], treePts[1]);
    cv::Point2f final2 = line_intersection(perp[0], perp[1], treePts[2], treePts[3]);


    // DRAW
    // tree lines 
    /**std::vector<cv::Point2f> ex_line;
    ex_line = extendLine(treePts[0], treePts[1], image.rows, image.cols);
    line(image, ex_line[0], ex_line[1], Scalar(0, 255, 0), 5);
    ex_line = extendLine(treePts[2], treePts[3], image.rows, image.cols);
    line(image, ex_line[0], ex_line[1], Scalar(0, 255, 0), 5);

    // card lines
    line(image, cardPts[0], cardPts[1], Scalar(255, 0, 0), 5);
    line(image, cardPts[1], cardPts[2], Scalar(255, 0, 0), 5);
    line(image, cardPts[2], cardPts[3], Scalar(255, 0, 0), 5);
    line(image, cardPts[3], cardPts[0], Scalar(255, 0, 0), 5);

    // middle line
    ex_line = extendLine(startPt, endPt, image.rows, image.cols);
    line(image, ex_line[0], ex_line[1], Scalar(255, 255, 0), 5);

    // perp line
    ex_line = extendLine(perp[0], perp[1], image.rows, image.cols);
    line(image, ex_line[0], ex_line[1], Scalar(0, 255, 255), 5);

    // card top line
    ex_line = extendLine(cardPts[0], cardPts[1], image.rows, image.cols);
    line(image, ex_line[0], ex_line[1], Scalar(255, 0, 255), 5);

    // final points - tree width
    circle(image, final1, 20, Scalar(0, 0, 255), -1);
    circle(image, final2, 20, Scalar(0, 0, 255), -1);

    measure_points.push_back(final1);
    measure_points.push_back(final2);
    if (measure_points.at(0).x > measure_points.at(1).x) {
        std::swap(measure_points.at(0), measure_points.at(1));
    }**/

    float distance = distBetweenPoints(final1, final2);
    return distance;
}

/**
 * Compute distance between 2 points.
 * @param p1 first point
 * @param p2 second point
 * @return distance
 */
float distBetweenPoints(cv::Point2f p1, cv::Point2f p2) {
    return sqrt(((p1.x - p2.x) * (p1.x - p2.x)) + ((p1.y - p2.y) * (p1.y - p2.y)));
}

/**
 * Find perpendicular line to line (startPt, inSc). This perpendicular line intersect point inSc.
 * @param startPt first point of line
 * @param inSc second point of line and also point where perpendiclar line crosses input line
 * @return vector of 2 points - perpendical line
 */
std::vector<cv::Point2f> getPerpendicularInInterSc(cv::Point2f startPt, cv::Point2f inSc) {
    float vX = (inSc.x) - (startPt.x);
    float vY = (inSc.y) - (startPt.y);

    float mag = sqrt(vX * vX + vY * vY);
    vX = vX / mag;
    vY = vY / mag;
    float temp = vX;
    vX = 0 - vY;
    vY = temp;

    int len = 5000;
    float cX = inSc.x + vX * len;
    float cY = inSc.y + vY * len;
    float dX = inSc.x - vX * len;
    float dY = inSc.y - vY * len;

    std::vector<cv::Point2f> ret;
    ret.push_back(cv::Point2f((cX), (cY)));
    ret.push_back(cv::Point2f((dX), (dY)));
    return (ret);
}

/**
 * Convert degrees to radians.
 * @param degree angle in degrees
 * @return angle in radians
 */
float toRadians(float degree) {
    return float(degree * (CV_PI / 180));
}

/**
 * Convert radians to degrees.
 * @param radians angle in radians
 * @return angle in degrees
 */
float toDegrees(float radians) {
    return float(radians * (180 / CV_PI));
}

/**
 * Find intersection of 2 lines.
 * @param A first point of first line
 * @param B second point of first line
 * @param C first point of second line
 * @param D second point of second line
 * @return intersection point
 */
cv::Point2f line_intersection(cv::Point2f A, cv::Point2f B, cv::Point2f C, cv::Point2f D) {
    // Line AB represented as a1x + b1y = c1 
    float a1 = B.y - A.y;
    float b1 = A.x - B.x;
    float c1 = a1 * (A.x) + b1 * (A.y);

    // Line CD represented as a2x + b2y = c2 
    float a2 = D.y - C.y;
    float b2 = C.x - D.x;
    float c2 = a2 * (C.x) + b2 * (C.y);

    float determinant = a1 * b2 - a2 * b1;
    if (determinant == 0)
    {
        // The lines are parallel. There is no intersection.
        return cv::Point2f(0, 0);
    }
    else
    {
        float x = (b2 * c1 - b1 * c2) / determinant;
        float y = (a1 * c2 - a2 * c1) / determinant;
        return cv::Point2f((x), (y));
    }
}


/**
 * This function extend line to intersect the whole image. It is used only for drawing.
 * @param l1 first point of line
 * @param l2 second point of line
 * @param h height of image
 * @param w width of image
 * @return 2 points - line that intersect whole image
 */
std::vector<cv::Point2f> extendLine(cv::Point2f l1, cv::Point2f l2, int h, int w) {
    float nx = l2.x - l1.x;
    float ny = l2.y - l1.y;

    if (nx == 0) {     //vertical line

        std::vector<cv::Point2f> ret;
        ret.push_back(cv::Point2f(l1.x, 0));
        ret.push_back(cv::Point2f(l1.x, float(h)));
        return (ret);
    }

    float x = 0;
    float t = float((x - l1.x)) / nx;
    float y_e1 = l1.y + ny * t;

    x = float(w);
    t = float((x - l1.x)) / nx;
    float y_e2 = l1.y + ny * t;

    std::vector<cv::Point2f> ret;
    ret.push_back(cv::Point2f(0, (y_e1)));
    ret.push_back(cv::Point2f(float(w), (y_e2)));

    return (ret);
}

