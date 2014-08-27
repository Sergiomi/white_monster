#ifndef _GWINDOW_H_
#define _GWINDOW_H_

#include <algorithm>
#include <list>

class GRect
{
private:
	RECT _rect;

public:
	int left() { return _rect.left; }
	int right() { return _rect.right; }
};

class GWindow
{
protected:
	GWindow* _parent;
	std::list<GWindow*> _childs;

	HINSTANCE	_hInstance;
	

public:
	typedef std::list<GWindow*>::iterator child_iter;
	GWindow(GWindow* parent = NULL);
	virtual ~GWindow(void);

	GWindow* GetParent() const { return _parent; }
	
	virtual void RedrawWindow(bool childs = FALSE);
	//virtual void InvalidateRect();
};

#endif

