﻿#include "pch.h"
#include "TabManager.h"
#include "Globals.h"

using namespace ImGui;

TabManager::TabManager() :_procView(*this) {

}

void TabManager::BuildMainMenu()
{
	if (ImGui::BeginMainMenuBar()) {
		BuildFileMenu();
		BuildOptionsMenu();
		BuildWindowMenu();
		BuildHelpMenu();
		ImGui::EndMainMenuBar();
	}
}

void TabManager::BuildTabs()
{
	if (Begin("main", nullptr,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
		BuildMainMenu();
		auto size = GetIO().DisplaySize;
		auto height = 21.0f;
		
		SetWindowSize(ImVec2(size.x, size.y - height), ImGuiCond_Always);
		SetWindowPos(ImVec2(0, height), ImGuiCond_Always);

		if (BeginTabBar("tabs", ImGuiTabBarFlags_Reorderable)) {
			if (BeginTabItem("Processes", nullptr, ImGuiTabItemFlags_None)) {
				_procView.BuildWindow();
				EndTabItem();
			}
			EndTabBar();
		}
	}
	End();
}

void TabManager::BuildOptionsMenu()
{
	if (BeginMenu("Options")) {
		if (MenuItem("Always On Top", nullptr, &_alwaysOnTop)) {
			::SetWindowPos(Globals::Get().GetMainHwnd(), 
				_alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		Separator();
		if (BeginMenu("Theme")) {
			if (MenuItem("Classic", nullptr, _theme == Theme::Classic)) {
				StyleColorsClassic();
				_theme = Theme::Classic;
			}
			if (MenuItem("Light", nullptr, _theme == Theme::Light)) {
				StyleColorsLight();
				_theme = Theme::Light;
			}
			if (MenuItem("Dark", nullptr, _theme == Theme::Dark)) {
				StyleColorsDark();
				_theme = Theme::Dark;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
}

void TabManager::BuildFileMenu()
{
	if (BeginMenu("File")) {
		if (MenuItem("Exit")) {
			::PostQuitMessage(0);
		}
		ImGui::EndMenu();
	}
}

void TabManager::BuildWindowMenu()
{
	std::vector<std::string> names;
	for (auto& [name, win] : _windows) {
		if (!win->WindowOpen)
			names.push_back(name);
	}

	for(auto& name: names)
		_windows.erase(name);

	if(_windows.empty())
		return;

	if (BeginMenu("Window")) {
		CStringA text;
		for (auto& [name, p] : _windows) {
			if (MenuItem(name.c_str()))
				SetWindowFocus(name.c_str());
		}
		ImGui::EndMenu();
	}
}

void TabManager::BuildHelpMenu()
{
	static bool open = false;
	if (BeginMenu("Help")) {
		if(MenuItem("About Extreme Injector"))
			open = true;
		ImGui::EndMenu();
	}

	if (open) {
		auto title = "About Extreme Injector";
		if (MessageBoxResult::OK == SimpleMessageBox::ShowModal(title,
			"Extreme Injector \n\n by VirtualCC")) {
			open = false;
		}
	}
}

void TabManager::AddWindow(std::shared_ptr<WindowProperties> window)
{
	_windows.insert({ window->GetName(),window });
}
