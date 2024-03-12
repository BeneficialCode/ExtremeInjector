#pragma once
#include "imgui.h"
#include <ProcessManager.h>
#include "TabManager.h"
#include "Settings.h"


class Globals {
public:
	Globals(HWND hWnd);

	static Globals& Get();

	ImFont* MonoFont{ nullptr };
	ImFont* RegFont{ nullptr };
	HWND GetMainHwnd() const;

	WinSys::ProcessManager ProcMgr;
	TabManager& GetTabManager();
	Settings& GetSettings();

private:
	Settings _settings;
	inline static Globals* _globals{ nullptr };
	std::unique_ptr<TabManager> _tabs;
	HWND _hWnd;
};
