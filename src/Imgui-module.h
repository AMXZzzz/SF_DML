#pragma once
#include <windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_internal.h>
#include <imgui/imstb_rectpack.h>
#include <imgui/imstb_truetype.h>
#include <imgui/imstb_truetype.h>
#include <imgui/imfilebrowser.h>
#include <spdlog/sinks/basic_file_sink.h>


#define ReleaseCom(x)	{if(x){x->Release();x=nullptr;}}

struct ImguiModule {
	ImguiModule() {};
	~ImguiModule() {};

	static ImguiModule& Get() {
		static ImguiModule m_pInstance;
		return m_pInstance;
	}
	ID3D11Device* g_pd3dDevice = NULL;
	IDXGISwapChain* g_pSwapChain = NULL;

	VOID CleanupRenderBuffer();
	VOID CreateRanderBuffer();
	VOID RenderImguiLayout(VOID(*fun)());
	BOOL InitImguiMdule();
	VOID CloseD3D();

private:

	BOOL CreateImguiWinClass();
	VOID SetNextFrame();
	VOID PresentFrame();

	const float clear_color_with_alpha[4] = {0.45f, 0.55f, 0.60f, 1.00f };

	ImGuiIO* io;
	HWND hwnd;
	ID3D11RenderTargetView* g_mainRenderTargetView = NULL;
	ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
};


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern std::shared_ptr<spdlog::logger> g_logger;	// 全局对象

