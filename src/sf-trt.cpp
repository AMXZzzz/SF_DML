#include "sf-trt.h"
#include "Imgui-module.h"
#include "config-module.h"
#include "dml-module.h"

static const char* theme_items_name[3] = { u8"����",u8"����",u8"����" };
static const char* theme_items_english_name[3] = { u8"Blue",u8"Purple",u8"White" };
static const char* layout_items_name[2] = { u8"�°�",u8"�ɰ�"};
static const char* layout_items__english_name[2] = { u8"New",u8"Old"};
static const char* language_ui_name[2] = { u8"����",u8"English" };

static inline BOOL GetLanguage() {
	return SF::Variables::Get().language == 0;
}
static BOOL IButton(char* label, char* label_engine) {
	return ImGui::Button(GetLanguage() ? label : label_engine);
}
static VOID ICheckbox(char* label, char* label_engine, bool* v) {
	ImGui::Checkbox(GetLanguage() ? label : label_engine, v);
}
static VOID ISliderFloat(char* label, char* label_engine, float* v, float v_min, float v_max) {
	ImGui::SliderFloat(GetLanguage() ? label : label_engine, v, v_min, v_max);
}
static VOID ISliderFloat(char* label, char* label_engine, double* v, float v_min, float v_max) {
	ImGui::SliderFloat(GetLanguage() ? label : label_engine, v, v_min, v_max);
}
static VOID ISliderInt(char* label, char* label_engine, int* v, int v_min, int v_max) {
	ImGui::SliderInt(GetLanguage() ? label : label_engine, v, v_min, v_max);
}
static VOID IInputScalar(char* label, char* label_engine, void* p_data) {
	ImGui::InputScalar(GetLanguage() ? label : label_engine, ImGuiDataType_S32, p_data, NULL, NULL, "%d", NULL);
}
static VOID IText(char* label, char* label_engine) {
	ImGui::Text(GetLanguage() ? label : label_engine);
}
static VOID IText2(int loop, int cap, int pred, int fps) {
	ImGui::Text(GetLanguage() ? u8"����:[ѭ��:%-3d ��ͼ:%-3d ����:%-3d FPS:%-4d]" : "Performance:[Loop:%-3d Capture:%-3d Pred:%-3d FPS:%-4d]",
		loop, cap, pred, fps);
}
static VOID IFovInput(char* label, float* val) {
	ImGui::SetNextItemWidth(80);
	ImGui::InputFloat(label, val);
}
static VOID IFovPixelInput(char* label, char* label2, int* val) {
	ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	ImGui::InputScalar(GetLanguage()? label: label2, ImGuiDataType_S32, val, NULL, NULL, "%d", NULL);
}
static VOID IRadioButton(char* label, char* label_engine, int* key, int val,bool sameline=TRUE) {
	if(sameline)ImGui::SameLine();
	ImGui::RadioButton(GetLanguage() ? label : label_engine, key, val);
}
static VOID IPushID(char* label, char* label_engine) {
	ImGui::PushID(GetLanguage() ? label : label_engine);
}

static inline VOID ClassChoose(int& num, char* label, bool* v) {
	if (num) {
		ImGui::SameLine();
		ImGui::Checkbox(label, v);
		num--;
	}
}
static VOID IMenu(const char* key, const char* key_english, int* val_prt, int val,bool sameline=TRUE) {
	ImGui::SetNextItemWidth(200);
	if (sameline) ImGui::SameLine();
	if (ImGui::Button(GetLanguage() ? key : key_english)) {
		*val_prt = val;
	}
}

static VOID ExplanationMake(char* strdata, char* strdata2, BOOL Same = TRUE) {
	if (Same)
		ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.f);
		ImGui::TextUnformatted(GetLanguage() ? strdata : strdata2);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}
// ================================================= UI ============================================================//

static VOID BaseUI() {
	SF::Signl* signl = &SF::Signl::Get();

	SF::Variables* variables = &SF::Variables::Get();
	// ����config	Load Button
	if (IButton(u8"����config", "Load Config"))
		SF::FileBrowser::Get().config_dialog.Open();
	//ExplanationMake(u8"��������һ��config�ļ�", "Start Loading a Config File");
	// ���水ť  Save Button
	if (signl->KeepSave==FALSE) {
		ImGui::SameLine();
		if (IButton(u8"�������", "Save Parame")) {
			ConfigModule::Get().SaveParame();
		}
		ExplanationMake(u8"���������Config.ini", "Save Parameters To Config.ini");
	}
	// ʵʱ����
	ImGui::SameLine();
	ICheckbox(u8"ʵʱ����", "Keep Save", &signl->KeepSave);
	ExplanationMake(u8"cpu��������", "Increased Cpu Overhead");

	// ָ��config�ļ���		Specify the cnfig file name
	ImGui::SetNextItemWidth(150);
	ImGui::InputTextWithHint(GetLanguage() ? u8"Config�ļ���" : u8"Save Config Name", GetLanguage() ? u8"ʾ��:Config.ini" : u8"Example:Config.ini", ConfigModule::Get().Config_name, 20);
	ExplanationMake(u8"���浽ָ����ini�ļ���Ĭ��ΪConfig.ini,ASCII����", "Save to the specified ini file, the default is Config.ini,ASCII encoding");

	// Imgui����	Imgui theme
	ImGui::Separator();
	ImGui::SetNextItemWidth(80);
	if (GetLanguage())
		ImGui::Combo(u8"##����", &variables->themeitems, theme_items_name, IM_ARRAYSIZE(theme_items_name));
	else
		ImGui::Combo("##Theme", &variables->themeitems, theme_items_english_name, IM_ARRAYSIZE(theme_items_english_name));
	switch (variables->themeitems) {
	case 0:ImGui::StyleColorsDark(); break;
	case 1:ImGui::StyleColorsClassic(); break;
	case 2:ImGui::StyleColorsLight(); break;
	}
	// ����ui	language ui
	ImGui::SameLine();
	ImGui::SetNextItemWidth(85);
	ImGui::Combo(GetLanguage() ? u8"##����" : "##Language", &variables->language, language_ui_name, IM_ARRAYSIZE(language_ui_name));

	//������ʽ
	ImGui::SetNextItemWidth(60);
	ImGui::SameLine();
	if (GetLanguage())
		ImGui::Combo(u8"##����", &variables->layout, layout_items_name, IM_ARRAYSIZE(layout_items_name));
	else
		ImGui::Combo("##Layout", &variables->layout, layout_items__english_name, IM_ARRAYSIZE(layout_items__english_name));

}

static VOID InferenceUI() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();
	SF::Signl* signl = &SF::Signl::Get();

	if (signl->ThreadReady) {
		// ���
		//ImGui::RadioButton(GetLanguage() ? u8"CUDA" : "CUDA", &variables->inference_backend, 0);
		//ImGui::SameLine();
		//ImGui::RadioButton(GetLanguage() ? u8"DML" : "DML", &variables->inference_backend, 1);
		if (variables->dx_gpu_number > 1) {
			ImGui::Text(GetLanguage() ? u8"ָ��GPU" : "Designated GPU");
			for (int i = 0; i < variables->dx_gpu_number; ++i) {
				ImGui::SameLine();
				ImGui::RadioButton((char*)std::to_string(i).c_str(), &variables->dx_gpu_idx, i);
			}
			ExplanationMake(u8"ָ��GPU�������,֧��DX11���Կ�����,���֧��x4", "Designated GPU to participate in computing,Graphics card index that supports DX11,Max index x4");
		}

		ImGui::SetNextItemWidth(300);
		ImGui::InputText(u8"##onnx_path_cpu", (char*)StringToUTF8(value->cpu_onnx).c_str(), MAX_PATH);
		ImGui::SameLine();
		if (IButton(u8"���##onnx_cpu", "Browse##onnx_cpu"))
			SF::FileBrowser::Get().cpu_onnx_dialog.Open();
		

		// Ԥ(��)������	Pre (Post) Processing Framework
		ImGui::Separator();
		ImGui::RadioButton(u8"YOLOV5/7", &variables->process_frame, 0);
		ImGui::SameLine();
		ImGui::RadioButton(u8"YOLOX", &variables->process_frame, 1);
		ImGui::SameLine();
		ImGui::RadioButton(u8"YOLOV8", &variables->process_frame, 2);

		// �ƶ���ʽ		way of moveing
		ImGui::Separator();
		ImGui::RadioButton(u8"GHUB", &variables->move_way, 0);
		ImGui::SameLine();
		ImGui::RadioButton(u8"�׼���", &variables->move_way, 1);
		ImGui::SameLine();
		ImGui::RadioButton(u8"SendInput", &variables->move_way, 2);
		ImGui::SameLine();
		ImGui::RadioButton(GetLanguage() ? u8"�Զ���##�ƶ���ʽ" : u8"Customize##�ƶ���ʽ", &variables->move_way, 3);
		if (variables->move_way == 3) {
			IText(u8"�Զ��崫�������", "Customize Data");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);
			ImGui::InputText(u8"##CustomizeData", value->customize_data, MAX_PATH);
		}
		if (IButton(u8"�����߳�", "Create Native Thread")) {
			DMLFrame();
		}
	}
	else {
		// ��ʾ������		show pred window
		ICheckbox(u8"��ʾ������", "Show Forecast Window", &signl->ShowWindow);
		// ��ʾ����			show performance
		IText2(int(variables->on_loop), int(variables->capture_time), int(variables->pred_time), int(1000.0 / variables->on_loop));
		if (IButton(u8"�˳��߳�", "Exit Thread")) {
			signl->ThreadStopSignl = FALSE;
		}
	}
}

static VOID ParameUI() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();
	SF::Signl* signl = &SF::Signl::Get();

	ISliderFloat(u8"���Ŷ�", "Confidence", &value->confidence, 0.1f, 1.f);
	ISliderFloat(u8"IOU", "IOU", &value->iou, 0.1f, 1.f);
	ISliderFloat(u8"λ��", "Location", &value->location, 0.f, 1.f);
	ISliderInt(u8"���Χ", "Max Range", &value->effectiverange, 0, variables->max_range);
	ImGui::SetNextItemWidth(40);
	IInputScalar(u8"������ؾ���", "Max Pixel", &value->max_pixels);

	// ���ѡ��	
	ImGui::Separator();
	ICheckbox(u8"ģ�����", "Model Class", &signl->ModelClasses);
	if (signl->ModelClasses) {
		int class_num_temp = value->class_number;
		ClassChoose(class_num_temp, u8"0", &value->class0);
		ClassChoose(class_num_temp, u8"1", &value->class1);
		ClassChoose(class_num_temp, u8"2", &value->class2);
		ClassChoose(class_num_temp, u8"3", &value->class3);
	}
}

static VOID KeyUI() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();
	SF::Signl* signl = &SF::Signl::Get();

	if (signl->AimSwitch == TRUE) {
		IPushID(u8"״̬:ON ", "State:ON ");
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.f / 255, 255.f / 255, 0.0f / 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.f / 255, 225.f / 255, 0.0f / 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.f / 255, 155.f / 255, 0.0f / 255));
		if (IButton(u8"״̬:ON ", "State:ON "))
			signl->AimSwitch = FALSE;
		ImGui::PopStyleColor(3);
		ImGui::PopID();
	}
	else {
		IPushID(u8"״̬:OFF", "State:OFF");
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255.f / 255, 0.0f / 255, 0.0f / 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(225.f / 255, 0.0f / 255, 0.0f / 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(155.f / 255, 0.0f / 255, 0.0f / 255));
		if (IButton(u8"״̬:OFF", "State:OFF"))
			signl->AimSwitch = TRUE;
		ImGui::PopStyleColor(3);
		ImGui::PopID();
	}

	ICheckbox(u8"����1: ", "Key 1: ", &value->lock_key_switch);
	IRadioButton(u8"��", u8"Left", &value->lock_key, 0x01);
	IRadioButton(u8"��", u8"Right", &value->lock_key, 0x02);
	IRadioButton(u8"��", u8"Side", &value->lock_key, 0x05);
	
	ImGui::SetNextItemWidth(30);	//���ÿ��
	IInputScalar(u8"����2##�Զ��尴��2", u8"Key2##�Զ��尴��2", &value->lock_key2);
	ExplanationMake(u8"�Զ��崥������,��Ӧ��������ʮ������,0Ϊ��ʹ��", "Custom trigger button, corresponding to the decimal number of the virtual key table, 0 means not used");
	
	// ���鷽ʽ
	IRadioButton(u8"��������", "Key", &value->lock_model, 0, FALSE);
	IRadioButton(u8"��������", "Keep", &value->lock_model, 1);	
	ExplanationMake(u8"����:�����������飬����:����Ŀ�괥���ƶ�", "Button: button triggers self-aiming, continuous: there is a target to trigger movement");
	
	//! ������ʽ
	IRadioButton(u8"API", "API", &value->monitor_module, 0, FALSE);
	IRadioButton(u8"BOX", "BOX", &value->monitor_module, 1);
	ExplanationMake(u8"������갴���ķ�ʽ��APIΪGetAsyncKeyState������BOX��Ҫ�Զ���dll(6.2����)","API��GetAsyncKeyState��BOX: NULL");
	
	// ���
	ImGui::Separator();
	ICheckbox(u8"���ð��", "Auto Fire", &value->auto_fire);
	if (value->auto_model != 2) {
		ImGui::SetNextItemWidth(30);	//���ÿ��
		IInputScalar(u8"�������##�Զ���������", u8"Auto Key##�Զ���������", &value->auto_key);
	}

	IRadioButton(u8"Send##�����ʽ", "Send##�����ʽ", &value->auto_click, 0, FALSE);
	IRadioButton(u8"�׼���##�����ʽ", "MSDK##�����ʽ", &value->auto_click, 1);
	ExplanationMake(u8"������������ķ�ʽ��Send��Sendinputִ�У��׼����׼���ִ��", u8"Send:SendInput, MSDK��msdk.dll");

	ImGui::SetNextItemWidth(40);	//���ÿ��
	IInputScalar(u8"���", "Firing Interval", &value->auto_interval);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(40);	//���ÿ��
	IInputScalar(u8"���", "Random", &value->auto_random);

	IRadioButton(u8"��ʽ1", "Model 1", &value->auto_model, 0,FALSE);
	IRadioButton(u8"��ʽ2", "Model 2", &value->auto_model, 1);
	IRadioButton(u8"��ʽ3", "Model 3", &value->auto_model, 2);
	ExplanationMake(u8"��ʽ1:���ƶ���Ŀ�귶Χ���ٿ�ǹ����ʽ2:���ƶ�,��Ŀ�귶Χ�ڿ�ǹ����ʽ3:�̶��ӳٺ�ǹ", u8"Method 1: Move to the target range before shooting, Method 2: Do not move, shoot within the target range��Method 3: Shoot after a fixed delay");

	if (value->auto_model != 2) {
		ISliderFloat(u8"����(X)", "trigger range w", &value->trigger_w, 0.1f, 1.f);
		ISliderFloat(u8"����(Y)", "trigger range h", &value->trigger_h, 0.1f, 1.f);
	}
}

static VOID PidUI() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();

	ICheckbox(u8"����PID", "FOV Switch", &value->pid_algorithm);
	IText(u8"��	X��", u8"��	X Axis");
	ImGui::SliderFloat(u8"P##P_X", &value->P_x, 0.f, 1.f);
	ImGui::SliderFloat(u8"I##I_X", &value->I_x, 0.f, 1.f);
	ImGui::SliderFloat(u8"P##D_X", &value->D_x, 0.f, 1.f);

	ImGui::Separator();
	IText(u8"��	Y��", u8"��	Y Axis");
	ImGui::SliderFloat(u8"P##P_Y", &value->P_y, 0.f, 1.f);
	ImGui::SliderFloat(u8"I##I_Y", &value->I_y, 0.f, 1.f);
	ImGui::SliderFloat(u8"P##D_Y", &value->D_y, 0.f, 1.f);
}

static VOID FovUI() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();
	ICheckbox(u8"����FOV", "FOV Switch", &value->fov_algorithm);

	IFovInput(u8"HFOV", &value->hfov);
	IFovPixelInput(u8"X ����",u8"X Pixel", &value->game_x_pixel);

	IFovInput(u8"VFOV", &value->vfov);
	IFovPixelInput(u8"Y ����",u8"Y Pixel", &value->game_y_pixel);
	ImGui::SameLine();
	if (IButton(u8"�Ƶ�", u8"Derive")) {
		value->game_y_pixel = (value->vfov / value->hfov) * value->game_x_pixel;
	}
}

static VOID OpenFileManager() {
	SF::FileBrowser* filebrowser = &SF::FileBrowser::Get();
	SF::Value* value = &SF::Value::Get();

	filebrowser->config_dialog.Display();
	if (filebrowser->config_dialog.HasSelected()) {
		ConfigModule::Get().GetParame((char*)filebrowser->config_dialog.GetSelected().string().c_str());
		filebrowser->config_dialog.ClearSelected();
	}

	filebrowser->build_onnx_dialog.Display();
	if (filebrowser->build_onnx_dialog.HasSelected()) {
		//strcpy_s(value->build_onnx, StringToUTF8(filebrowser->build_onnx_dialog.GetSelected().string()).c_str());
		strcpy_s(value->build_onnx, (filebrowser->build_onnx_dialog.GetSelected().string()).c_str());
		filebrowser->build_onnx_dialog.ClearSelected();
	}

	filebrowser->cpu_onnx_dialog.Display();
	if (filebrowser->cpu_onnx_dialog.HasSelected()) {
		strcpy_s(value->cpu_onnx, (filebrowser->cpu_onnx_dialog.GetSelected().string()).c_str());
		filebrowser->cpu_onnx_dialog.ClearSelected();
	}

	filebrowser->engine_dialog.Display();
	if (filebrowser->engine_dialog.HasSelected()) {
		strcpy_s(value->engine_path, StringToUTF8(filebrowser->engine_dialog.GetSelected().string()).c_str());
		filebrowser->engine_dialog.ClearSelected();
	}
}

static VOID AdvancedSettings() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();

	ISliderInt(u8"FPS����", "FPS Limit", &variables->fps_limit, 0, 1000);
	ExplanationMake(u8"FPS���ޣ�0Ϊ������", u8"FPS upper limit, 0 means no limit");
	// �Զ����ƶ�dll
	IText(u8"�Զ����ƶ�Dll", "Dll of move");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120);
	ImGui::InputText(u8"##CustomizeMoveDll", variables->move_dll, MAX_PATH);
	
	// ������
	ICheckbox(u8"��ǰ", "advanced", &value->advanced);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(40);
	IInputScalar(u8"��ǰ֡", "advanced fame", &value->advanced_size);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120);
	ISliderFloat(u8"��ǰ����", "advanced multiple", &value->advanced_multiple, 0.f, 2.f);


	ImGui::Separator();
	ICheckbox(u8"����", "debounce", &value->debounce);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(40);
	IInputScalar(u8"������С", "debounce size", &value->debounce_size);
}

static VOID ImguiLayout() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();
	SF::Signl* signl = &SF::Signl::Get();

	ImGui::Separator();
	ImGui::Separator();
	BaseUI();

	ImGui::Separator();
	ImGui::Separator();
	ImGui::Text(u8"�� ");
	ImGui::SameLine();
	ICheckbox(u8"��������", "Inference Settings", &variables->infer_ui);
	if (variables->infer_ui) {
		InferenceUI();
	}

	ImGui::Separator();
	ImGui::Separator();
	ImGui::Text(u8"�� ");
	ImGui::SameLine();
	ICheckbox(u8"��������", "Set Parame", &variables->parame_ui);
	if (variables->parame_ui) {
		ParameUI();
	}

	ImGui::Separator();
	ImGui::Separator();
	ImGui::Text(u8"�� ");
	ImGui::SameLine();
	ICheckbox(u8"��������", "Set Key", &variables->key_ui);
	if (variables->key_ui) {
		KeyUI();
	}

	ImGui::Separator();
	ImGui::Separator();
	ImGui::Text(u8"�� ");
	ImGui::SameLine();
	ICheckbox(u8"PID����", "PID Control", &variables->pid_ui);
	if (variables->pid_ui) {
		PidUI();
	}

	ImGui::Separator();
	ImGui::Separator();
	ImGui::Text(u8"�� ");
	ImGui::SameLine();
	ICheckbox(u8"FOV�㷨", "FOV Algorithm", &variables->fov_ui);
	if (variables->fov_ui) {
		FovUI();
	}
	OpenFileManager();
}

static VOID ImguiLayout2() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();
	IMenu(u8"��������", u8"Home     ", &variables->menu_id, 0,FALSE);
	IMenu(u8"��������", u8"Inference", &variables->menu_id, 1);
	IMenu(u8"��������", u8"Parame   ", &variables->menu_id, 2);
	IMenu(u8"��������", u8"Key      ", &variables->menu_id, 3);
	IMenu(u8"PID ����", u8"PID		", &variables->menu_id, 4, FALSE);
	IMenu(u8"FOV ����", u8"FOV		", &variables->menu_id, 5);
	IMenu(u8"�߼�����", u8"Advanced    ", &variables->menu_id, 6);

	// Tips
	ExplanationMake(u8"Tips: Ctrl + �����ֱ���޸Ļ�����ֵ", u8"��Ctrl + left can directly modify the slider value", FALSE);
	ImGui::Separator();
	ImGui::Separator();

	switch (variables->menu_id) {
	case 0:
		BaseUI();
		break;
	case 1:
		InferenceUI();
		break;
	case 2:
		ParameUI();
		break;
	case 3:
		KeyUI();
		break;
	case 4:
		PidUI();
		break;
	case 5:
		FovUI();
		break;
	case 6:
		AdvancedSettings();
		break;
	}

	OpenFileManager();
}

// ======================================================================================================================== //
static VOID GetGPUCount(SF::Variables* variables) {
	// dx �Կ�
	IDXGIFactory* pFactory;
	IDXGIAdapter* pAdapter;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)(&pFactory));

	while (pFactory->EnumAdapters(variables->dx_gpu_number, &pAdapter) != DXGI_ERROR_NOT_FOUND){
		++variables->dx_gpu_number;
	}
}

static VOID start_main() {
	ImguiModule* imgui_module = &ImguiModule::Get();
	SF::Signl* signl = &SF::Signl::Get();
	//Examine* examine = &Examine::Get();
	SF::Variables* variables = &SF::Variables::Get();
	ConfigModule* config = &ConfigModule::Get();

	// imgui ��ʼ��
	if (!imgui_module->InitImguiMdule()) return;

	// ��ȡ�Կ����� ��dml and cuda��
	GetGPUCount(variables);

	// �Զ�����һ�β���
	config->GetParame((char*)GetIniPath().c_str());

	// run
	while (signl->ImGuiWinStop) {

		imgui_module->RenderImguiLayout(variables->layout ? ImguiLayout : ImguiLayout2);
		if (signl->KeepSave) {
			ConfigModule::Get().SaveParame();
	}
	}
	imgui_module->CloseD3D();

}

static BOOL CheckDriverVersion() {
	std::string temp = "Driver Version: ";
	std::string info = exe_cmd("nvidia-smi");
	info = info.substr(info.find("Driver Version:") + temp.size(), 6);
	if (std::stof(info) > 527.41) {
		return TRUE;
	}
	g_logger->warn("�Կ�����Ҫ�� > 527.41,��ǰ�����汾:{}", info.c_str());
	char* tisp = GetLanguage() ? "�Կ������汾Ҫ��>527.41����������������ǡ�����ִ�У����ܷ���δ֪���󣬡����˳�" : " Graphics card driver version requirement > 527.41, please update the driver. \"Yes\" to continue execution, unknown error may occur, \"No\" to exit";
	int hr = MessageBoxA(NULL, tisp, MESSAGEBOX_TITLE, MB_YESNO);
	if (hr == IDYES) {
		return TRUE;
	}
	return FALSE;
}

static BOOL CheckSystemReleaseId() {
	std::string Id = GetSystemReleaseId();
	std::string Id2 = Id.c_str();	
	if (std::stoi(Id) == -1) {
		g_logger->warn("��ȡϵͳ�ڲ��汾�Ŵ��󣬿��ܷ���δ֪����");
	}
	if (std::stoi(Id) < 1903) {
		g_logger->warn("ϵͳ�ڲ�Ҫ��1903����ǰ�汾:{},������ϵͳ", Id.c_str());
		char temp[MAX_PATH]{};
		_snprintf_s(temp, MAX_PATH, GetLanguage() ? "����ϵͳ��Ͱ汾:1903����ǰϵͳ�汾:%s,������ϵͳ" : "Minimum operating system version: 1903, current system version: %s, please upgrade the system", Id2.c_str());
		MessageBoxA(NULL, temp, MESSAGEBOX_TITLE, MB_OK);
		return FALSE;
	}
	return TRUE;
}

static BOOL CheckUAC() {
	BOOL retn;
	HANDLE hToken;
	LUID Luid;

	retn = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
	if (retn != TRUE) {
		g_logger->info("��ȡ���ƾ��ʧ��");
		MessageBoxA(NULL, "��ȡ���ƾ��ʧ��", MESSAGEBOX_TITLE, MB_OK);
		return FALSE;
	}

	retn = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Luid);
	if (retn != TRUE) {
		g_logger->info("��ȡLuidʧ��");
		MessageBoxA(NULL, "��ȡLuidʧ��", MESSAGEBOX_TITLE, MB_OK);
		return FALSE;
	}

	//��TP��TP���LUID�ṹ�帳ֵ
	TOKEN_PRIVILEGES tp{}; //����Ȩ�ṹ��
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = Luid;

	//����Ȩ��
	AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (GetLastError() == ERROR_SUCCESS) {
		return TRUE;
	}
	g_logger->info("��ȡUACȨ�޲���ȫ��ʧ��,���ֶ��Թ���Ա�������,������:{}",std::to_string(GetLastError()).c_str());
	char temp[MAX_PATH]{};
	_snprintf_s(temp, MAX_PATH, GetLanguage() ? "��ȡUACȨ�޲���ȫ��ʧ��,\"��\"��������,\"��\"�˳�����" : "Obtaining UAC permission is incomplete or failed, \"Yes\" to continue running, \"No\" to exit the program.");
	int hr = MessageBoxA(NULL, temp, MESSAGEBOX_TITLE, MB_YESNO);
	if (hr == IDYES) {
		return TRUE;
	}
	return FALSE;
}

#if 1
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
) {

#else
extern "C" API int main() {
#endif // _WINDOWS

	InitSpdLog();

	try {
		if (!CheckDriverVersion()) return 0;
	}
	catch (...) {
		g_logger->info("CheckDriverVersion �쳣");
	}
	
	if (!CheckUAC()) return 0;

	start_main();
	g_logger->info("main exit...");
	return 0;
}

