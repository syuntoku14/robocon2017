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
	const int numberOfCheckerPatterns = 5; //チェッカーパターンの枚数
	vector<Mat> checkerImg1, checkerImg2;  //チェッカーパターン画像
	string filename_leftImg = "../depth_picture/imL.jpg";
	string filename_rightImg = "../depth_picture/imR.jpg";

	Mat sceneImg1 = imread(filename_leftImg,IMREAD_GRAYSCALE);
	Mat sceneImg2 = imread(filename_rightImg,IMREAD_GRAYSCALE); //グレースケール

	if (sceneImg1.empty() || sceneImg2.empty()){
		std::cout << " --(!) Error reading images " << std::endl; return -1;
	}
	
	Size imageSize;
	const Size patternSize(9, 6); //チェッカーパターンの交点の数
	const int MARKER_SIZE = 10; //マーカーの1マスのサイズ(mm)

	vector<vector<Point3f> > worldPoints(numberOfCheckerPatterns); //チェッカー交点座標と対応する世界座標の値を格納する行列
	vector<vector<Point2f> > imagePoints1(numberOfCheckerPatterns);//チェッカー交点座標を格納
	vector<vector<Point2f> > imagePoints2(numberOfCheckerPatterns);//チェッカー交点座標を格納
	TermCriteria criteria(TermCriteria::MAX_ITER | TermCriteria::EPS, 20, 0.001);//対応するワールド座標系パターン

																				 //カメラパラメータ行列
	Mat cameraMatrix1, cameraMatrix2;	//内部パラメータ行列
	Mat distCoeffs1, distCoeffs2;		//レンズ歪み行列 
	Mat R, T, E, F, R1, R2, P1, P2, Q;	//2眼レンズの間の関係を保持する行列群

	//stereo
	Mat disp;
	Mat	disp8;
	Rect roi1, roi2;
	Mat xyz;
	int blockSize = 0, numberOfDisparities = 0,MinDisparity=0;
	Ptr<StereoBM> bm = StereoBM::create();

	//ブロックマッチングのパラメータを決定する
	//numberOfDisparities = numberOfCheckerPatterns > 0 ? numberOfDisparities : sceneImg1.cols / 8;
	numberOfDisparities = 16*5; //16の倍数でないといけない、こいつが大きくなると画面が狭まる 左右のカメラの見える範囲が原因？
	blockSize =5;//大きいと滑らかになるけど正確ではない
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
	
	//世界座標を決める
	for (int i = 0; i < numberOfCheckerPatterns; i++) {
		for (int j = 0; j < patternSize.area(); j++) {
			worldPoints[i].push_back(Point3f(static_cast<float>(j%patternSize.width * MARKER_SIZE),
				static_cast<float>(j / patternSize.width * MARKER_SIZE),
				0.0));
		}
	}

	//2眼分のチェッカーパターン画像を読み込む
	for (int i = 0; i < numberOfCheckerPatterns; i++) {
		stringstream stream1, stream2;
		stream1 <<"../checkBoard/"<< i + 1 <<"L"<< ".jpg";
		stream2 <<"../checkBoard/"<< i + 1 <<"R"<< ".jpg";
		string fileName1 = stream1.str();
		string fileName2 = stream2.str();
		checkerImg1.push_back(imread(fileName1, 0));
		checkerImg2.push_back(imread(fileName2, 0));
	}

	//画像サイズを得る
	imageSize = Size(checkerImg1[0].cols, checkerImg1[0].rows);

	//チェックパターンの交点座標を求め、imagePointsに格納する
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

	//2眼ステレオカメラを同時にキャリブレーション
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

	//視差画像の計算
	bm->compute(sceneImg1, sceneImg2, disp);

	//視差の最大値と最小値を出す
	double minVal; double maxVal;
	minMaxLoc(disp, &minVal, &maxVal);
	printf("Min disp: %f Max value: %f \n", minVal, maxVal);
	
	//視差画像を距離に変換
	reprojectImageTo3D(disp, xyz, Q, true);

	//視差画像を変換して表示
	//①numberOfDisparities*16.と同じ結果になった
	disp.convertTo(disp8, CV_8U, 255 /(maxVal-minVal));
	//②教科書通り
	//disp.convertTo(disp8, CV_8U, 255 / (numberOfDisparities*16.));
	imshow("Disparity Map", disp8);
	waitKey(0);
	imwrite("disparity.bmp", disp8);
	
	return 0;
}