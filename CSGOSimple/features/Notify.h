#pragma once
#include <string>
#include "../valve_sdk/sdk.hpp"
namespace notification
{
	struct notify_t
	{
		std::string text;
		float time;
		Color _color;

		notify_t(std::string _text, Color __color)
		{
			text = _text;
			_color = __color;
			time = g_GlobalVars->curtime;
		}
	};

	inline std::vector<notify_t> notify_list;

	inline int offset_message = 0;

	void run(std::string text, std::string chat_text, bool console, bool chat, bool screen);
	void log_in_console(std::string text, Color _color);
	void log_on_scene(std::string text, Color _color);
	void draw();
}