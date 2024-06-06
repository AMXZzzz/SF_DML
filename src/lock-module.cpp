#include <iostream>
#include "lock-module.h"
#include <stdio.h>
#include "sf-trt.h"
#include "move-way.h"
#include "framework.h"
#include "SFDXGI.h"
#include "control-algorithm.h"
#include "msdk.h"

typedef std::chrono::system_clock::time_point ITIME_POINT;

BOOL LockMode::CreateLockEvent() {
	char new_name[MAX_PATH]{};
	new_name[MAX_PATH-1] = 0;
	_snprintf_s(new_name, 64, "%s%lu", "SF_TRT", (long)GetCurrentProcessId());
	lock_event = CreateEventA(NULL, FALSE, FALSE, new_name);
	if (lock_event == NULL) {
		MessageBoxA(NULL, "创建事件失败,错误码查看日志", MESSAGEBOX_TITLE, MB_OK);
		g_logger->warn("创建事件失败，错误码{}", std::to_string(GetLastError()).c_str());
		return FALSE;
	}
	return TRUE;
}

static VOID PushIndices(std::vector<float>& EuclideanDistance, 
	std::vector<int>& indices, 
	Process* process,
	const int& idx ,
	const bool& classes){
	SFPoint* point = &SF_DXGI::Get().point;
	if (classes) {
		indices.push_back(process->indices[idx]);
		EuclideanDistance.push_back(
			pow((point->origin_x + process->boxes[process->indices[idx]].x) - point->center_x, 2) +
			pow((point->origin_y + process->boxes[process->indices[idx]].y) - point->center_y, 2)
		);
	}
}

static inline BOOL IGetAsyncKeyState(SF::Value* value) {

	switch (value->monitor_module) {
	case 0:
		if (value->lock_key2 == 0) {
			return GetAsyncKeyState(value->lock_key) && value->lock_key_switch;
		}
		else {
			return (GetAsyncKeyState(value->lock_key) && value->lock_key_switch) || GetAsyncKeyState(value->lock_key2);
		}
	case 1:
		if (value->lock_key2 == 0) {
			return value->conitor(value->lock_key) && value->lock_key_switch;
		} else {

			return (value->conitor(value->lock_key) && value->lock_key_switch) || value->conitor(value->lock_key2);
		}
	}
	return false;
}

static ITIME_POINT auto_start = std::chrono::system_clock::now();
static ITIME_POINT sleep_start = std::chrono::system_clock::now();
inline bool AutoComplete(SF::Value* value, ITIME_POINT start) {
	int random_time = 0;
	if (value->auto_random != 0) {
		random_time = (int)value->auto_random * rand() / (RAND_MAX + 1);
	}
	ITIME_POINT auto_end = std::chrono::system_clock::now();
	return std::chrono::duration_cast <std::chrono::milliseconds> (auto_end - start).count() >= (value->auto_interval + random_time) * 1.0;
}

static inline VOID LeftButtonClick(SF::Value* value) {

	if (value->auto_click == 1) {
		M_LeftClick(MoveWay::Get().yjs, 1);
	}else{
		INPUT input{};
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;   //MOUSEEVENTF_LEFTDOWN 左键按下
		input.mi.time = 0;
		input.mi.dwExtraInfo = 0;
		SendInput(1, &input, sizeof(INPUT));

		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_LEFTUP;   // MOUSEEVENTF_LEFTUP  左键松开
		input.mi.time = 0;
		input.mi.dwExtraInfo = 0;
		SendInput(1, &input, sizeof(INPUT));
	}
}


DWORD WINAPI LockThread() {
	SF::Signl* signl = &SF::Signl::Get();
	SF::Value* value = &SF::Value::Get();
	LockMode* lock = &LockMode::Get();
	MoveWay* moveway = &MoveWay::Get();
	Process* process = &Process::Get();
	SFPoint* point = &SF_DXGI::Get().point;

	Algorithm algorithm_x ;
	Algorithm algorithm_y ;
	Kalman kl;

	std::vector<float> EuclideanDistance;
	std::vector<int> indices;			// 类别选择

	int fame_temp = 0;
	while (signl->ThreadStopSignl && signl->ImGuiWinStop) {

		WaitForSingleObject(lock->lock_event, INFINITE);
		if (!process->indices.size()) continue;

		cv::Rect target_xywh;
		bool category = signl->ModelClasses;
		EuclideanDistance.clear();
		indices.clear();

		for (int i = 0; i < process->indices.size(); i++) {
			if (category) {
				switch (process->classes[process->indices[i]]) {
				case 0:
					PushIndices(EuclideanDistance, indices, process, i, value->class0);
					break;
				case 1:
					PushIndices(EuclideanDistance, indices, process, i, value->class1);
					break;
				case 2:
					PushIndices(EuclideanDistance, indices, process, i, value->class2);
					break;
				case 3:
					PushIndices(EuclideanDistance, indices, process, i, value->class3);
					break;
				}
			}else {
				EuclideanDistance.push_back(
					pow((point->origin_x + process->boxes[process->indices[i]].x) - point->center_x, 2) +
					pow((point->origin_y + process->boxes[process->indices[i]].y) - point->center_y, 2)
				);
			}
		}

		// 最近目标索引
		int idx = 0;
		if (category) {
			if (!indices.size())
				continue;
			idx = indices[std::distance(std::begin(EuclideanDistance), std::min_element(std::begin(EuclideanDistance), std::end(EuclideanDistance)))];
		}
		else {
			idx = process->indices[std::distance(std::begin(EuclideanDistance), std::min_element(std::begin(EuclideanDistance), std::end(EuclideanDistance)))];
		}

		target_xywh.x = process->boxes[idx].x;
		target_xywh.y = process->boxes[idx].y;
		target_xywh.width = process->boxes[idx].width;
		target_xywh.height = process->boxes[idx].height;

		// 偏移位置
		if (value->location)
			target_xywh.y = (target_xywh.y + (target_xywh.height * 0.5)) - (target_xywh.height * value->location);

		// 相对距离
		float x = (point->origin_x + target_xywh.x) - point->center_x;
		float y = (point->origin_y + target_xywh.y) - point->center_y;

		// 超前融合
		if (value->advanced) {
			++fame_temp;
			if (fame_temp > value->advanced_size) {
				if (x > 0) {
					target_xywh.x = target_xywh.x + (target_xywh.width * value->advanced_multiple);
				}
				if (x < 0) {
					target_xywh.x = target_xywh.x - (target_xywh.width * value->advanced_multiple);
				}
				x = (point->origin_x + target_xywh.x) - point->center_x;
				fame_temp = 0;
			}
		}

		// 范围外跳过 
		if (abs(x) > value->effectiverange * 0.5 || abs(y) > value->effectiverange * 0.5) continue;

		// FOV控制计算 
		if (value->fov_algorithm) {
			algorithm_x.FOVControl(&x, value->hfov, value->game_x_pixel, point->CapWidth, 360);	// point->CapWidth
			algorithm_x.FOVControl(&y, value->vfov, value->game_y_pixel, point->CapHeight, 180);
		}

		// PID控制 减少内存复制
		if (value->pid_algorithm) {
			algorithm_x.PidControl(&x, value->P_x, value->I_x, value->D_x);
			algorithm_y.PidControl(&y, value->P_y, value->I_y, value->D_y);
		}

		// 最大移动
		if (value->max_pixels) {
			algorithm_y.MaxMovePixel(&x, value->max_pixels);
			algorithm_y.MaxMovePixel(&y, value->max_pixels);
		}
		
		//! 削抖
		if (value->debounce) {
			if (x < value->debounce_size && x > -(value->debounce_size)) {
				continue;
			}
		}

		// 扳机	
		if (value->auto_fire) {
			if (!IGetAsyncKeyState(value)) {
				sleep_start = std::chrono::system_clock::now();
			}

			if (IGetAsyncKeyState(value)) {

				switch (value->auto_model) {
				case 0:		// 先移动后开枪
					moveway->MoveR(int(x), int(y));
					if (((target_xywh.width * 0.5f) * value->trigger_w)  > abs(x) && ((target_xywh.height * 0.5f) * value->trigger_h) > abs(y) && AutoComplete(value, auto_start)) {
						if (GetAsyncKeyState(value->auto_key)) {
							LeftButtonClick(value);
						}
						auto_start = std::chrono::system_clock::now();
					}
					break;
				case 1:		// 范围内开枪
					if (((target_xywh.width * 0.5f) * value->trigger_w) > abs(x) && ((target_xywh.height * 0.5f) * value->trigger_h) > abs(y) && AutoComplete(value, auto_start)) {
						if (GetAsyncKeyState(value->auto_key)) {
							LeftButtonClick(value);
						}
						auto_start = std::chrono::system_clock::now();
					}
					break;
				case 2:		// 移动后延迟开枪
					moveway->MoveR(int(x), int(y));
					if (AutoComplete(value, sleep_start)) {
						LeftButtonClick(value);
						sleep_start = std::chrono::system_clock::now();
					}
					break;
				case 3:

					break;
				}
			}
			continue;
		}

		// 单移动
		if (value->lock_model) {
			moveway->MoveR(int(x), int(y));
		}
		else {
			if (IGetAsyncKeyState(value)) {
				moveway->MoveR(int(x), int(y));
			}
		}
	}

	// 释放
	moveway->MoveClose();
	return 0;
}

VOID SetLockEvent() {
	SetEvent(LockMode::Get().lock_event);
}

BOOL LockMode::InitLock(int way,VOID (**f)()) {	// 二级指针修改，不能直接修改
	// 初始化事件
	if (!CreateLockEvent()) {
		return FALSE;
	}
	// 初始化移动
	if (!MoveWay::Get().InitMove(way)) return FALSE;

	// 创建线程
	lock_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LockThread, 0, 0, 0);
	if (lock_thread == NULL) {
		MessageBoxA(NULL, "创建Lock线程失败，错误码查看日志", MESSAGEBOX_TITLE, MB_OK);
		g_logger->warn("创建Lock线程失败，错误码{}", std::to_string(GetLastError()).c_str());
		return FALSE;
	}
	*f = SetLockEvent;

	return TRUE;
}