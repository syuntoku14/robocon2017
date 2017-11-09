/**

* @for adjusting mask parameters
*/

#include <iostream>
#include<opencv2\opencv.hpp>
#include<cmath>
#include<string>
#include<opencv2\highgui.hpp>
#include<fstream>

using namespace std;
using namespace cv;


//Matインスタンス
Mat img_src;
Mat img_hsv;
Mat img_masked;
//ノイズ除去用の構造要素
Mat element4 = (Mat_<uchar>(3, 3) << 0, 1, 0, 1, 1, 1, 0, 1, 0);
Mat element8 = (Mat_<uchar>(3, 3) << 1, 1, 1, 1, 1, 1, 1, 1, 1);
//ウィンドウの名前
const string win_src = "src";
const string win_hsv = "hsv";
const string win_masked = "masked";
const string win_track = "ajust_hsv";

//hsvの値
int min_h = 0, min_s = 0, min_v = 0;
int max_h = 255, max_s = 255, max_v = 255;
Scalar hsv_min;
Scalar hsv_max;

//トラックバーに変更がある場合に行う処理
void onTrackbar(int a, void*) {
	hsv_min = Scalar(min_h, min_s, min_v);
	hsv_max = Scalar(max_h, max_s, max_v);
}

//トラックバーについての関数
void  make_track() {
	namedWindow(win_track, CV_WINDOW_AUTOSIZE);
	createTrackbar("MIN_H", win_track, &min_h, 180, onTrackbar);
	createTrackbar("MIN_S", win_track, &min_s, 255, onTrackbar);
	createTrackbar("MIN_V", win_track, &min_v, 255, onTrackbar);
	createTrackbar("MAX_H", win_track, &max_h, 180, onTrackbar);
	createTrackbar("MAX_S", win_track, &max_s, 255, onTrackbar);
	createTrackbar("MAX_V", win_track, &max_v, 255, onTrackbar);
}

//hsvの値の保存
void save_hsv_values() {
	fstream fs;
	fs.open("hsv.txt",ios::out);
	string hsv_values = to_string(min_h) + "," + to_string(min_s) + "," + to_string(min_v)
		+ "," + to_string(max_h) + "," + to_string(max_s) + "," + to_string(max_v);
	fs << hsv_values;
	fs.close();

}

//ウィンドウの生成
void make_windows() {
	namedWindow(win_src, CV_WINDOW_AUTOSIZE);
	namedWindow(win_hsv, CV_WINDOW_AUTOSIZE);
	namedWindow(win_masked, CV_WINDOW_AUTOSIZE);
	make_track();
}

//画像の表示
void imshows() {
	imshow(win_src, img_src);
	imshow(win_hsv, img_hsv);
	imshow(win_masked, img_masked);
}

//処理の内容
void process_images() {
	//hsv空間に
	cvtColor(img_src, img_hsv, CV_RGB2HSV);
	//抽出する色の指定
	inRange(img_hsv, hsv_min, hsv_max, img_masked);
	//ノイズ除去（いらないかも、element4のほうが良い結果が出る）
	morphologyEx(img_masked, img_masked, MORPH_OPEN, element4, Point(-1, -1), 1);
}

void save_hsv_value() {
	FileStorage fs("hsv_value.xml", FileStorage::WRITE);
	fs << "min_h" << min_h;
	fs << "min_s" << min_s;
	fs << "min_v" << min_v;
	fs << "max_h" << max_h;
	fs << "max_s" << max_s;
	fs << "max_v" << max_v;
	fs.release();
}

int main(int argc, char **args) {
	//ビデオ0読みとり
	VideoCapture capture(1);

	//エラーの処理
	if (!capture.isOpened()) {
		cout << "error" << endl;
		return -1;
	}

	make_windows();

	while (1) {
		capture >> img_src;

		process_images();

		imshows();

		if (waitKey(1) == 'q') {
			save_hsv_values();
			break;
		}
	}

	save_hsv_value();

	return 0;
}
