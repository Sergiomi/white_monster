void DrawGrid(HDC hdc, int x, int y, int width, int height, int stepX = 100, int stepY = 50);
void DrawTemperature(HDC hdc, int x, int y, int width, int height, double timeS, double timeE);
void DrawCurrentInfo(HDC hdc, int x, int y, int width, int height);
void DrawGraph(HDC hdc, int x, int y, int width, int height, int type, double timeS, double timeE);
void DrawCursor(HDC hdc, int x, int y, int height);