#include <opencv2\opencv.hpp>
#include<opencv2\calib3d.hpp>
#include<opencv2\imgcodecs.hpp>
#include<opencv2\highgui.hpp>
#include<iostream>
#include<sstream>
using namespace std;
using namespace cv;

int main(int argc, char** argv) {
	
	string filename_leftImg = "../depth_picture/imL.jpg";
	string filename_rightImg = "../depth_picture/imR.jpg";

	Mat sceneImg1 = imread(filename_leftImg, IMREAD_GRAYSCALE);
	Mat sceneImg2 = imread(filename_rightImg, IMREAD_GRAYSCALE); //�O���[�X�P�[��

	if (sceneImg1.empty() || sceneImg2.empty()) {
		std::cout << " --(!) Error reading images " << std::endl; return -1;
	}

										//stereo
	Mat disp,disp8;
	Ptr<StereoBM> bm = StereoBM::create();
	int numberOfDisparities,blockSize;
	Rect roi1, roi2;
	//�u���b�N�}�b�`���O�̃p�����[�^�����肷��
	//numberOfDisparities = numberOfCheckerPatterns > 0 ? numberOfDisparities : sceneImg1.cols / 8;
	numberOfDisparities = 16 *5; //16�̔{���łȂ��Ƃ����Ȃ��A�������傫���Ȃ�Ɖ�ʂ��Ȃ������܂�H
	blockSize = 5;//�傫���Ɗ��炩�ɂȂ邯�ǐ��m�ł͂Ȃ��A�������Ƃ����Ɠ����Ȃ���������Ȃ�
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

	//�����摜�̌v�Z
	bm->compute(sceneImg1, sceneImg2, disp);

	//�����̍ő�l�ƍŏ��l���o��
	double minVal; double maxVal;
	minMaxLoc(disp, &minVal, &maxVal);
	printf("Min disp: %f Max value: %f \n", minVal, maxVal);

	//�����摜�������ɕϊ�
	Mat xyz;
	Mat Q;
	FileStorage fs("../Q_value.xml", FileStorage::READ);
	FileNode node(fs.fs, NULL);
	read(node["Q_value"], Q);
	reprojectImageTo3D(disp, xyz, Q, true);

	//�����摜��ϊ����ĕ\��
	//�@numberOfDisparities*16.�Ɠ������ʂɂȂ���
	//disp.convertTo(disp8, CV_8U, 255 /(maxVal-minVal));
	//�A���ȏ��ʂ�
	disp.convertTo(disp8, CV_8U, 255 / (numberOfDisparities*16.));
	imshow("Disparity Map", disp8);
	waitKey(0);
	imwrite("disparity.bmp", disp8);


	return 0;
}