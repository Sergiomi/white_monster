#pragma once

#include "resource.h"

#define MARGIN				5
#define LISTBOX_WIDTH		300
#define LISTBOX_HEIGHT		(LAYERSWND_HEIGHT - BUTTON_HEIGHT - MARGIN)
#define LISTBOX_X			(MARGIN)
#define LISTBOX_Y			(MARGIN)
#define BUTTON_WIDTH		80
#define BUTTON_HEIGHT		20
#define LAYERSWND_WIDTH		200
#define LAYERSWND_HEIGHT	200
#define LAYERSWND_X			(LISTBOX_X + LISTBOX_WIDTH + MARGIN)
#define LAYERSWND_Y			(LISTBOX_Y)
#define INFOWND_WIDTH		200
#define INFOWND_HEIGHT		(LAYERSWND_HEIGHT)
#define INFOWND_X			(LAYERSWND_X + LAYERSWND_WIDTH + MARGIN)
#define INFOWND_Y			(LISTBOX_Y)
#define GRAPHWND_WIDTH		(LISTBOX_WIDTH + LAYERSWND_WIDTH + INFOWND_WIDTH + 2*MARGIN)
#define GRAPHWND_HEIGHT		150
#define GRAPHWND_X			(LISTBOX_X)
#define GRAPHWND_Y			(LISTBOX_Y + LAYERSWND_HEIGHT + MARGIN)
#define MAINWND_WIDTH		(GRAPHWND_WIDTH + 2*MARGIN)
#define MAINWND_HEIGHT		(LAYERSWND_HEIGHT + GRAPHWND_HEIGHT + 3*MARGIN)

#define WINDOW_FONT			_T("MS Shell Dlg")

void UpdateGraph();
void UpdateLayersView();
void UpdateInfo();
void UpdateLayersList();
void UpdateTemperatureLog();
void UpdateCurrentTime();
void UpdateEditLayerData();
void UpdateCurrentLayer();
void UpdateAll();

