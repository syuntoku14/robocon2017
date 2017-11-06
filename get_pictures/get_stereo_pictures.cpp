/**
* @brief Get two images from number0 & 1 cameras
*/

#include<iostream>
#include<opencv2\opencv.hpp>
#include<string>
using namespace std;
using namespace cv;

VideoCapture camera_L(1);
VideoCapture camera_R(2);
Mat picture_L, picture_R;
int count_get = 0;

void insertAndShowImg() {
	camera_L >> picture_L;
	camera_R >> picture_R;

	imshow("camera_L", picture_L);
	imshow("camera_R", picture_R);
}

void get_depth_picture() {
	string depth_L_filename = "../depth_picture/imL.jpg";
	string depth_R_filename = "../depth_picture/imR.jpg";
	imwrite(depth_L_filename, picture_L);
	imwrite(depth_R_filename, picture_R);
	cout << "got depth picture!" << endl;
	cout << "G key --> get pictures\nQ key --> quit\nD key-->get depth picture" << endl;
}

void get_picture() {
	count_get++;

	string checkBoad_L_filename="../checkBoad/"+to_string(count_get)+"L.jpg";
	string checkBoad_R_filename = "../checkBoad/" + to_string(count_get) + "R.jpg";\
	imwrite(checkBoad_L_filename,picture_L);
	imwrite(checkBoad_R_filename, picture_R);
	cout << "got successfully!" << endl;
	cout << "G key --> get pictures\nQ key --> quit\nD key-->get depth picture" << endl;
}

void get_picture_G_key() {
	if (waitKey(3) == 'g') {
		get_picture();
			waitKey(500); //�A���ŃL���v�`���[����̂�h�~
	}
}

void get_depth_picture_D_key() {
	if (waitKey(3) == 'd') {
		get_depth_picture();
		waitKey(500); //�A���ŃL���v�`���[����̂�h�~
	}
}

int main() {
	if (!(camera_L.isOpened() && camera_R.isOpened())) {
		cout << "capture failed!" << endl;
		return -1;
	}

	cout << "G key --> get pictures\nQ key --> quit\nD key-->get depth picture" << endl;
	while (1) {
		insertAndShowImg();
		get_picture_G_key();
		get_depth_picture_D_key();
		if (waitKey(1) == 'q') {
			break;
		}
	}

	return 0;
}