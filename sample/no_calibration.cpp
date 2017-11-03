#include <opencv2\opencv.hpp>
#include<opencv2\calib3d.hpp>
#include<opencv2\imgcodecs.hpp>
#include<opencv2\highgui.hpp>
#include<iostream>
#include<sstream>
using namespace std;
using namespace cv;

int main(int argc, char** argv) {
	
	string filename_leftImg = "imL.png";
	string filename_rightImg = "imR.png";

	Mat sceneImg1 = imread(filename_leftImg, IMREAD_GRAYSCALE);
	Mat sceneImg2 = imread(filename_rightImg, IMREAD_GRAYSCALE); //グレースケール

	if (sceneImg1.empty() || sceneImg2.empty()) {
		std::cout << " --(!) Error reading images " << std::endl; return -1;
	}

										//stereo
	Mat disp,disp8;
	Ptr<StereoBM> bm = StereoBM::create();
	int numberOfDisparities,blockSize;
	Rect roi1, roi2;
	//ブロックマッチングのパラメータを決定する
	//numberOfDisparities = numberOfCheckerPatterns > 0 ? numberOfDisparities : sceneImg1.cols / 8;
	numberOfDisparities = 16 *5; //16の倍数でないといけない、こいつが大きくなると画面がなぜか狭まる？
	blockSize = 5;//大きいと滑らかになるけど正確ではない、小さいとちゃんと働かないかもしれない
	bm->setROI1(roi1);
	bm->setROI2(roi2);
	bm->setPreFilterCap(31);
	bm->setBlockSize(blockSize);
	bm->setMinDisparity(0);
	bm->setNumDisparities(numberOfDisparities);
	bm->setTextureThreshold(10);
	bm->setUniquenessRatio(15);
	bm->setSpeckleWindowSize(100);
	bm->setSpeckleRange(32);
	bm->setDisp12MaxDiff(1);

	//視差画像の計算
	bm->compute(sceneImg1, sceneImg2, disp);
	//dispが右にずれちゃってる　なんで？
	//calibration無しでもずれてる

	//視差の最大値と最小値を出す
	double minVal; double maxVal;
	minMaxLoc(disp, &minVal, &maxVal);
	printf("Min disp: %f Max value: %f \n", minVal, maxVal);
	disp.convertTo(disp8, CV_8U, 255 / (maxVal - minVal));
	imshow("Diparity", disp);
	waitKey(0);

	return 0;
}