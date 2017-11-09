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


//Mat�C���X�^���X
Mat img_src;
Mat img_hsv;
Mat img_masked;
//�m�C�Y�����p�̍\���v�f
Mat element4 = (Mat_<uchar>(3, 3) << 0, 1, 0, 1, 1, 1, 0, 1, 0);
Mat element8 = (Mat_<uchar>(3, 3) << 1, 1, 1, 1, 1, 1, 1, 1, 1);
//�E�B���h�E�̖��O
const string win_src = "src";
const string win_hsv = "hsv";
const string win_masked = "masked";
const string win_track = "ajust_hsv";

//hsv�̒l
int min_h = 0, min_s = 0, min_v = 0;
int max_h = 255, max_s = 255, max_v = 255;
Scalar hsv_min;
Scalar hsv_max;

//�g���b�N�o�[�ɕύX������ꍇ�ɍs������
void onTrackbar(int a, void*) {
	hsv_min = Scalar(min_h, min_s, min_v);
	hsv_max = Scalar(max_h, max_s, max_v);
}

//�g���b�N�o�[�ɂ��Ă̊֐�
void  make_track() {
	namedWindow(win_track, CV_WINDOW_AUTOSIZE);
	createTrackbar("MIN_H", win_track, &min_h, 180, onTrackbar);
	createTrackbar("MIN_S", win_track, &min_s, 255, onTrackbar);
	createTrackbar("MIN_V", win_track, &min_v, 255, onTrackbar);
	createTrackbar("MAX_H", win_track, &max_h, 180, onTrackbar);
	createTrackbar("MAX_S", win_track, &max_s, 255, onTrackbar);
	createTrackbar("MAX_V", win_track, &max_v, 255, onTrackbar);
}

//hsv�̒l�̕ۑ�
void save_hsv_values() {
	fstream fs;
	fs.open("hsv.txt",ios::out);
	string hsv_values = to_string(min_h) + "," + to_string(min_s) + "," + to_string(min_v)
		+ "," + to_string(max_h) + "," + to_string(max_s) + "," + to_string(max_v);
	fs << hsv_values;
	fs.close();

}

//�E�B���h�E�̐���
void make_windows() {
	namedWindow(win_src, CV_WINDOW_AUTOSIZE);
	namedWindow(win_hsv, CV_WINDOW_AUTOSIZE);
	namedWindow(win_masked, CV_WINDOW_AUTOSIZE);
	make_track();
}

//�摜�̕\��
void imshows() {
	imshow(win_src, img_src);
	imshow(win_hsv, img_hsv);
	imshow(win_masked, img_masked);
}

//�����̓��e
void process_images() {
	//hsv��Ԃ�
	cvtColor(img_src, img_hsv, CV_RGB2HSV);
	//���o����F�̎w��
	inRange(img_hsv, hsv_min, hsv_max, img_masked);
	//�m�C�Y�����i����Ȃ������Aelement4�̂ق����ǂ����ʂ��o��j
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
	//�r�f�I0�ǂ݂Ƃ�
	VideoCapture capture(1);

	//�G���[�̏���
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
