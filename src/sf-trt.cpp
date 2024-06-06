#include "sf-trt.h"
#include "Imgui-module.h"
#include "config-module.h"
#include "dml-module.h"

static const char* theme_items_name[3] = { u8"深蓝",u8"暗紫",u8"蓝白" };
static const char* theme_items_english_name[3] = { u8"Blue",u8"Purple",u8"White" };
static const char* layout_items_name[2] = { u8"新版",u8"旧版"};
static const char* layout_items__english_name[2] = { u8"New",u8"Old"};
static const char* language_ui_name[2] = { u8"中文",u8"English" };

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
	ImGui::Text(GetLanguage() ? u8"性能:[循环:%-3d 截图:%-3d 推理:%-3d FPS:%-4d]" : "Performance:[Loop:%-3d Capture:%-3d Pred:%-3d FPS:%-4d]",
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
	// 加载config	Load Button
	if (IButton(u8"加载config", "Load Config"))
		SF::FileBrowser::Get().config_dialog.Open();
	//ExplanationMake(u8"启动加载一次config文件", "Start Loading a Config File");
	// 保存按钮  Save Button
	if (signl->KeepSave==FALSE) {
		ImGui::SameLine();
		if (IButton(u8"保存参数", "Save Parame")) {
			ConfigModule::Get().SaveParame();
		}
		ExplanationMake(u8"保存参数到Config.ini", "Save Parameters To Config.ini");
	}
	// 实时保存
	ImGui::SameLine();
	ICheckbox(u8"实时保存", "Keep Save", &signl->KeepSave);
	ExplanationMake(u8"cpu开销增大", "Increased Cpu Overhead");

	// 指定config文件名		Specify the cnfig file name
	ImGui::SetNextItemWidth(150);
	ImGui::InputTextWithHint(GetLanguage() ? u8"Config文件名" : u8"Save Config Name", GetLanguage() ? u8"示例:Config.ini" : u8"Example:Config.ini", ConfigModule::Get().Config_name, 20);
	ExplanationMake(u8"保存到指定的ini文件，默认为Config.ini,ASCII编码", "Save to the specified ini file, the default is Config.ini,ASCII encoding");

	// Imgui主题	Imgui theme
	ImGui::Separator();
	ImGui::SetNextItemWidth(80);
	if (GetLanguage())
		ImGui::Combo(u8"##主题", &variables->themeitems, theme_items_name, IM_ARRAYSIZE(theme_items_name));
	else
		ImGui::Combo("##Theme", &variables->themeitems, theme_items_english_name, IM_ARRAYSIZE(theme_items_english_name));
	switch (variables->themeitems) {
	case 0:ImGui::StyleColorsDark(); break;
	case 1:ImGui::StyleColorsClassic(); break;
	case 2:ImGui::StyleColorsLight(); break;
	}
	// 语言ui	language ui
	ImGui::SameLine();
	ImGui::SetNextItemWidth(85);
	ImGui::Combo(GetLanguage() ? u8"##语言" : "##Language", &variables->language, language_ui_name, IM_ARRAYSIZE(language_ui_name));

	//布局样式
	ImGui::SetNextItemWidth(60);
	ImGui::SameLine();
	if (GetLanguage())
		ImGui::Combo(u8"##布局", &variables->layout, layout_items_name, IM_ARRAYSIZE(layout_items_name));
	else
		ImGui::Combo("##Layout", &variables->layout, layout_items__english_name, IM_ARRAYSIZE(layout_items__english_name));

}

static VOID InferenceUI() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();
	SF::Signl* signl = &SF::Signl::Get();

	if (signl->ThreadReady) {
		// 后端
		//ImGui::RadioButton(GetLanguage() ? u8"CUDA" : "CUDA", &variables->inference_backend, 0);
		//ImGui::SameLine();
		//ImGui::RadioButton(GetLanguage() ? u8"DML" : "DML", &variables->inference_backend, 1);
		if (variables->dx_gpu_number > 1) {
			ImGui::Text(GetLanguage() ? u8"指定GPU" : "Designated GPU");
			for (int i = 0; i < variables->dx_gpu_number; ++i) {
				ImGui::SameLine();
				ImGui::RadioButton((char*)std::to_string(i).c_str(), &variables->dx_gpu_idx, i);
			}
			ExplanationMake(u8"指定GPU参与计算,支持DX11的显卡索引,最多支持x4", "Designated GPU to participate in computing,Graphics card index that supports DX11,Max index x4");
		}

		ImGui::SetNextItemWidth(300);
		ImGui::InputText(u8"##onnx_path_cpu", (char*)StringToUTF8(value->cpu_onnx).c_str(), MAX_PATH);
		ImGui::SameLine();
		if (IButton(u8"浏览##onnx_cpu", "Browse##onnx_cpu"))
			SF::FileBrowser::Get().cpu_onnx_dialog.Open();
		

		// 预(后)处理框架	Pre (Post) Processing Framework
		ImGui::Separator();
		ImGui::RadioButton(u8"YOLOV5/7", &variables->process_frame, 0);
		ImGui::SameLine();
		ImGui::RadioButton(u8"YOLOX", &variables->process_frame, 1);
		ImGui::SameLine();
		ImGui::RadioButton(u8"YOLOV8", &variables->process_frame, 2);

		// 移动方式		way of moveing
		ImGui::Separator();
		ImGui::RadioButton(u8"GHUB", &variables->move_way, 0);
		ImGui::SameLine();
		ImGui::RadioButton(u8"易键鼠", &variables->move_way, 1);
		ImGui::SameLine();
		ImGui::RadioButton(u8"SendInput", &variables->move_way, 2);
		ImGui::SameLine();
		ImGui::RadioButton(GetLanguage() ? u8"自定义##移动方式" : u8"Customize##移动方式", &variables->move_way, 3);
		if (variables->move_way == 3) {
			IText(u8"自定义传入的数据", "Customize Data");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);
			ImGui::InputText(u8"##CustomizeData", value->customize_data, MAX_PATH);
		}
		if (IButton(u8"启动线程", "Create Native Thread")) {
			DMLFrame();
		}
	}
	else {
		// 显示推理窗口		show pred window
		ICheckbox(u8"显示推理窗口", "Show Forecast Window", &signl->ShowWindow);
		// 显示性能			show performance
		IText2(int(variables->on_loop), int(variables->capture_time), int(variables->pred_time), int(1000.0 / variables->on_loop));
		if (IButton(u8"退出线程", "Exit Thread")) {
			signl->ThreadStopSignl = FALSE;
		}
	}
}

static VOID ParameUI() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();
	SF::Signl* signl = &SF::Signl::Get();

	ISliderFloat(u8"置信度", "Confidence", &value->confidence, 0.1f, 1.f);
	ISliderFloat(u8"IOU", "IOU", &value->iou, 0.1f, 1.f);
	ISliderFloat(u8"位置", "Location", &value->location, 0.f, 1.f);
	ISliderInt(u8"最大范围", "Max Range", &value->effectiverange, 0, variables->max_range);
	ImGui::SetNextItemWidth(40);
	IInputScalar(u8"最大像素距离", "Max Pixel", &value->max_pixels);

	// 类别选择	
	ImGui::Separator();
	ICheckbox(u8"模型类别", "Model Class", &signl->ModelClasses);
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
		IPushID(u8"状态:ON ", "State:ON ");
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.f / 255, 255.f / 255, 0.0f / 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.f / 255, 225.f / 255, 0.0f / 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.f / 255, 155.f / 255, 0.0f / 255));
		if (IButton(u8"状态:ON ", "State:ON "))
			signl->AimSwitch = FALSE;
		ImGui::PopStyleColor(3);
		ImGui::PopID();
	}
	else {
		IPushID(u8"状态:OFF", "State:OFF");
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255.f / 255, 0.0f / 255, 0.0f / 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(225.f / 255, 0.0f / 255, 0.0f / 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(155.f / 255, 0.0f / 255, 0.0f / 255));
		if (IButton(u8"状态:OFF", "State:OFF"))
			signl->AimSwitch = TRUE;
		ImGui::PopStyleColor(3);
		ImGui::PopID();
	}

	ICheckbox(u8"按键1: ", "Key 1: ", &value->lock_key_switch);
	IRadioButton(u8"左", u8"Left", &value->lock_key, 0x01);
	IRadioButton(u8"右", u8"Right", &value->lock_key, 0x02);
	IRadioButton(u8"侧", u8"Side", &value->lock_key, 0x05);
	
	ImGui::SetNextItemWidth(30);	//设置宽度
	IInputScalar(u8"按键2##自定义按键2", u8"Key2##自定义按键2", &value->lock_key2);
	ExplanationMake(u8"自定义触发按键,对应虚拟键表的十进制数,0为不使用", "Custom trigger button, corresponding to the decimal number of the virtual key table, 0 means not used");
	
	// 自瞄方式
	IRadioButton(u8"按键触发", "Key", &value->lock_model, 0, FALSE);
	IRadioButton(u8"持续触发", "Keep", &value->lock_model, 1);	
	ExplanationMake(u8"按键:按键触发自瞄，持续:存在目标触发移动", "Button: button triggers self-aiming, continuous: there is a target to trigger movement");
	
	//! 监听方式
	IRadioButton(u8"API", "API", &value->monitor_module, 0, FALSE);
	IRadioButton(u8"BOX", "BOX", &value->monitor_module, 1);
	ExplanationMake(u8"监听鼠标按键的方式，API为GetAsyncKeyState函数，BOX需要自定义dll(6.2开发)","API：GetAsyncKeyState，BOX: NULL");
	
	// 扳机
	ImGui::Separator();
	ICheckbox(u8"启用扳机", "Auto Fire", &value->auto_fire);
	if (value->auto_model != 2) {
		ImGui::SetNextItemWidth(30);	//设置宽度
		IInputScalar(u8"扳机按键##自定义扳机按键", u8"Auto Key##自定义扳机按键", &value->auto_key);
	}

	IRadioButton(u8"Send##扳机方式", "Send##扳机方式", &value->auto_click, 0, FALSE);
	IRadioButton(u8"易键鼠##扳机方式", "MSDK##扳机方式", &value->auto_click, 1);
	ExplanationMake(u8"触发左键单击的方式，Send：Sendinput执行，易键鼠：易键鼠执行", u8"Send:SendInput, MSDK：msdk.dll");

	ImGui::SetNextItemWidth(40);	//设置宽度
	IInputScalar(u8"间隔", "Firing Interval", &value->auto_interval);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(40);	//设置宽度
	IInputScalar(u8"随机", "Random", &value->auto_random);

	IRadioButton(u8"方式1", "Model 1", &value->auto_model, 0,FALSE);
	IRadioButton(u8"方式2", "Model 2", &value->auto_model, 1);
	IRadioButton(u8"方式3", "Model 3", &value->auto_model, 2);
	ExplanationMake(u8"方式1:先移动至目标范围内再开枪，方式2:不移动,在目标范围内开枪，方式3:固定延迟后开枪", u8"Method 1: Move to the target range before shooting, Method 2: Do not move, shoot within the target range，Method 3: Shoot after a fixed delay");

	if (value->auto_model != 2) {
		ISliderFloat(u8"比例(X)", "trigger range w", &value->trigger_w, 0.1f, 1.f);
		ISliderFloat(u8"比例(Y)", "trigger range h", &value->trigger_h, 0.1f, 1.f);
	}
}

static VOID PidUI() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();

	ICheckbox(u8"启用PID", "FOV Switch", &value->pid_algorithm);
	IText(u8"·	X轴", u8"·	X Axis");
	ImGui::SliderFloat(u8"P##P_X", &value->P_x, 0.f, 1.f);
	ImGui::SliderFloat(u8"I##I_X", &value->I_x, 0.f, 1.f);
	ImGui::SliderFloat(u8"P##D_X", &value->D_x, 0.f, 1.f);

	ImGui::Separator();
	IText(u8"·	Y轴", u8"·	Y Axis");
	ImGui::SliderFloat(u8"P##P_Y", &value->P_y, 0.f, 1.f);
	ImGui::SliderFloat(u8"I##I_Y", &value->I_y, 0.f, 1.f);
	ImGui::SliderFloat(u8"P##D_Y", &value->D_y, 0.f, 1.f);
}

static VOID FovUI() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();
	ICheckbox(u8"启用FOV", "FOV Switch", &value->fov_algorithm);

	IFovInput(u8"HFOV", &value->hfov);
	IFovPixelInput(u8"X 像素",u8"X Pixel", &value->game_x_pixel);

	IFovInput(u8"VFOV", &value->vfov);
	IFovPixelInput(u8"Y 像素",u8"Y Pixel", &value->game_y_pixel);
	ImGui::SameLine();
	if (IButton(u8"推导", u8"Derive")) {
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

	ISliderInt(u8"FPS上限", "FPS Limit", &variables->fps_limit, 0, 1000);
	ExplanationMake(u8"FPS上限，0为无限制", u8"FPS upper limit, 0 means no limit");
	// 自定义移动dll
	IText(u8"自定义移动Dll", "Dll of move");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120);
	ImGui::InputText(u8"##CustomizeMoveDll", variables->move_dll, MAX_PATH);
	
	// 卡尔曼
	ICheckbox(u8"超前", "advanced", &value->advanced);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(40);
	IInputScalar(u8"超前帧", "advanced fame", &value->advanced_size);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120);
	ISliderFloat(u8"超前倍率", "advanced multiple", &value->advanced_multiple, 0.f, 2.f);


	ImGui::Separator();
	ICheckbox(u8"削抖", "debounce", &value->debounce);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(40);
	IInputScalar(u8"削抖大小", "debounce size", &value->debounce_size);
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
	ImGui::Text(u8"※ ");
	ImGui::SameLine();
	ICheckbox(u8"推理设置", "Inference Settings", &variables->infer_ui);
	if (variables->infer_ui) {
		InferenceUI();
	}

	ImGui::Separator();
	ImGui::Separator();
	ImGui::Text(u8"※ ");
	ImGui::SameLine();
	ICheckbox(u8"参数设置", "Set Parame", &variables->parame_ui);
	if (variables->parame_ui) {
		ParameUI();
	}

	ImGui::Separator();
	ImGui::Separator();
	ImGui::Text(u8"※ ");
	ImGui::SameLine();
	ICheckbox(u8"按键设置", "Set Key", &variables->key_ui);
	if (variables->key_ui) {
		KeyUI();
	}

	ImGui::Separator();
	ImGui::Separator();
	ImGui::Text(u8"※ ");
	ImGui::SameLine();
	ICheckbox(u8"PID控制", "PID Control", &variables->pid_ui);
	if (variables->pid_ui) {
		PidUI();
	}

	ImGui::Separator();
	ImGui::Separator();
	ImGui::Text(u8"※ ");
	ImGui::SameLine();
	ICheckbox(u8"FOV算法", "FOV Algorithm", &variables->fov_ui);
	if (variables->fov_ui) {
		FovUI();
	}
	OpenFileManager();
}

static VOID ImguiLayout2() {
	SF::Variables* variables = &SF::Variables::Get();
	SF::Value* value = &SF::Value::Get();
	IMenu(u8"基础设置", u8"Home     ", &variables->menu_id, 0,FALSE);
	IMenu(u8"推理设置", u8"Inference", &variables->menu_id, 1);
	IMenu(u8"参数设置", u8"Parame   ", &variables->menu_id, 2);
	IMenu(u8"按键设置", u8"Key      ", &variables->menu_id, 3);
	IMenu(u8"PID 控制", u8"PID		", &variables->menu_id, 4, FALSE);
	IMenu(u8"FOV 控制", u8"FOV		", &variables->menu_id, 5);
	IMenu(u8"高级设置", u8"Advanced    ", &variables->menu_id, 6);

	// Tips
	ExplanationMake(u8"Tips: Ctrl + 左键可直接修改滑块数值", u8"·Ctrl + left can directly modify the slider value", FALSE);
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
	// dx 显卡
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

	// imgui 初始化
	if (!imgui_module->InitImguiMdule()) return;

	// 获取显卡数量 （dml and cuda）
	GetGPUCount(variables);

	// 自动加载一次参数
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
	g_logger->warn("显卡驱动要求 > 527.41,当前驱动版本:{}", info.c_str());
	char* tisp = GetLanguage() ? "显卡驱动版本要求>527.41，请更新驱动。“是”继续执行，可能发生未知错误，“否”退出" : " Graphics card driver version requirement > 527.41, please update the driver. \"Yes\" to continue execution, unknown error may occur, \"No\" to exit";
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
		g_logger->warn("获取系统内部版本号错误，可能发生未知错误");
	}
	if (std::stoi(Id) < 1903) {
		g_logger->warn("系统内部要求1903，当前版本:{},请升级系统", Id.c_str());
		char temp[MAX_PATH]{};
		_snprintf_s(temp, MAX_PATH, GetLanguage() ? "运行系统最低版本:1903，当前系统版本:%s,请升级系统" : "Minimum operating system version: 1903, current system version: %s, please upgrade the system", Id2.c_str());
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
		g_logger->info("获取令牌句柄失败");
		MessageBoxA(NULL, "获取令牌句柄失败", MESSAGEBOX_TITLE, MB_OK);
		return FALSE;
	}

	retn = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Luid);
	if (retn != TRUE) {
		g_logger->info("获取Luid失败");
		MessageBoxA(NULL, "获取Luid失败", MESSAGEBOX_TITLE, MB_OK);
		return FALSE;
	}

	//给TP和TP里的LUID结构体赋值
	TOKEN_PRIVILEGES tp{}; //新特权结构体
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = Luid;

	//调整权限
	AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (GetLastError() == ERROR_SUCCESS) {
		return TRUE;
	}
	g_logger->info("获取UAC权限不完全或失败,请手动以管理员身份运行,错误码:{}",std::to_string(GetLastError()).c_str());
	char temp[MAX_PATH]{};
	_snprintf_s(temp, MAX_PATH, GetLanguage() ? "获取UAC权限不完全或失败,\"是\"继续运行,\"否\"退出程序。" : "Obtaining UAC permission is incomplete or failed, \"Yes\" to continue running, \"No\" to exit the program.");
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
		g_logger->info("CheckDriverVersion 异常");
	}
	
	if (!CheckUAC()) return 0;

	start_main();
	g_logger->info("main exit...");
	return 0;
}

