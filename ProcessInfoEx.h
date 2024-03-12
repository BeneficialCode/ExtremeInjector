#pragma once

#include "imgui.h"
#include <ProcessInfo.h>
#include <ProcessManager.h>
#include <Processes.h>
#include <d3d11.h>

enum class ProcessAttributes {
	NotComputed = -1,
	None = 0,
	Protected = 1,
	InJob = 2,
	Service = 4,
	Managed = 8,
	Secure = 0x10,
	Immersive = 0x20,
};
DEFINE_ENUM_FLAG_OPERATORS(ProcessAttributes);

class ProcessInfoEx {
public:
	ProcessInfoEx(WinSys::ProcessInfo* pi):_pi(pi){
		_process = WinSys::Process::OpenById(_pi->Id, WinSys::ProcessAccessMask::QueryLimitedInformation);
	}

	bool IsNew() const { return _isNew; }
	bool IsTerminated() const { return _isTerminated; }

	std::pair<const ImVec4&, const ImVec4&> GetColors(WinSys::ProcessManager& pm) const;
	ProcessAttributes GetAttributes(WinSys::ProcessManager& pm) const;
	const std::wstring& UserName() const;

	bool Update();
	void New(uint32_t ms);
	void Term(uint32_t ms);
	const std::wstring& GetExecutablePath() const;
	ID3D11ShaderResourceView* Icon() const;

	int GetBitness() const;

	bool Filtered{ false };

private:
	std::unique_ptr<WinSys::Process> _process;
	mutable CComPtr<ID3D11ShaderResourceView> m_spIcon;
	DWORD _expiryTime = 0;
	WinSys::ProcessInfo* _pi = nullptr;
	mutable std::wstring _executablePath;
	mutable ProcessAttributes _attributes = ProcessAttributes::NotComputed;
	mutable std::wstring _username;
	bool _isNew : 1 = false, _isTerminated : 1 = false;
	mutable int _bitness{ 0 };
};