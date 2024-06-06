#include "framework.h"
#include "sf-trt.h"
#include "SFDXGI.h"


VOID YoloV5Per(cv::Mat& img,float* host_input_prt) {
	int img_rows = img.rows;
	int img_clos = img.cols;
	for (size_t h = 0; h < img_rows; ++h) {
		const cv::Vec3b* p1 = img.ptr<cv::Vec3b>(h);
		for (size_t w = 0; w < img_clos; ++w) {
			for (size_t i = 0; i < img.channels(); ++i) {
				host_input_prt[i * img_rows * img_rows + h * img_clos + w] = (p1[w][static_cast<size_t>(3) - 1 - i]) * (1 / 255.f);
			}
		}
	}
}

VOID YoloXPer(cv::Mat& img, float* host_input_prt) {
	for (int c = 0; c < 3; ++c) {
		for (int h = 0; h < img.rows; ++h) {
			cv::Vec3b* p1 = img.ptr<cv::Vec3b>(h);
			for (int w = 0; w < img.cols; ++w) {
				host_input_prt[c * img.cols * img.rows + h * img.cols + w] = (p1[w][c]);
			}
		}
	}
}

VOID YoloV8Per(cv::Mat& img, float* host_input_prt) {
	int img_rows = img.rows;
	int img_clos = img.cols;
	for (size_t h = 0; h < img_rows; ++h) {
		const cv::Vec3b* p1 = img.ptr<cv::Vec3b>(h);
		for (size_t w = 0; w < img_clos; ++w) {
			for (size_t i = 0; i < img.channels(); ++i) {
				host_input_prt[i * img_rows * img_rows + h * img_clos + w] = (p1[w][static_cast<size_t>(3) - 1 - i]) * (1 / 255.f);
			}
		}
	}
}

VOID YoloV5Post(float* output, int class_num, int size, int Anchorbox_num) {	// 输出数组，类别，输入大小，先验框

	Process* process = &Process::Get();
	double conf = SF::Value::Get().confidence;
	float iou = SF::Value::Get().iou;

	process->boxes.clear();
	process->confidences.clear();
	process->classes.clear();
	process->indices.clear();

	//√   
	for (int i = 0; i < Anchorbox_num; ++i) {
		int index = i * (5 + class_num);
		float confidence = output[(class_num + 5) * i + 4];
		if (confidence <= conf) continue;

		cv::Mat scores(1, class_num, CV_32FC1, &output[(class_num + 5) * i + 5]);
		cv::Point class_id;
		double max_class_score;
		minMaxLoc(scores, 0, &max_class_score, 0, &class_id);

		cv::Rect temp;
		temp.x = (int)output[index + 0];
		temp.y = (int)output[index + 1];
		temp.width = (int)output[index + 2];
		temp.height = (int)output[index + 3];

		process->boxes.push_back(temp);
		process->confidences.push_back(confidence);
		process->classes.push_back(class_id.x);
	}

	cv::dnn::NMSBoxes(process->boxes, process->confidences, conf, iou, process->indices);
}

static VOID generate_grids_and_stride(Process* process, int size) {

	for (auto stride : process->strides) {
		int num_grid_w = size / stride;
		int num_grid_h = size / stride;
		for (int g1 = 0; g1 < num_grid_w; g1++) {
			for (int g0 = 0; g0 < num_grid_h; g0++) {
				Process::GridAndStride gs{};
				gs.grid0 = g0;
				gs.grid1 = g1;
				gs.stride = stride;
				process->grid_strides.push_back(gs);
			}
		}
	}
}

VOID YoloXPost(float* output, int class_num,int size,int Anchorbox_num) {

	Process* process = &Process::Get();
	float conf = SF::Value::Get().confidence;
	float iou = SF::Value::Get().iou;

	process->boxes.clear();
	process->confidences.clear();
	process->classes.clear();
	process->indices.clear();
	process->grid_strides.clear();

	generate_grids_and_stride(process, size);

	for (int anchor_idx = 0; anchor_idx < process->grid_strides.size(); anchor_idx++) {
		const int grid0 = process->grid_strides[anchor_idx].grid0;
		const int grid1 = process->grid_strides[anchor_idx].grid1;
		const int stride = process->grid_strides[anchor_idx].stride;

		const int basic_pos = anchor_idx * (class_num + 5);

		float x_center = (output[basic_pos + 0] + grid0) * stride;
		float y_center = (output[basic_pos + 1] + grid1) * stride;
		float w = exp(output[basic_pos + 2]) * stride;
		float h = exp(output[basic_pos + 3]) * stride;
		float x0 = x_center - w * 0.5f;
		float y0 = y_center - h * 0.5f;
		float box_objectness = output[basic_pos + 4];

		for (int class_idx = 0; class_idx < class_num; ++class_idx) {
			float box_cls_score = output[basic_pos + 5 + class_idx];
			float box_prob = box_objectness * box_cls_score;

			if (box_prob < conf)
				continue;

			cv::Rect rect;
			rect.x = x_center;
			rect.y = y_center;
			rect.width = w;
			rect.height = h;

			process->classes.push_back(class_idx);
			process->confidences.push_back((float)box_prob);
			process->boxes.push_back(rect);
		}
	}
	cv::dnn::NMSBoxes(process->boxes, process->confidences, conf, iou, process->indices);
}

VOID YoloV8Post(float* output, int class_num, int size, int Anchorbox_num) {

	Process* process = &Process::Get();
	float conf = SF::Value::Get().confidence;
	float iou = SF::Value::Get().iou;

	process->boxes.clear();
	process->confidences.clear();
	process->classes.clear();
	process->indices.clear();

	cv::Mat outputs = cv::Mat((4 + class_num), Anchorbox_num, CV_32F, output);
	outputs = outputs.t();

	for (int i = 0; i < Anchorbox_num; i++) {
		auto rowPtr = outputs.row(i).ptr<float>();
		auto bboxesPtr = rowPtr;
		auto scoresPtr = rowPtr + 4;
		auto maxSPtr = std::max_element(scoresPtr, scoresPtr + class_num);
		float score = *maxSPtr;
		if (score > conf) {

			cv::Rect_<float> bbox;
			bbox.x = *bboxesPtr++;
			bbox.y = *bboxesPtr++;
			bbox.width = *bboxesPtr++;
			bbox.height = *bboxesPtr;

			process->boxes.push_back(bbox);
			process->confidences.push_back(score);
			process->classes.push_back((maxSPtr - scoresPtr));
		}
	}
	
	cv::dnn::NMSBoxes(process->boxes, process->confidences, conf, iou, process->indices);
}



