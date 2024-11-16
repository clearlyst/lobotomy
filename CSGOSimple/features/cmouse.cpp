#include "cmouse.h"
#include "../valve_sdk/sdk.hpp"

void cMouse::Drag(bool& bDrag, bool bCheck, bool bDragCheck, int& x, int& y, int& xdif, int& ydif)
{
	if (bCheck)
	{
		if (bDragCheck || (mouse1pressed && bDrag))
		{
			if (!bDrag)
				bDrag = true;

			if (xdif == -1 || ydif == -1)
			{
				xdif = mouse_x - x;
				ydif = mouse_y - y;
			}

			x += mouse_x - (xdif + x);
			y += mouse_y - (ydif + y);
		}
		else
		{
			if (bDrag)
				bDrag = false;

			xdif = -1;
			ydif = -1;
		}
	}

}

//not mine

bool cMouse::IsOver(int x, int y, int w, int h)
{
	return (mouse_x > x && w + x > mouse_x && mouse_y > y && h + y > mouse_y);
}
bool cMouse::LeftClick(int x, int y, int w, int h)
{
	return (mouse1pressed && IsOver(x, y, w, h));
}

bool cMouse::RightClick(int x, int y, int w, int h)
{
	return (mouse2pressed && IsOver(x, y, w, h));
}

bool cMouse::OneLeftClick(int x, int y, int w, int h)
{
	return (mouse1released && IsOver(x, y, w, h));
}

bool cMouse::OneRightClick(int x, int y, int w, int h)
{
	return (mouse2released && IsOver(x, y, w, h));
}



//kolonote:
//now the credits to this go to someone that posted it on GD in 2004 i think i forgot his name
void cMouse::Update()
{
	//static ValveSDK::ConVar *pRawInput = g_Valve.pConVar->FindVar(/*m_rawinput*/XorStr<0x63,11,0x160FA730>("\x0E\x3B\x17\x07\x10\x01\x07\x1A\x1E\x18"+0x160FA730).s);	

	////IMPORTANT FIX DONT REMOVE ME
	//if(pRawInput->GetInt())
	//	pRawInput->SetValue(0);

	int width, height;
	g_EngineClient->GetScreenSize(width, height);

	int winX = width * 0.5;
	int winY = height * 0.5;

	//3 winapi calls is all we need
	static auto csgo = FindWindowA(0, "Counter-Strike: Global Offensive");

	tagPOINT tp;
	GetCursorPos(&tp);

	LPPOINT pPoint = &tp;
	ScreenToClient(csgo, pPoint);

	mouse_x = pPoint->x;
	mouse_y = pPoint->y;

	//the mouse coordinates depending on the screen size
	if (cMouse::mouse_x > width)
		cMouse::mouse_x = width;

	if (mouse_x < 0)
		mouse_x = 0;

	if (mouse_y > height)
		mouse_y = height;

	if (mouse_y < 0)
		mouse_y = 0;

	//HANDLING:

	//handle mouse1
	if (GetAsyncKeyState(VK_LBUTTON))
		mouse1pressed = true;
	else if (!GetAsyncKeyState(VK_LBUTTON))
	{
		if (mouse1pressed)
			mouse1released = true;
		else
			mouse1released = false;

		mouse1pressed = false;
	}

	//handle mouse2
	if (GetAsyncKeyState(VK_RBUTTON))
		mouse2pressed = true;
	else if (!GetAsyncKeyState(VK_RBUTTON))
	{
		if (mouse2pressed)
			mouse2released = true;
		else
			mouse2released = false;

		mouse2pressed = false;
	}
}