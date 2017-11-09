/*
*@ stereo calibration and stereo matching
*@ prease prepare more than five images of checkerboads and one image to measure depth 
*/

#include <opencv2\opencv.hpp>
#include<opencv2\calib3d.hpp>
#include<opencv2\imgcodecs.hpp>
#include<opencv2\highgui.hpp>
#include<iostream>
#include<sstream>
using namespace std;
using namespace cv;

int main(int argc, char** argv) {
	const int numberOfCheckerPatterns = 5; //�`�F�b�J�[�p�^�[���̖���
	vector<Mat> checkerImg1, checkerImg2;  //�`�F�b�J�[�p�^�[���摜
	string filename_leftImg = "../depth_picture/imL.jpg";
	string filename_rightImg = "../depth_picture/imR.jpg";

	Mat sceneImg1 = imread(filename_leftImg,IMREAD_GRAYSCALE);
	Mat sceneImg2 = imread(filename_rightImg,IMREAD_GRAYSCALE); //�O���[�X�P�[��

	if (sceneImg1.empty() || sceneImg2.empty()){
		std::cout << " --(!) Error reading images " << std::endl; return -1;
	}
	
	Size imageSize;
	const Size patternSize(9, 6); //�`�F�b�J�[�p�^�[���̌�_�̐�
	const int MARKER_SIZE = 10; //�}�[�J�[��1�}�X�̃T�C�Y(mm)

	vector<vector<Point3f> > worldPoints(numberOfCheckerPatterns); //�`�F�b�J�[��_���W�ƑΉ����鐢�E���W�̒l���i�[����s��
	vector<vector<Point2f> > imagePoints1(numberOfCheckerPatterns);//�`�F�b�J�[��_���W���i�[
	vector<vector<Point2f> > imagePoints2(numberOfCheckerPatterns);//�`�F�b�J�[��_���W���i�[
	TermCriteria criteria(TermCriteria::MAX_ITER | TermCriteria::EPS, 20, 0.001);//�Ή����郏�[���h���W�n�p�^�[��

																				 //�J�����p�����[�^�s��
	Mat cameraMatrix1, cameraMatrix2;	//�����p�����[�^�s��
	Mat distCoeffs1, distCoeffs2;		//�����Y�c�ݍs�� 
	Mat R, T, E, F, R1, R2, P1, P2, Q;	//2�჌���Y�̊Ԃ̊֌W��ێ�����s��Q

	//stereo
	Mat disp;
	Mat	disp8;
	Rect roi1, roi2;
	Mat xyz;
	int blockSize = 0, numberOfDisparities = 0,MinDisparity=0;
	Ptr<StereoBM> bm = StereoBM::create();

	//�u���b�N�}�b�`���O�̃p�����[�^�����肷��
	//numberOfDisparities = numberOfCheckerPatterns > 0 ? numberOfDisparities : sceneImg1.cols / 8;
	numberOfDisparities = 16*5; //16�̔{���łȂ��Ƃ����Ȃ��A�������傫���Ȃ�Ɖ�ʂ����܂� ���E�̃J�����̌�����͈͂������H
	blockSize =5;//�傫���Ɗ��炩�ɂȂ邯�ǐ��m�ł͂Ȃ�
	bm->setROI1(roi1);
	bm->setROI2(roi2);
	bm->setPreFilterCap(31);
	bm->setBlockSize(blockSize);
	bm->setMinDisparity(MinDisparity);
	bm->setNumDisparities(numberOfDisparities);
	bm->setTextureThreshold(10);
	bm->setUniquenessRatio(15);
	bm->setSpeckleWindowSize(100);
	bm->setSpeckleRange(32);
	bm->setDisp12MaxDiff(1);
	
	//���E���W�����߂�
	for (int i = 0; i < numberOfCheckerPatterns; i++) {
		for (int j = 0; j < patternSize.area(); j++) {
			worldPoints[i].push_back(Point3f(static_cast<float>(j%patternSize.width * MARKER_SIZE),
				static_cast<float>(j / patternSize.width * MARKER_SIZE),
				0.0));
		}
	}

	//2�ᕪ�̃`�F�b�J�[�p�^�[���摜��ǂݍ���
	for (int i = 0; i < numberOfCheckerPatterns; i++) {
		stringstream stream1, stream2;
		stream1 <<"../checkBoard/"<< i + 1 <<"L"<< ".jpg";
		stream2 <<"../checkBoard/"<< i + 1 <<"R"<< ".jpg";
		string fileName1 = stream1.str();
		string fileName2 = stream2.str();
		checkerImg1.push_back(imread(fileName1, 0));
		checkerImg2.push_back(imread(fileName2, 0));
	}

	//�摜�T�C�Y�𓾂�
	imageSize = Size(checkerImg1[0].cols, checkerImg1[0].rows);

	//�`�F�b�N�p�^�[���̌�_���W�����߁AimagePoints�Ɋi�[����
	for (int i = 0; i < numberOfCheckerPatterns; i++) {
		cout << "Find corners from image" << i + 1;\
		if (findChessboardCorners(checkerImg1[i], patternSize, imagePoints1[i]) &&
			findChessboardCorners(checkerImg2[i], patternSize, imagePoints2[i])) {
			cout << "...All corners found." << endl;
			cornerSubPix(checkerImg1[i], imagePoints1[i], Size(11, 11),
				Size(-1, -1), criteria);
			cornerSubPix(checkerImg2[i], imagePoints2[i], Size(11, 11),
				Size(-1, -1), criteria);
		}
		else {
			cout << "...at least 1 corner not found." << endl;
			waitKey(0);
			return -1;
		}
	}

	//2��X�e���I�J�����𓯎��ɃL�����u���[�V����
	stereoCalibrate(worldPoints, imagePoints1, imagePoints2,
		cameraMatrix1, distCoeffs1,
		cameraMatrix2, distCoeffs2,
		imageSize, R, T, E, F);

	//Rectification
	stereoRectify(
		cameraMatrix1, distCoeffs1,
		cameraMatrix2, distCoeffs2,
  		imageSize, R, T, R1, R2, P1, P2, Q);
	
	FileStorage fs("../Q_value.xml", FileStorage::WRITE);
	fs << "Q_value" << Q;
	fs.release();

	//�����摜�̌v�Z
	bm->compute(sceneImg1, sceneImg2, disp);

	//�����̍ő�l�ƍŏ��l���o��
	double minVal; double maxVal;
	minMaxLoc(disp, &minVal, &maxVal);
	printf("Min disp: %f Max value: %f \n", minVal, maxVal);
	
	//�����摜�������ɕϊ�
	reprojectImageTo3D(disp, xyz, Q, true);

	//�����摜��ϊ����ĕ\��
	//�@numberOfDisparities*16.�Ɠ������ʂɂȂ���
	disp.convertTo(disp8, CV_8U, 255 /(maxVal-minVal));
	//�A���ȏ��ʂ�
	//disp.convertTo(disp8, CV_8U, 255 / (numberOfDisparities*16.));
	imshow("Disparity Map", disp8);
	waitKey(0);
	imwrite("disparity.bmp", disp8);
	
	return 0;
}