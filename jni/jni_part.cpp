#include <jni.h>
#include "opencv2/core/core.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/contrib/contrib.hpp"
#include <stdio.h>
#include <vector>
#include <stdio.h>

using namespace std;
using namespace cv;

int getDisp(Mat g1, Mat g2, Mat &disp);
int getThreshold(Mat img, Point p1, int range, Mat &foreground);
int segmentForeground(Mat img, Mat &foreground, Mat &background);
int getBlurMaskedImage(Mat img, Mat &foreground);
int getMaskedImage(Mat img, Mat &foreground);
int addFgBg(Mat foreground, Mat background, Mat &img);
int getBlurMaskedGrayImage(Mat img, Mat &foreground);
int getMaskedGrayImage(Mat img, Mat &foreground);

int getGray(Mat& img)
{
	cvtColor(img, img, CV_BGR2GRAY);
	return 1;
}
extern "C" {
JNIEXPORT void JNICALL Java_com_example_imagetest_MainActivity_getThreshold(JNIEnv*, jobject, jlong addrBgr, jlong addrDisp, jlong finalImage, jint ji1, jint ji2);
JNIEXPORT void JNICALL Java_com_example_imagetest_MainActivity_getDisparity(JNIEnv*, jobject, jlong addrRgba, jlong finalImage);

JNIEXPORT void JNICALL Java_com_example_imagetest_MainActivity_getDisparity(JNIEnv*, jobject, jlong addrRgba, jlong finalImage)
{
    Mat& img = *(Mat*)addrRgba;
    Mat g1, g2;
    Mat& disp = *(Mat*)finalImage;
    cvtColor(img, img, CV_RGBA2BGR);
    Mat img1(img, Rect(0, 0, img.cols/2, img.rows));
    Mat img2(img, Rect(img.cols/2, 0, img.cols/2, img.rows));
    cvtColor(img1, g1, CV_BGR2GRAY);
    cvtColor(img2, g2, CV_BGR2GRAY);
    getDisp(g1, g2, disp);
    //cvtColor(finImg, finImg, CV_BGR2RGBA);
    return;
}
JNIEXPORT void JNICALL Java_com_example_imagetest_MainActivity_getThreshold(JNIEnv*, jobject, jlong addrBgr, jlong addrDisp, jlong finalImage, jint ji1, jint ji2)
{
	Mat& img = *(Mat*)addrBgr;
	Mat& disp = *(Mat*)addrDisp;
	Mat background;
	Mat& finImg = *(Mat*)finalImage;
	Mat img1(img, Rect(0, 0, img.cols/2, img.rows));
    Mat img2(img, Rect(img.cols/2, 0, img.cols/2, img.rows));
	Point point1;
    int x, y;
    x = ji1;
    y = ji2;
    point1 = Point(x, y); // to get from android
    getThreshold(disp, point1, 10, finImg);
    segmentForeground(img1, finImg, background);
    getMaskedGrayImage(img1, background);
    getMaskedImage(img1, finImg);
    imwrite("/mnt/sdcard/SimpleImageCapture/img_fg.jpg", finImg);
    imwrite("/mnt/sdcard/SimpleImageCapture/img_bg.jpg", background);
    //getMaskedImage(img1, foreground);
    addFgBg(finImg, background, finImg);
    imwrite("/mnt/sdcard/SimpleImageCapture/img_fin.jpg", finImg);
    return;
}
}


int getDisp(Mat g1, Mat g2, Mat &disp)
{
    Mat disp16;
    StereoSGBM sbm;
    sbm.SADWindowSize = 7; // 5
    sbm.numberOfDisparities = 192;
    sbm.preFilterCap = 4;
    sbm.minDisparity = -32; // -64
    sbm.uniquenessRatio = 9; // 1
    sbm.speckleWindowSize = 180; //150
    sbm.speckleRange = 2;
    sbm.disp12MaxDiff = 20; // 10
    sbm.fullDP = false;
    sbm.P1 = 600;
    sbm.P2 = 2400;
    /*
    sbm.SADWindowSize = 5; // 5
    sbm.numberOfDisparities = 112;
    sbm.preFilterCap = 61;
    sbm.minDisparity = -39; // -64
    sbm.uniquenessRatio = 1; // 1
    sbm.speckleWindowSize = 180; //150
    sbm.speckleRange = 2;
    sbm.disp12MaxDiff = 20; // 10
    sbm.fullDP = false;
    sbm.P1 = 600;
    sbm.P2 = 2400;
    */
    sbm(g1, g2, disp16);
    normalize(disp16, disp, 0, 255, CV_MINMAX, CV_8U);
    if (disp.cols > 0 && disp.rows > 0)
    {
        return 1;
    }
    return 0;
}

int getThreshold(Mat img, Point p1, int range, Mat &foreground)
{
    int disval;
    disval = img.at<uchar>(p1.y, p1.x);
    inRange(img, disval - range, disval + range, foreground);
    medianBlur(foreground, foreground, 9);
    return 1;
}

int segmentForeground(Mat img, Mat &foreground, Mat &background)
{
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Mat drawing, kernel;
    int size=3;
    drawing = Mat::zeros(img.size(), CV_8UC3);
    findContours(foreground.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
    for (int i=0; i<contours.size(); i++)
    {
        if (contourArea(contours[i]) > 15000)
        {
            drawContours(drawing, contours, i, Scalar(255, 255, 255), CV_FILLED, 8, hierarchy, 0, Point());
        }
    }

    kernel = getStructuringElement(MORPH_ELLIPSE, Size(2*size+1, 2*size+1), Point(size, size));
    erode(drawing, drawing, kernel, Point(-1, -1), 2);
    Mat temp;
    dilate(drawing, temp, kernel, Point(-1, -1), 1);
    drawing = Mat::zeros(img.size(), CV_8UC3);
    contours.clear();
    hierarchy.clear();
    cvtColor(temp, temp, CV_BGR2GRAY);
    findContours(temp.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
    for (int i=0; i<contours.size(); i++)
    {
        if (contourArea(contours[i]) > 15000)
        {
            drawContours(drawing, contours, i, Scalar(255, 255, 255), CV_FILLED, 8, hierarchy, 0, Point());
        }
    }
    dilate(drawing, drawing, kernel, Point(-1, -1), 1);
    foreground = drawing.clone();
    background = Scalar(255, 255, 255) - foreground;
    return 1;
}

int getBlurMaskedGrayImage(Mat img, Mat &foreground)
{
    Mat blur, blurGray;
    cvtColor(img, blurGray, CV_BGR2GRAY);
    GaussianBlur(blurGray, blurGray, Size(9, 9), 11, 11);
    vector<Mat> gray;
    gray.push_back(blurGray);
    gray.push_back(blurGray);
    gray.push_back(blurGray);
    merge(gray, blur);
    bitwise_and(blur, foreground, foreground);
    //bilateralFilter(foreground, newImg, 20, 100.0, 150.0, BORDER_DEFAULT);
    //GaussianBlur(newImg, foreground, Size(9, 9), 11, 11);
    //foreground = newImg.clone();
    return 1;
}
int getBlurMaskedImage(Mat img, Mat &foreground)
{
    Mat blur;
    GaussianBlur(img, blur, Size(9, 9), 11, 11);
    bitwise_and(blur, foreground, foreground);
    return 1;
}
int getMaskedImage(Mat img, Mat &foreground)
{
    bitwise_and(img, foreground, foreground);
    return 1;
}

int getMaskedGrayImage(Mat img, Mat &foreground)
{
    Mat g, bitwise;
    cvtColor(img, g, CV_BGR2GRAY);
    vector<Mat> gray;
    gray.push_back(g);
    gray.push_back(g);
    gray.push_back(g);
    merge(gray, bitwise);
    bitwise_and(bitwise, foreground, foreground);
    return 1;
}

int addFgBg(Mat foreground, Mat background, Mat &img)
{
    Mat tempImg;
    add(foreground, background, img);
    return 1;
}
