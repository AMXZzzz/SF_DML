#pragma once
#include <windows.h>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"


struct Algorithm {

	VOID PidControl(float*, float, float, float);
	VOID FOVControl(float*, const float&,const int&, const int&, const int&);
	VOID MaxMovePixel(float* ,const int&);

private:
	float deviation = 0;		// ƫ����
	float last_deviation = 0;		// ��һ�ε�ƫ����
	float target_amount = 0;	// Ŀ�������㶨Ϊ0
	float P = 0;
	float I = 0;
	float D = 0;
	float pi = 3.14159265358979323846;
}; 



class Kalman {
public:
	Kalman() {
		const int stateNum = 4;                                      //״ֵ̬4��1����(x,y,��x,��y)
		const int measureNum = 2;                                    //����ֵ2��1����(x,y)	
		KF = cv::KalmanFilter(stateNum, measureNum, 0);

		KF.transitionMatrix = (cv::Mat_<float>(4, 4) << 1, 0, 1, 0,
			0, 1, 0, 1,
			0, 0, 1, 0,
			0, 0, 0, 1);  //ת�ƾ���A

		setIdentity(KF.measurementMatrix);												   //��������H
		setIdentity(KF.processNoiseCov, cv::Scalar::all(1e-2));                            //ϵͳ�����������Q
		setIdentity(KF.measurementNoiseCov, cv::Scalar::all(1e-1));                        //���������������R
		setIdentity(KF.errorCovPost, cv::Scalar::all(1));                                  //����������Э�������P

		measurement = cv::Mat::zeros(measureNum, 1, CV_32F);
	}

	VOID Predict(float& x, float& y) {
		float temp_x = x;
		float temp_y = y;

		// Ԥ��
		cv::Mat prediction = KF.predict();

		// ��������ֵ
		x = prediction.at<float>(0);
		y = prediction.at<float>(1);

		// ���²���ֵ
		measurement.at<float>(0) = temp_x;
		measurement.at<float>(1) = temp_y;

		//4.����״̬��
		KF.correct(measurement);
	};

	~Kalman() {};

private:
	cv::KalmanFilter KF;
	cv::Mat measurement;
};
