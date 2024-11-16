#define MAX_DRAG_OBJECTS 20

namespace cMouse {



    //called when menu on/whatever
    void Update();

    bool LeftClick(int x, int y, int w, int h);
    bool OneLeftClick(int x, int y, int w, int h);
    bool RightClick(int x, int y, int w, int h);
    bool OneRightClick(int x, int y, int w, int h);
    bool IsOver(int x, int y, int w, int h);

    void Drag(bool& bDrag, bool bCheck, bool bDragCheck, int& x, int& y, int& xdif, int& ydif);


    inline bool bReturn{};

    inline bool bDragged[MAX_DRAG_OBJECTS]{};
    inline int iDiffX[MAX_DRAG_OBJECTS]{};
    inline int iDiffY[MAX_DRAG_OBJECTS]{};
    inline int  mouse_x{}, mouse_y{};

    inline bool mouse1pressed{};
    inline bool mouse1released{};
    inline bool mouse2pressed{};
    inline bool mouse2released{};
    inline void GetMousePosition(int& posx, int& posy)
    {
        posx = mouse_x;
        posy = mouse_y;
    }

    inline bool HasMouseOneJustBeenReleased()
    {
        return mouse1released;
    }

    inline bool IsMouseTwoBeingHeld()
    {
        return mouse2pressed;
    }



};