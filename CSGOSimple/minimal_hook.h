#pragma once
#include "valve_sdk/sdk.hpp"
#include "valve_sdk/csgostructs.hpp"
#include <d3d9.h>
#include "helpers/prediction.hpp"
#include "minhookde.h"
#include "MininmalHook/minhook.h"

using HookType = MinHook;



class detour {
public:
	detour() = default;

	explicit detour(void* pFunction, void* pDetour)
		: _base(pFunction), _replace(pDetour) { }

	bool create(void* pFunction, void* pDetour) {
		_base = pFunction;

		if (_base == nullptr)
			return false;

		_replace = pDetour;

		if (_replace == nullptr)
			return false;

		auto status = MH_CreateHook(_base, _replace, &_original);

		if (status == MH_OK) {
			if (!this->replace())
				return false;
		}
		else {
			throw std::runtime_error("failed to create hook");
			return false;
		}

		return true;
	}

	bool replace() {
		if (_base == nullptr)
			return false;

		if (_is_hooked)
			return false;

		auto status = MH_EnableHook(_base);

		if (status == MH_OK)
			_is_hooked = true;
		else {
			throw std::runtime_error("failed to enable hook");
			return false;
		}

		return true;
	}

	template <typename arguments> arguments original() { return (arguments)_original; }
	inline bool is_hooked() { return _is_hooked; }

private:
	bool _is_hooked = false;
	void* _base = nullptr;
	void* _replace = nullptr;
	void* _original = nullptr;
};
namespace menu {
	inline int current_tab{ 0 };
	inline int title_height;
	inline int tabs_height = 45;
	inline bool reloadfonts;
	inline bool uninject;
	static const auto blue = '\x0C';
	static const auto red = '\x0C';
	inline auto color_p = red;
	inline const char* prefix_caps;
};

namespace reset {
	using fn = long(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
	long __stdcall hook(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pp);
}
namespace present {
	using fn = HRESULT(D3DAPI*) (IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
	HRESULT D3DAPI hook(IDirect3DDevice9* device, RECT* pSourceRect, RECT* pDestRect, HWND hDestWindowOverride, RGNDATA* pDirtyRegion);
}
namespace draw_print_text {
	using fn = void(__fastcall*)(void*, void*, const wchar_t*, int, int);
	void __fastcall hook(void* ecx, void* edx, const wchar_t* Text, int TextLength, int TextType);
};
namespace shithooks
{
	bool initialize();
	inline MinHook modifyEyePosition;
	inline MinHook isDepthOfFieldEnabled;
	inline MinHook tfd3d9;
}