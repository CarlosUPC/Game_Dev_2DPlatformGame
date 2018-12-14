#ifndef __UIELEMENT_H__
#define __UIELEMENT_H__

#include "j1App.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1Gui.h"
#include "p2Log.h"

class j1Module;

enum UI_type {
	CHECKBOX,
	INPUT_TEXT,
	SCROLLBAR,
	BUTTON,
	LABEL,
	IMAGE,
	WINDOW,
	UNKNOW
};

enum Mouse_Event {
	HOVER,
	CLICKED_DOWN,
	CLICKED_REPEAT,
	CLICKED_UP,

	NONE
};

class UIElement {

public:

	UIElement() : type(UNKNOW) {}
	UIElement(UI_type type, const int &pos_x, const int &pos_y, UIElement* parent, bool interactable = true, bool draggable = false, const int &width = 0, const int &height = 0, bool drawable = true);
	~UIElement() {}

	void Draw();

	virtual void InnerDraw() {}
	virtual void Update();
	virtual void CleanUp() {}

	virtual void Scroll(char dir, float percentage) {}

	void SetPos(const int &x, const int &y);

	UI_type GetType()const { return type; }
	int GetPriority() const;
	iPoint GetGlobalPosition() const;
	iPoint GetLocalPosition() const;

	void DebugDraw();

	void AddListener(j1Module* module);
	p2List_item<j1Module*>* GetFirstListener();
	p2List_item<j1Module*>* GetLastListener();

public:
	SDL_Rect position;

	bool interactable = true;
	bool draggable = true;
	bool drawable = true;
	
	bool to_delete = false;
	iPoint draw_offset = { 0,0 };

	UIElement* parent = nullptr;

	p2List<UIElement*> childs;

	Mouse_Event current_state = NONE;
	iPoint last_mouse;
	
private:
	UI_type type = UNKNOW;
	
	//SDL_Rect viewport = { 0,0,0,0 };
	
	int priority = 0;

	p2List<j1Module*> listeners;
};

#endif
