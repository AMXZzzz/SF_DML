#pragma once
#include <windows.h>
#include "opencv2/opencv.hpp"

struct Process {
	static Process& Get() {
		static Process m_pInstance;
		return m_pInstance;
	}

	std::vector<int> indices;			// 所有目标的容器索引
	std::vector<cv::Rect> boxes;		// 所有目标的坐标
	std::vector<int> classes;			// 所有目标的类别
	std::vector<float> confidences;		// 所有目标的置信度

	//yolox
	struct GridAndStride {				// 网格维度
		int grid0;
		int grid1;
		int stride;
	};
	std::vector<int> strides = { 8, 16, 32 };	// 步长，即640/8，640/16，640/32
	std::vector<GridAndStride> grid_strides;	// 网格
};



VOID YoloV5Per(cv::Mat& img, float* host_input_prt);
VOID YoloXPer(cv::Mat& img, float* host_input_prt);
VOID YoloV8Per(cv::Mat& img, float* host_input_prt);

VOID YoloV5Post(float* , int ,int,int);
VOID YoloXPost(float*, int, int, int);
VOID YoloV8Post(float*, int, int, int);