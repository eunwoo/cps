#pragma once

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <vector>
using namespace std;
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
class ClickData
{
public:
   DWORD time;
   int count;
   char evt;
   ClickData(DWORD t, int c, char e) : time(t), count(c), evt(e) {}
} ;
class Point_d {
public:
    double x, y;
    Point_d() {}
    Point_d(double x, double y) : x(x), y(y) {}
};
#define N   9
class DrawingTool {
public:
    HDC hdc;
	double scale_x, scale_y, org_x, org_y, virtual_width, virtual_height;
    Point_d virtual_org;
    Point screen_org, grid_scr;
	double width = 300;
	double height = 280;
    RECT rect;
    double test_x1, test_x2;

public:
    DrawingTool();
    DrawingTool(HDC hdc, RECT &rect);
    void SetHDC(HDC hdc);
    void SetRect(RECT &rect);
    int GetWidth(RECT &rect);
    int GetHeight(RECT &rect);
    void Draw();
    void DrawLine(Point_d a, Point_d b);
    void DrawLine(Point_d a, Point_d b, Color c);
    Point VirtualToScreen(Point_d a);
    void DrawNumber(int x, int y, int num);
    void DrawNumber(int x, int y, double num);
    void DrawStatistic(vector<ClickData *> &cd);
};

bool solveSuduko(int grid[N][N], int row, int col);
