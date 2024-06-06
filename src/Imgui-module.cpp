#include "Imgui-module.h"
#include "sf-trt.h"


LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	//ImguiModule* imguimodule = &ImguiModule::Get();
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return TRUE;
	switch (msg) {
	case WM_SIZE:	// 如果窗口是固定大小则无需这些代码，
		//if (imguimodule->g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
		//	imguimodule->CleanupRenderBuffer();
		//	imguimodule->g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
		//	imguimodule->CreateRanderBuffer();
		//}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // 禁用 ALT 应用程序菜单
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

VOID ImguiModule::CreateRanderBuffer() {
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (pBackBuffer != NULL)
		g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

VOID ImguiModule::CleanupRenderBuffer() {
	if (g_mainRenderTargetView) {
		g_mainRenderTargetView->Release();
		g_mainRenderTargetView = NULL;
	}
}

VOID ImguiModule::CloseD3D() {
	ReleaseCom(g_pd3dDevice);
	ReleaseCom(g_pd3dDeviceContext);
	ReleaseCom(g_pSwapChain);
	ReleaseCom(g_mainRenderTargetView);
	DestroyWindow(hwnd);
	g_logger->info("imgui释放 PASS...");
}

BOOL ImguiModule::CreateImguiWinClass() {
	// 注册窗口类		Create winClass
	WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"Temp Example", NULL };
	RegisterClassExW(&wc);
	hwnd = CreateWindow(wc.lpszClassName, L"SF_TRT 6.1", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

	//初始化d3d		Init D3D
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK) {
		CleanupRenderBuffer();
		if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
		if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
		if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
		g_logger->warn("D3D11CreateDeviceAndSwapChain失败");
		return FALSE;
	}
	return TRUE;
}

BOOL ImguiModule::InitImguiMdule() {
	if (!CreateImguiWinClass()) return FALSE;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// 设置imgui的IO		Set the IO of imgui
	io = &ImGui::GetIO(); 
	//(void*)io;						// 转为万能指针		Convert to Universal Pointer
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// 启用键盘控制
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;			// 启用对接
	io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;			// 取消保存ini文件
	io->IniFilename = NULL;
	io->ConfigViewportsNoAutoMerge = TRUE;

	ImFont* font = io->Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\simhei.ttf", 16.f, NULL, io->Fonts->GetGlyphRangesChineseFull());
	ImGuiStyle& style = ImGui::GetStyle();

	if (io->ConfigFlags == ImGuiConfigFlags_ViewportsEnable) {
		style.Colors[ImGuiCol_WindowBg].w = 10.0f;	// 颜色深度
		style.WindowRounding = 10.0f;				// 角弧度
	}
	// 初始化渲染后端		Initialize the rendering backend
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	// 设置控件样式		Set control style
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);			// 控件弧度
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5);			// 边缘阴影
	ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 10.f);				// 滑条滑块弧度
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 5.f);			// 主窗口边框阴影

	SF::FileBrowser* filebrowser = &SF::FileBrowser::Get();

	filebrowser->build_onnx_dialog.SetTypeFilters({ ".onnx",".sftrt" });
	filebrowser->build_onnx_dialog.SetTitle("onnx##ONNX文件浏览器");

	filebrowser->cpu_onnx_dialog.SetTypeFilters({ ".onnx" });
	filebrowser->cpu_onnx_dialog.SetTitle("onnx##cpuONNX文件浏览器");

	filebrowser->engine_dialog.SetTypeFilters({ ".engine" ,".trt" });
	filebrowser->engine_dialog.SetTitle("engine##engine文件浏览器");

	filebrowser->config_dialog.SetTypeFilters({ ".ini" });
	filebrowser->config_dialog.SetTitle("config##config文件浏览器");

	return TRUE;
}

VOID ImguiModule::SetNextFrame() {
	MSG msg{};
	while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT) {
			//Signl.ImGuiWinStop = FALSE;	// 激活停止信号
			g_logger->info("imgui主窗口关闭...");
		}
	}

	// 使用imgui渲染帧		Render frames using imgui
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

VOID ImguiModule::PresentFrame() {
	// 绑定到管线		bind to the pipe
	ImGui::Render();
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);	//获取管线
	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);	// 初始化管线
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());	// 获取ImGui的控件Buff
	if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();	//绑定到管线
	}
	g_pSwapChain->Present(1, 0); // 展示到屏幕	Present
}

#if SFUSE
#define SFUSETITLE u8"SF_DML 编译时间:" __DATE__
#else
#define SFUSETITLE u8"SF_DML  | TIME: 2023/10/25"
#endif // SF_USE

VOID ImguiModule::RenderImguiLayout(VOID (*fun)()) {
	SetNextFrame();
	ImGui::Begin(SFUSETITLE, &SF::Signl::Get().ImGuiWinStop, ImGuiWindowFlags_NoSavedSettings + ImGuiWindowFlags_AlwaysAutoResize);

	fun();
	ImGui::End();
	PresentFrame();
}

