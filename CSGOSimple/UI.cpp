#include "ui.hpp"
#include "menu.hpp"
#include <deque>
#include <algorithm>
#include <vector>

bool ImGui::ToggleButton(const char* label, bool* v, const ImVec2& size_arg)
{

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	int flags = 0;
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;


	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = ImGui::GetColorU32((hovered && held || *v) ? ImGuiCol_ButtonActive : (hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button));
	ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
	if (pressed)
		*v = !*v;

	return pressed;
}

static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
	const char* const* items = (const char* const*)data;
	if (out_text)
		*out_text = items[idx];
	return true;
}

static auto vector_getter = [](void* vec, int idx, const char** out_text) {
	auto& vector = *static_cast<std::vector<std::string>*>(vec);
	if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
	*out_text = vector.at(idx).c_str();
	return true;
};

bool ImGui::Combo(const char* label, int* currIndex, std::vector<std::string>& values) {
	if (values.empty()) { return false; }
	return ImGui::Combo(label, currIndex, vector_getter,
		static_cast<void*>(&values), values.size());
}

bool ImGui::BeginGroupBox(const char* name, const ImVec2& size_arg)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow;

	window->DC.CursorPos.y += GImGui->FontSize / 2;
	const ImVec2 content_avail = ImGui::GetContentRegionAvail();
	ImVec2 size = ImFloor(size_arg);
	if (size.x <= 0.0f) {
		size.x = ImMax(content_avail.x, 4.0f) - fabsf(size.x); // Arbitrary minimum zero-ish child size of 4.0f (0.0f causing too much issues)
	}
	if (size.y <= 0.0f) {
		size.y = ImMax(content_avail.y, 4.0f) - fabsf(size.y);
	}

	ImGui::SetNextWindowSize(size);
	bool ret;
	ImGui::Begin(name, &ret, flags);
	//bool ret = ImGui::Begin(name, NULL, size, -1.0f, flags);

	window = ImGui::GetCurrentWindow();

	auto padding = ImGui::GetStyle().WindowPadding;

	auto text_size = ImGui::CalcTextSize(name, NULL, true);

	if (text_size.x > 1.0f) {
		window->DrawList->PushClipRectFullScreen();
		//window->DrawList->AddRectFilled(window->DC.CursorPos - ImVec2{ 4, 0 }, window->DC.CursorPos + (text_size + ImVec2{ 4, 0 }), GetColorU32(ImGuiCol_ChildWindowBg));
		//RenderTextClipped(pos, pos + text_size, name, NULL, NULL, GetColorU32(ImGuiCol_Text));
		window->DrawList->PopClipRect();
	}
	//if (!(window->Flags & ImGuiWindowFlags_ShowBorders))
	//	ImGui::GetCurrentWindow()->Flags &= ~ImGuiWindowFlags_ShowBorders;

	return ret;
}

void ImGui::EndGroupBox()
{
	ImGui::EndChild();
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DC.CursorPosPrevLine.y -= GImGui->FontSize / 2;
}

const char* const KeyNames[] = {
	"[-]",
	"[m1]",
	"[m2]",
	"[esc]",
	"[m3]",
	"[m4]",
	"[m5]",
	"[none]",
	"[back]",
	"[tab]",
	"[none]",
	"[none]",
	"[backspace]",
	"[enter]",
	"[none]",
	"[none]",
	"[shift]",
	"[ctrl]",
	"[alt]",
	"[pause break]",
	"[caps]",
	"[VK_KANA]",
	"[Unknown]",
	"[VK_JUNJA]",
	"[VK_FINAL]",
	"[VK_KANJI]",
	"[Unknown]",
	"[esc]",
	"[VK_CONVERT]",
	"[VK_NONCONVERT]",
	"[VK_ACCEPT]",
	"[VK_MODECHANGE]",
	"[space]",
	"[page up]",
	"[page down]",
	"[end]",
	"[home]",
	"[left]",
	"[up]",
	"[right]",
	"[down]",
	"[VK_SELECT]",
	"[VK_PRINT]",
	"[VK_EXECUTE]",
	"[Print Screen]",
	"[ins]",
	"[del]",
	"[VK_HELP]",
	"[0]",
	"[1]",
	"[2]",
	"[3]",
	"[4]",
	"[5]",
	"[6]",
	"[7]",
	"[8]",
	"[9]",
	"[unknown]",
	"[unknown]",
	"[unknown]",
	"[unknown]",
	"[unknown]",
	"[unknown]",
	"[unknown]",
	"[a]",
	"[b]",
	"[c]",
	"[d]",
	"[e]",
	"[f]",
	"[g]",
	"[h]",
	"[i]",
	"[j]",
	"[k]",
	"[l]",
	"[m]",
	"[n]",
	"[o]",
	"[p]",
	"[q]",
	"[r]",
	"[s]",
	"[t]",
	"[u]",
	"[v]",
	"[w]",
	"[x]",
	"[y]",
	"[z]",
	"[Left Windows]",
	"[Right Windows]",
	"[VK_APPS]",
	"[Unknown]",
	"[VK_SLEEP]",
	"[NUMPAD0]",
	"[NUMPAD1]",
	"[NUMPAD2]",
	"[NUMPAD3]",
	"[NUMPAD4]",
	"[NUMPAD5]",
	"[NUMPAD6]",
	"[NUMPAD7]",
	"[NUMPAD8]",
	"[NUMPAD9]",
	"[Multiply]",
	"[+]",
	"[Separator]",
	"[Subtract]",
	"[-]",
	"[/]",
	"[f1]",
	"[f2]",
	"[f3]",
	"[f4]",
	"[f5]",
	"[f6]",
	"[f7]",
	"[f8]",
	"[f9]",
	"[f10]",
	"[f11]",
	"[f12]",
	"[f13]",
	"[f14]",
	"[f15]",
	"[f16]",
	"[f17]",
	"[f18]",
	"[f19]",
	"[f20]",
	"[f21]",
	"[f22]",
	"[f23]",
	"[f24]",
	"[Unknown]",
	"[Unknown]",
	"[Unknown]",
	"[Unknown]",
	"[Unknown]",
	"[Unknown]",
	"[Unknown]",
	"[Unknown]",
	"[Num Lock]",
	"[Scroll lock]",
	"[VK_OEM_NEC_EQUAL]",
	"[VK_OEM_FJ_MASSHOU]",
	"[VK_OEM_FJ_TOUROKU]",
	"[VK_OEM_FJ_LOYA]",
	"[VK_OEM_FJ_ROYA]",
	"[Unknown]",
	"[Unknown]",
	"[Unknown]",
	"[Unknown]",
	"[Unknown]",
	"[Unknown]",
	"[Unknown]",
	"[Unknown]",
	"[Unknown]",
	"[l shift]",
	"[r shift]",
	"[l ctrl]",
	"[r ctrl]",
	"[l alt]",
	"[r alt]"
};

bool ImGui::Hotkey(const char* label, int* k, const ImVec2& size_arg)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	const ImGuiStyle& style = g.Style;

	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	ImVec2 size = CalcItemSize(ImVec2(50, size_arg.y), CalcItemWidth(), label_size.y + style.FramePadding.y * 2.0f);
	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? (style.ItemInnerSpacing.x + label_size.x) : 0.0f, 0.0f));

	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id))
		return false;

	//bool focus_requested = ImGui::FocusableItemRegister(window, g.ActiveId == id);

	const bool hovered = ImGui::ItemHoverable(frame_bb, id);

	if (hovered) {
		SetHoveredID(id);
		g.MouseCursor = ImGuiMouseCursor_TextInput;
	}

	const bool user_clicked = hovered && io.MouseClicked[0];

	if (user_clicked) {
		if (g.ActiveId != id) {
			// Start edition
			memset(io.MouseDown, 0, sizeof(io.MouseDown));
			memset(io.KeysDown, 0, sizeof(io.KeysDown));
			*k = 0;
		}
		SetActiveID(id, window);
		FocusWindow(window);
	}
	else if (io.MouseClicked[0]) {
		// Release focus when we click outside
		if (g.ActiveId == id)
			ClearActiveID();
	}

	bool value_changed = false;
	int key = *k;

	if (g.ActiveId == id) {
		for (auto i = 0; i < 5; i++) {
			if (io.MouseDown[i]) {
				switch (i) {
				case 0:
					key = VK_LBUTTON;
					break;
				case 1:
					key = VK_RBUTTON;
					break;
				case 2:
					key = VK_MBUTTON;
					break;
				case 3:
					key = VK_XBUTTON1;
					break;
				case 4:
					key = VK_XBUTTON2;
					break;
				}
				value_changed = true;
				ClearActiveID();
			}
		}
		if (!value_changed) {
			for (auto i = VK_BACK; i <= VK_RMENU; i++) {
				if (io.KeysDown[i]) {
					key = i;
					value_changed = true;
					ClearActiveID();
				}
			}
		}

		if (IsKeyPressedMap(ImGuiKey_Escape)) {
			*k = 0;
			ClearActiveID();
		}
		else {
			*k = key;
		}
	}

	// Render
	// Select which buffer we are going to display. When ImGuiInputTextFlags_NoLiveEdit is set 'buf' might still be the old value. We set buf to NULL to prevent accidental usage from now on.

	char buf_display[64] = "[-]";

//	RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

	if (*k != 0 && g.ActiveId != id) {
		strcpy(buf_display, KeyNames[*k]);
	}
	else if (g.ActiveId == id) {
		strcpy(buf_display, "...");
	}

	const ImRect clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + size.x, frame_bb.Min.y + size.y); // Not using frame_bb.Max because we have adjusted size
	ImVec2 render_pos = frame_bb.Min + style.FramePadding;
	RenderTextClipped(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding, buf_display, NULL, NULL);
	//draw_window->DrawList->AddText(g.Font, g.FontSize, render_pos, GetColorU32(ImGuiCol_Text), buf_display, NULL, 0.0f, &clip_rect);

	if (label_size.x > 0)
		RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

	return value_changed;
}

bool ImGui::ListBox(const char* label, int* current_item, std::string items[], int items_count, int height_items) {
	char **tmp;
	tmp = new char*[items_count];//(char**)malloc(sizeof(char*) * items_count);
	for (int i = 0; i < items_count; i++) {
		//tmp[i] = new char[items[i].size()];//(char*)malloc(sizeof(char*));
		tmp[i] = const_cast<char*>(items[i].c_str());
	}

	const bool value_changed = ImGui::ListBox(label, current_item, Items_ArrayGetter, static_cast<void*>(tmp), items_count, height_items);
	return value_changed;
}

bool ImGui::ListBox(const char* label, int* current_item, std::function<const char*(int)> lambda, int items_count, int height_in_items)
{
	return ImGui::ListBox(label, current_item, [](void* data, int idx, const char** out_text)
	{
		*out_text = (*reinterpret_cast<std::function<const char*(int)>*>(data))(idx);
		return true;
	}, &lambda, items_count, height_in_items);
}

bool ImGui::Combo(const char* label, int* current_item, std::function<const char*(int)> lambda, int items_count, int height_in_items)
{
	return ImGui::Combo(label, current_item, [](void* data, int idx, const char** out_text)
	{
		*out_text = (*reinterpret_cast<std::function<const char*(int)>*>(data))(idx);
		return true;
	}, &lambda, items_count, height_in_items);
}
