/**
* @brief Get two images from number0 & 1 cameras
*/

#include <iostream>
#include<opencv2\opencv.hpp>
#include<string>
using namespace std;
using namespace cv;

VideoCapture camera_L(0);
VideoCapture camera_R(1);
Mat picture_L, picture_R;

void get_picture() {
	camera_L >> picture_L;
	camera_R >> picture_R;

	cout << "got successfully!" << endl;
}

void get_picture_G_key() {
	if (waitKey(1) == 'g') {
		get_picture();
	}
	waitKey(500); //連続でキャプチャーするのを防止
}


int main() {
	if (!(camera_L.isOpened() && camera_R.isOpened())) {
		cout << "capture failed!" << endl;
		return -1;
	}

	while (1) {
		get_picture_G_key();
		if (waitKey(1) == 'q') {
			break;
		}
	}

	return 0;
}