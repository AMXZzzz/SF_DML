#include "dml-module.h"
#include "sf-trt.h"
#include "SFDXGI.h"
#include "framework.h"
#include "lock-module.h"
#pragma comment(lib, "winmm.lib")
#define CHECKORT(x,y) if (!CheckStatus(x,y)) return FALSE;

static HANDLE dml_thread = NULL;		// �߳̾��
inline void Delay(int fps) {
	int time = 1000.f / fps;
	timeBeginPeriod(1);
	Sleep(time);
	timeEndPeriod(1);
}
BOOL DMLModel::CheckStatus(OrtStatus * status, int line) {
	if (status != NULL) {
		std::cout << line << std::endl;
		const char* msg = g_ort->GetErrorMessage(status);
		std::cout << msg << std::endl;
		g_ort->ReleaseStatus(status);
		return FALSE;
	}
	return TRUE;
}

VOID DMLModel::LoadDMLResource() {
	switch (SF::Variables::Get().process_frame) {
	case 0:		// yolov5/7
		Normalized = 1 / 255.0f;
		Postprocess = YoloV5Post;
		break;
	case 1:		// yolox
		Normalized = 1.0;
		Postprocess = YoloXPost;
		break;
	case 2:		//yolov8
		Normalized = 1 / 255.0f;
		Postprocess = YoloV8Post;
		break;
	}
}

BOOL DMLModel::InitDML(const char* path) {

	// ����ģ��
	LoadDMLResource();
	CHECKORT(g_ort->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "SuperResolution", &env) , __LINE__);	// �������л���
	CHECKORT(g_ort->CreateSessionOptions(&session_options), __LINE__);							// �����Ự�����ļ�
	CHECKORT(g_ort->SetSessionGraphOptimizationLevel(session_options, ORT_ENABLE_BASIC), __LINE__);	// �Ż��ȼ�
	CHECKORT(g_ort->DisableMemPattern(session_options), __LINE__);			// �ر��ڴ�
	OrtStatus* status = OrtSessionOptionsAppendExecutionProvider_DML(session_options, SF::Variables::Get().dx_gpu_idx);	// �������, sb����	
	CHECKORT(g_ort->CreateSession(env, String2WString(path).c_str(), session_options, &session), __LINE__);	// �����Ự
	// ��ȡģ����Ϣ
	asserthr(GetModelInfo());
	// ����host�ڴ�
	CHECKORT(g_ort->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info), __LINE__);
	return TRUE;
}

BOOL DMLModel::GetLayerInfo(std::vector<int64_t>* dims,int idx, BOOL IsInput = TRUE) {
	size_t dims_size = 0;		// ά�ȴ�С
	OrtTypeInfo* typeinfo = nullptr;
	const OrtTensorTypeAndShapeInfo* info;

	if (IsInput) {
		CHECKORT(g_ort->SessionGetInputTypeInfo(session, idx, &typeinfo), __LINE__);
	}
	else {
		CHECKORT(g_ort->SessionGetOutputTypeInfo(session, idx, &typeinfo), __LINE__);
	}
	CHECKORT(g_ort->CastTypeInfoToTensorInfo(typeinfo, &info), __LINE__);
	CHECKORT(g_ort->GetDimensionsCount(info, &dims_size), __LINE__);

	(*dims).resize(dims_size);
	CHECKORT(g_ort->GetDimensions(info, (*dims).data(), dims_size), __LINE__);
	for (size_t i = 0; i < dims_size; i++)
		std::cout << "ά�� " << i << " = " << (*dims)[i] << std::endl;
	if (IsInput) {
		for (size_t i = 0; i < dims_size; i++)
			input_tensor_size *= input_dims[i];
	}
	return TRUE;
}

BOOL DMLModel::GetModelInfo() {
	BOOL hr_in = FALSE;
	BOOL hr_out = FALSE;
	int frame = SF::Variables::Get().process_frame;		// ��ֹ�䶯

	size_t nodes_num = 0;								// ����
	char* temp_name;
	CHECKORT(g_ort->GetAllocatorWithDefaultOptions(&allocator), __LINE__);	// ��ȡ��Ϣ

	// -------- ����ڵ���� --------
	CHECKORT(g_ort->SessionGetInputCount(session, &nodes_num), __LINE__);	// ��ȡ��������
	for (size_t i = 0; i < nodes_num; i++) {
		CHECKORT(g_ort->SessionGetInputName(session, i, allocator, &temp_name), __LINE__);	// ��ȡ����

		asserthr(GetLayerInfo(&input_dims, i, TRUE));		// �Ȼ�ȡ�ڵ�ά�ȣ���ʹ��ά��

		if (strcmp("images", temp_name) == 0) {				// ������벻��images
			// ��Ҫ�ж��������
			if (input_dims[2] != input_dims[3]) {
				MessageBoxA(NULL, "��֧�����벻�Եȵ�ģ��", MESSAGEBOX_TITLE, MB_OK);
				return FALSE;
			}
			input_idx = i;
			SF::Variables::Get().max_range = static_cast<int>(input_dims[2]);
			Input_size = static_cast<int>(input_dims[2]);
			hr_in = TRUE;
			g_logger->info("����:{} {} {} {}", std::to_string(input_dims[0]).c_str(), std::to_string(input_dims[1]).c_str(),
				std::to_string(input_dims[2]).c_str(), std::to_string(input_dims[3]).c_str());
			break;			// �ҵ�����������������ʱ����
		}
	}

	if (!hr_in) {
		g_logger->info("�Ҳ�����Ϊimages������,���԰���ά�Ȳ���");
		for (size_t i = 0; i < nodes_num; i++) {
			CHECKORT(g_ort->SessionGetInputName(session, i, allocator, &temp_name), __LINE__);
			asserthr(GetLayerInfo(&input_dims, i, TRUE));	// �Ȼ�ȡά�ȣ���ʹ��ά��
			if (input_dims.size() == 4 && input_dims[1] < input_dims[2] && input_dims[2] == input_dims[3]) {
				input_idx = i;
				SF::Variables::Get().max_range = static_cast<int>(input_dims[2]);
				Input_size = static_cast<int>(input_dims[2]);
				g_logger->info("�ҵ���������������:{} {} {} {}", std::to_string(input_dims[0]).c_str(), std::to_string(input_dims[1]).c_str(),
					std::to_string(input_dims[2]).c_str(), std::to_string(input_dims[3]).c_str());
				hr_in = TRUE;
				break;			// �ҵ�������
			}
		}
	}

	// ��������tensor  fp16/fp32
	CHECKORT(g_ort->CreateTensorAsOrtValue(allocator, input_dims.data(), 4, ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16, &input_tensors), __LINE__);	// FP16
	//CHECKORT(g_ort->CreateTensorAsOrtValue(allocator, input_dims.data(), 4, ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, &input_tensors), __LINE__);		// FP32

	//  -------- ����ڵ���� --------
	CHECKORT(g_ort->SessionGetOutputCount(session, &nodes_num), __LINE__);	// ���������
	for (size_t i = 0; i < nodes_num; i++) {
		g_ort->SessionGetOutputName(session, i, allocator, &temp_name);				// ��ȡ�ڵ�����
		asserthr(GetLayerInfo(&output_dims, i, FALSE));								// ��ȡ�ڵ���Ϣ

		if (strcmp("output", temp_name) == 0 || strcmp("output0", temp_name) == 0) {				// �Ҳ�������ڵ�  yolov8��outputs0
			output_idx = i;
			if (frame == 2) {	// yolov8
				g_logger->info("YOLOv8���:{} {} {}", std::to_string(output_dims[0]).c_str(), std::to_string(output_dims[1]).c_str(),
					std::to_string(output_dims[2]).c_str());
				output_names_is_v8 = 1;
				Anchorbox_num = static_cast<int>(output_dims[2]);
				SF::Value::Get().class_number = num_classes = static_cast<int>(output_dims[1] - 4);
				g_logger->info("Anchorbox_num:{}, num_classes:{}", std::to_string(Anchorbox_num).c_str(), std::to_string(num_classes));
				hr_out = TRUE;
				break;			// �ҵ�������,�������������߻Ḳ��
			} else {
				g_logger->info("���ά��:{} {} {}", std::to_string(output_dims[0]).c_str(), std::to_string(output_dims[1]).c_str(),
					std::to_string(output_dims[2]).c_str());
				output_names_is_v8 = 0;
				Anchorbox_num = static_cast<int>(output_dims[1]);
				SF::Value::Get().class_number = num_classes = static_cast<int>(output_dims[2] - 5);				// �������
				g_logger->info("Anchorbox_num:{}, num_classes:{}", std::to_string(Anchorbox_num).c_str(), std::to_string(num_classes));
				hr_out = TRUE;
				break;			// �ҵ�������
			}
		} 
	}

	// �Ҳ���output��output0
	if (!hr_out) {
		g_logger->info("�Ҳ�����Ϊoutput��output0�����,���԰���ά�Ȳ���");

		for (size_t i = 0; i < nodes_num; i++) {
			g_ort->SessionGetOutputName(session, i, allocator, &temp_name);				// ��ȡ�ڵ�����
			asserthr(GetLayerInfo(&output_dims, i, FALSE));								// ��ȡ�ڵ���Ϣ
			if (frame == 2 ) {	 // yolov8
				if (output_dims[0] < output_dims[1] && output_dims[1]< output_dims[2]) {
					g_logger->info("�ҵ�����YOLOv8�����:{} {} {}", std::to_string(output_dims[0]).c_str(), std::to_string(output_dims[1]).c_str(),
						std::to_string(output_dims[2]).c_str());
					output_idx = i;
					Anchorbox_num = static_cast<int>(output_dims[2]);
					SF::Value::Get().class_number = num_classes = static_cast<int>(output_dims[1] - 4);
					hr_out = TRUE;
					break;			// �ҵ�������������
				}
			} else {
				if (output_dims[0] < output_dims[1] && output_dims[1] > output_dims[2]) {	// yolov5 or yolox
					g_logger->info("�ҵ����ϵ����:{} {} {}", std::to_string(output_dims[0]).c_str(), std::to_string(output_dims[1]).c_str(),
						std::to_string(output_dims[2]).c_str());
					output_idx = i;
					Anchorbox_num = static_cast<int>(output_dims[1]);
					SF::Value::Get().class_number = num_classes = static_cast<int>(output_dims[2] - 5);				// �������
					hr_out = TRUE;
					break;			// �ҵ�������
				}
			}
		}
	}

	// �ܽ�
	if (!hr_in || !hr_out) {
		if (!hr_in) {
			g_logger->info("ģ���������ʧ��,����ģ�͵�����ڵ�,��������:[1 3 416 416](��[1 3 640 640]),��֧�ֶ�̬batch size");
		}
		if (!hr_out) {
			g_logger->info("ģ���������ʧ��,����ѡ��Ŀ���Ƿ���ȷ,ע��yolov8��ά����:[1 84 25200]");
		}
		return FALSE;
	}

	// �������tensor  fp32
	CHECKORT(g_ort->CreateTensorAsOrtValue(allocator, output_dims.data(), 3, ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, &output_tensors), __LINE__);
	return TRUE;
}

VOID DMLModel::Preprocess(cv::Mat &img) {

	static cv::Mat blob;	// ���ڴ�
	blob = cv::dnn::blobFromImage(img, Normalized, cv::Size(input_dims[2], input_dims[3]), cv::Scalar(), true, false);
#if !SF_DEBUG
	CheckStatus(g_ort->CreateTensorWithDataAsOrtValue(memory_info, blob.ptr<float>(), input_tensor_size * sizeof(float), input_dims.data(), input_dims.size(), ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, &input_tensors)
	,__LINE__);
#else
	g_ort->CreateTensorWithDataAsOrtValue(memory_info, blob.ptr<float>(), input_tensor_size * sizeof(float), input_dims.data(), input_dims.size(), ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, &input_tensors);
#endif // SF_DEBUG
}

VOID DMLModel::Detect() {
#if !SF_DEBUG
	if (output_names_is_v8) {
		CheckStatus(g_ort->Run(session, NULL, input_names.data(), &input_tensors, 1, output_names_v8.data(), 1, &output_tensors), __LINE__);
	} else {
		CheckStatus(g_ort->Run(session, NULL, input_names.data(), &input_tensors, 1, output_names.data(), 1, &output_tensors), __LINE__);
	}
	
	CheckStatus(g_ort->GetTensorMutableData(output_tensors, (void**)&floatarr), __LINE__);
#else
	g_ort->Run(session, NULL, input_names.data(), &input_tensors, 1, output_names.data(), 1, &output_tensors);
	g_ort->GetTensorMutableData(output_tensors, (void**)&floatarr);
	
#endif // SF_DEBUG
}

VOID DMLModel::Release() {

	if (env) g_ort->ReleaseEnv(env);
	if (memory_info) g_ort->ReleaseMemoryInfo(memory_info);
	if (session) g_ort->ReleaseSession(session);
	if (session_options) g_ort->ReleaseSessionOptions(session_options);
	if (input_tensors) g_ort->ReleaseValue(input_tensors);
	if (output_tensors) g_ort->ReleaseValue(output_tensors);
	//if (allocator) g_ort->ReleaseAllocator(allocator);
}

static DWORD WINAPI DMLThread() {
	// ��ʼ��ָ��
	SF::Signl* signl = &SF::Signl::Get();
	SF::Variables* variables = &SF::Variables::Get();
	DMLModel* dml = new DMLModel;
	SF_DXGI* sf = &SF_DXGI::Get();

	LockMode* lock = &LockMode::Get();
	cv::Mat img;

	// ��ʼ��DML
	if (!dml->InitDML(SF::Value::Get().cpu_onnx)) {
		goto Free;
	}
	//��ʼ��DXGI
	SF_DXGI_ERROR dxerr = sf->CaptureResource(dml->Input_size , dml->Input_size);
	if (dxerr != DXGI_SUCCECC) {
		g_logger->warn("DXGI��ͼ��ʼ��ʧ�ܣ�ʧ���룺{}",std::to_string(dxerr).c_str());
		MessageBoxA(NULL, "DXGI��ʼ��ʧ��,������鿴��־", MESSAGEBOX_TITLE, MB_OK);
		goto Free;
	}
	// ��ʼ���ƶ�
	if (!lock->InitLock(SF::Variables::Get().move_way, &dml->LockStart)) {
		g_logger->warn("����ģ���ʼ��ʧ��");
		goto Free;
	}

	// run
	g_logger->info("DMLThread: while��ʼ����...");
	while (signl->ThreadStopSignl && signl->ImGuiWinStop) {
		SF_TIME_POINT start = std::chrono::system_clock::now();
		sf->BitmapToMat(&img);
		variables->capture_time = SF_TIME_DIFFERENCE(std::chrono::system_clock::now() - start).count();

		SF_TIME_POINT start1 = std::chrono::system_clock::now();
		dml->Preprocess(img);
		dml->Detect();
		dml->Postprocess(dml->floatarr,dml->num_classes,dml->Input_size, dml->Anchorbox_num);

		variables->pred_time = SF_TIME_DIFFERENCE(std::chrono::system_clock::now() - start1).count();

		dml->LockStart();
		DrawBox(img);

		//! ����FPS
		if (variables->fps_limit) {
			Delay(variables->fps_limit);
		}
		variables->on_loop = SF_TIME_DIFFERENCE(std::chrono::system_clock::now() - start).count();
	}

	if (cv::getWindowProperty(WINDOWS_NAME, cv::WND_PROP_VISIBLE))
		cv::destroyWindow(WINDOWS_NAME);
	sf->Release();
	g_logger->info("DMLThread: while����...");
Free:
	//�ͷ�
	dml->Release();		 // �ͷ�˳�����
	delete dml;
	// ��λ
	signl->ThreadStopSignl = TRUE;
	signl->ThreadReady = TRUE;
	dml_thread = NULL;
	g_logger->info("DMLThread: ��λ���...");
	g_logger->info("-------------------- DML��ܽ��� --------------------");
	return 0;
}

VOID DMLFrame(){
	SF::Signl::Get().ThreadReady = FALSE;
	if (dml_thread == NULL) {
		dml_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DMLThread, 0, 0, 0);
		if (dml_thread == NULL) {
			SF::Signl::Get().ThreadReady = TRUE;
			g_logger->warn("����DML�����߳�ʧ��");
			MessageBoxA(NULL, "����DML�����߳�ʧ��", MESSAGEBOX_TITLE, MB_OK);
			return;
		}
	}
	g_logger->info("-------------------- ����DML�����߳�PASS --------------------");

}


VOID DrawBox(cv::Mat& img) {
	// ����
	Process* process = &Process::Get();
	if (!SF::Signl::Get().ShowWindow) {
		if (cv::getWindowProperty(WINDOWS_NAME, cv::WND_PROP_VISIBLE))
			cv::destroyWindow(WINDOWS_NAME);
	}
	else {
		for (int i = 0; i < process->indices.size(); ++i) {
			cv::rectangle(img,
				cv::Rect(
					process->boxes[process->indices[i]].x - (process->boxes[process->indices[i]].width * 0.5f),
					process->boxes[process->indices[i]].y - (process->boxes[process->indices[i]].height * 0.5f),
					process->boxes[process->indices[i]].width,
					process->boxes[process->indices[i]].height),
				cv::Scalar(0, 255, 0), 2, 8, 0);
		}
		cv::imshow(WINDOWS_NAME, img);
		cv::waitKey(1);
	}
}