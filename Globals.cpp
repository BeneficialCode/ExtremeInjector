#include "pch.h"
#include "Globals.h"
#include <assert.h>


Globals::Globals(HWND hwnd) : _hWnd(hwnd) {
	assert(_globals == nullptr);
	_globals = this;
	_tabs.reset(new TabManager);
}

Globals& Globals::Get() {
	assert(_globals);
	return *_globals;
}

HWND Globals::GetMainHwnd() const {
	return _hWnd;
}

TabManager& Globals::GetTabManager() {
	return *_tabs;
}

Settings& Globals::GetSettings() {
	return _settings;
}