#include "StdAfx.h"
#include "GWindow.h"


GWindow::GWindow(GWindow* parent) : _parent(parent)
{

}


GWindow::~GWindow(void)
{
	for (child_iter i = _childs.begin(); i != _childs.end(); i++)
		delete *i;

	_childs.clear();
}

void GWindow::RedrawWindow(bool childs)
{
	if (childs) {
		for (child_iter i = _childs.begin(); i != _childs.end(); i++)
			(*i)->RedrawWindow(childs);
	}
}
