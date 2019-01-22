#include "opencv2\highgui.hpp"
#include "opencv2\imgproc.hpp"
#include <opencv2\opencv.hpp>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <cmath>

//-----Algorithm to detect vanishing point in photos
//-----Works best under release mode, needs x64
//-----Makes use of opencv_contrib_master libraries, will probably fail without

using namespace std;
cv::Mat image, image_grey;
cv::Mat image_Cannymask, image_edges;
cv::Mat image_blurred;
cv::Mat image_colour;
float houghlines_divisor = 5; //Should generally be larger for large images and fewer vanishing lines
float min_crossing_angle = 1; //Minimum angle lines must cross at to be considered a vanishing point (in radians)


int main(int argc, char** argv) {
	//-----Get Image-----
	//cv::VideoCapture capture(filename); //Uncomment when working with video
	//also need catch if capture fails
	//string filename = "traintracks.jpeg";	//Works well with houghlines_divisor at 6
	//string filename = "tracks1.jpeg";	//Divisor of 5 or 6 (6 shows lots of false positive Hough lines but rejects crossing points
	//string filename = "tracks2.jpg";	//All crossing points falsely rejected
	//string filename = "tracks3.jpg"; //Performs poorly, probably due to aspect ratio
	//string filename = "tracks4.jpg";	//Performs poorly, due to horizon not being horizontal
	//string filename = "perspective1.jpg";
	//string filename = "perspective2.jpg";
	//string filename = "street1.jpg";
	string filename = "nystreet.jpg";	//Works well with houghlines_divisor at 5
	image = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
	if (!image.data) { return 0; }	//Make sure program read something
	
	//-----Resize image-----	Note that for this to take effect project must be cleaned before being built
	float width = image.cols;
	float scaleFactor = 1000 / width;
	cv::resize(image, image, cv::Size(), scaleFactor, scaleFactor, CV_INTER_LINEAR);

	//-----Greyscale image-----
	cv::cvtColor(image, image_grey, CV_BGR2GRAY);
	
	//-----Blur image to reduce noise-----
	cv::blur(image_grey, image_blurred, cv::Size(3, 3));
	//-----Setting the thresholds automatically-----
	//Get the average pixel value of a grey image, find the standard deviation
	cv::Mat image_greycopy; image_grey.copyTo(image_greycopy); 	//First, copy greyscale to new mat
	std::vector<double> vecFromMat;
	image_greycopy = image_greycopy.reshape(0, 1); 	//Rearrange into row matrix
	image_greycopy.copyTo(vecFromMat);
	//And let the fun being
	std::nth_element(vecFromMat.begin(), vecFromMat.begin() + vecFromMat.size() / 2, vecFromMat.end()); 	//Find median average of greyscale

	float median = vecFromMat[vecFromMat.size() / 2];
	float low_thresh, high_thresh, std_dev = 0.34;
	low_thresh = median / std_dev;
	high_thresh = median * std_dev;
	//-----Perform edge detection-----
	cv::Canny(image_blurred, image_Cannymask, low_thresh, high_thresh, 3);
	image_edges = cv::Scalar::all(0); //create completely black window
	image_grey.copyTo(image_edges, image_Cannymask);	//copyTo copies 'image' onto 'dst'. It only copies non-zero pixels (so everything black is left out)
	cv::cvtColor(image_Cannymask, image_colour, CV_GRAY2BGR);
	cv::imshow("Canny", image_colour);

	//-----Standard Hough Transform-----
	vector<cv::Vec2f> lines;
	float hough_threshold = (sqrt(((image.rows*image.rows) + (image.cols*image.cols))) / houghlines_divisor); //Set threshold as a fraction of the image diagonal size. Still needs tweaking for each image
	HoughLines(image_edges, lines, 1, CV_PI / 180, hough_threshold, 0, 0);
	size_t i = 0;
	for (i; i < lines.size(); i++) {
		float rho = lines[i][0], theta1 = lines[i][1];
		float theta_thresh = 0.2;
		cv::Point pt1, pt2;
		cv::Point pt3, pt4;
		cv::Point intersection;
		float costheta1, costheta2, sintheta1, sintheta2, rho1, rho2;
		//Reject horizontal and vertical lines
		if ((theta1 >(0 + theta_thresh)) && (theta1 < (CV_PI / 2 - theta_thresh)) || (theta1 >((CV_PI / 2) + theta_thresh)) && (theta1 < (CV_PI - theta_thresh))) {
			double a = cos(theta1), b = sin(theta1);
			double x0 = a * rho, y0 = b * rho;
			//Draw detected lines on image
			pt1.x = cvRound(x0 + 1000 * (-b));
			pt1.y = cvRound(y0 + 1000 * (a));
			pt2.x = cvRound(x0 - 1000 * (-b));
			pt2.y = cvRound(y0 - 1000 * (a));
			line(image_colour, pt1, pt2, cv::Scalar(0, 0, 255), 1, CV_AA);
			costheta1 = cos(theta1);
			sintheta1 = sin(theta1);
			rho1 = rho;
			size_t j = i + 1;
			//Check every combination of two lines for crossings
			for (j; j < lines.size(); j++) {
				double rho = lines[j][0], theta2 = lines[j][1];
				costheta2 = cos(theta2);
				sintheta2 = sin(theta2);
				rho2 = rho;
				if ((abs(theta1) - abs(theta2)) < min_crossing_angle) {break;} 				//Reject lines that lie over the top of each; we only want lines that intersect
				float determinative = (costheta1 * sintheta2) - (sintheta1 * costheta2);
				intersection.x = ((sintheta2 * rho1) - (sintheta1 * rho2)) / determinative;
				intersection.y = ((-costheta2 * rho1) + (costheta1*rho2)) / determinative;
				cv::drawMarker(image_colour, intersection, cv::Scalar(255, 0, 0), cv::MARKER_CROSS, 50, 1, 8);
				cout << intersection << "\n";
			}
		}
	}

	cv::imshow("orig", image);
	cv::imshow("Lines", image_colour);

	cv::waitKey(0);
	return(0);
}