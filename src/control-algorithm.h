#pragma once
#include <windows.h>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"


struct Algorithm {

	VOID PidControl(float*, float, float, float);
	VOID FOVControl(float*, const float&,const int&, const int&, const int&);
	VOID MaxMovePixel(float* ,const int&);

private:
	float deviation = 0;		// 偏差量
	float last_deviation = 0;		// 上一次的偏差量
	float target_amount = 0;	// 目标量，恒定为0
	float P = 0;
	float I = 0;
	float D = 0;
	float pi = 3.14159265358979323846;
}; 



class Kalman {
public:
	Kalman() {
		const int stateNum = 4;                                      //状态值4×1向量(x,y,△x,△y)
		const int measureNum = 2;                                    //测量值2×1向量(x,y)	
		KF = cv::KalmanFilter(stateNum, measureNum, 0);

		KF.transitionMatrix = (cv::Mat_<float>(4, 4) << 1, 0, 1, 0,
			0, 1, 0, 1,
			0, 0, 1, 0,
			0, 0, 0, 1);  //转移矩阵A

		setIdentity(KF.measurementMatrix);												   //测量矩阵H
		setIdentity(KF.processNoiseCov, cv::Scalar::all(1e-2));                            //系统噪声方差矩阵Q
		setIdentity(KF.measurementNoiseCov, cv::Scalar::all(1e-1));                        //测量噪声方差矩阵R
		setIdentity(KF.errorCovPost, cv::Scalar::all(1));                                  //后验错误估计协方差矩阵P

		measurement = cv::Mat::zeros(measureNum, 1, CV_32F);
	}

	VOID Predict(float& x, float& y) {
		float temp_x = x;
		float temp_y = y;

		// 预测
		cv::Mat prediction = KF.predict();

		// 更新修正值
		x = prediction.at<float>(0);
		y = prediction.at<float>(1);

		// 更新测量值
		measurement.at<float>(0) = temp_x;
		measurement.at<float>(1) = temp_y;

		//4.更新状态器
		KF.correct(measurement);
	};

	~Kalman() {};

private:
	cv::KalmanFilter KF;
	cv::Mat measurement;
};
