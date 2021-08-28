#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <thread>
#include <vector>

#include <iostream>
#include <gdiplus.h>
#include "cps.h"

using namespace std;
using namespace Gdiplus;
DrawingTool::DrawingTool()
{
	virtual_width = 300;
	virtual_height = 280;
	screen_org.X = 10;
}

DrawingTool::DrawingTool(HDC hdc, RECT &rect) : hdc(hdc)
{
	virtual_width = 300;
	virtual_height = 300;
	screen_org.X = 10;
	screen_org.Y = GetHeight(rect) - 20;
	scale_x = (GetWidth(rect) - 20)  / virtual_width;
	scale_y = -(GetHeight(rect) - 20) / virtual_height;
	grid_scr.X = width/9 * scale_x;
	grid_scr.Y = height/9 * scale_y;
}
void DrawingTool::SetRect(RECT &rect)
{
	screen_org.Y = GetHeight(rect) - 10;
	scale_x = (GetWidth(rect) - 20)  / virtual_width;
	scale_y = -(GetHeight(rect) - 20) / virtual_height;
	grid_scr.X = width/9 * scale_x;
	grid_scr.Y = height/9 * scale_y;	
	this->rect.left = rect.left;
	this->rect.right = rect.right;
	this->rect.top = rect.top;
	this->rect.bottom = rect.bottom;
}
void DrawingTool::SetHDC(HDC hdc)
{
	this->hdc = hdc;
}
int DrawingTool::GetWidth(RECT &rect)
{
	return rect.right - rect.left;
}
int DrawingTool::GetHeight(RECT &rect)
{
	return rect.bottom - rect.top;
}
void DrawingTool::Draw()
{
	TCHAR greeting[] = _T("Hello, Windows desktop1!");

	TextOut(hdc,
         15, 15,
         greeting, _tcslen(greeting));
}
Point DrawingTool::VirtualToScreen(Point_d a)
{
	Point p;
	p.X = static_cast<int>(a.x * scale_x) + screen_org.X;
	p.Y = static_cast<int>(a.y * scale_y) + screen_org.Y;
	return p;
}
void DrawingTool::DrawLine(Point_d a, Point_d b)
{
	Graphics graphics(hdc);
	Point p1 = VirtualToScreen(a);
	Point p2 = VirtualToScreen(b);
	Point p1clip = VirtualToScreen(Point_d(0, 0));
	Point p2clip = VirtualToScreen(Point_d(width, height));
	Region clipRegion(Rect(p1clip.X, p2clip.Y, p2clip.X - p1clip.X, p1clip.Y - p2clip.Y));
	graphics.SetClip(&clipRegion, CombineModeReplace);

	Pen      pen(Color(255, 0, 0, 255));
	graphics.DrawLine(&pen, p1.X, p1.Y, p2.X, p2.Y);
}
void DrawingTool::DrawLine(Point_d a, Point_d b, Color c)
{
	Graphics graphics(hdc);
	Point p1 = VirtualToScreen(a);
	Point p2 = VirtualToScreen(b);

   Pen      pen(c);
   graphics.DrawLine(&pen, p1.X, p1.Y, p2.X, p2.Y);
}

void DrawingTool::DrawStatistic(vector<ClickData *> &cd)
{
	//Region clipRegion(Rect(rect));

	Color c_grid(100, 200, 200, 255);
	Color c_outline(255, 255, 0, 0);
	for(double w = 0; w <= width; w += width/9) {
		if(w == 0 || w > width/9*8) {
			DrawLine(Point_d(w,height/2), Point_d(w,height), c_outline);
		}
		else {
			DrawLine(Point_d(w,height/2), Point_d(w,height), c_grid);
		}
	}
	for(double h = height/2; h <= height; h += height/18) {
		if(h == height/2 || h > height/2 + height/18*8) {
			DrawLine(Point_d(0,h), Point_d(width,h), c_outline);
		}
		else {
			DrawLine(Point_d(0,h), Point_d(width,h), c_grid);
		}
	}
	DWORD t_now = timeGetTime();
	vector<ClickData*>::iterator itr2;
	ClickData *cd_i, *cd_i2, *cd_i_prev = nullptr;
#if 0	// staircase
	for(vector<ClickData*>::iterator itr = cd.begin();
		itr != cd.end(); itr++) {
		ClickData *cd_i = reinterpret_cast<ClickData*>(*itr), *cd_i2;
		if(cd_i->evt == '+') {
			double x1 = (cd_i->time - (t_now - 1000))/1000.*width, x2;
			for(itr2 = itr + 1; itr2 != cd.end(); itr2++) {
				cd_i2 = reinterpret_cast<ClickData*>(*itr2);
				if(cd_i2->evt == '+') {
					break;
				}
			}
			if(itr2 != cd.end()) {
				x2 = (cd_i2->time - (t_now - 1000))/1000.*width;
			}
			else {
				x2 = x1;
			}
			double y, y1;
			y = cd_i->count/20.*height/2;
			if(cd_i_prev != nullptr) {
				y1 = cd_i_prev->count/20.*height/2;
			}
			else {
				y1 = 0;
			}
			DrawLine(Point_d(x1, height/2 + y1),  Point_d(x1, height/2 + y));
			DrawLine(Point_d(x1, height/2 + y),  Point_d(x2, height/2 + y));
			cd_i_prev = cd_i;
		}
	}
#endif
// point-to-point
	for(vector<ClickData*>::iterator itr = cd.begin();
		itr != cd.end(); itr++) {
		ClickData *cd_i = reinterpret_cast<ClickData*>(*itr), *cd_i2;
		double x1 = ((int)cd_i->time - ((int)t_now - 1000))/1000.*width, x2;
		double y1, y2;
		y1 = cd_i->count/20.*height/2;
		if(itr+1 != cd.end()) {
			cd_i2 = reinterpret_cast<ClickData*>(*(itr+1));
			x2 = ((int)cd_i2->time - ((int)t_now - 1000))/1000.*width;
			y2 = cd_i2->count/20.*height/2;
		}
		else {
			x2 = x1;
			y2 = 0;
		}
		// DrawLine(Point_d(x1, height/2 + y1),  Point_d(x1, height/2 + y));
		// DrawLine(Point_d(x1, height/2 + y),  Point_d(x2, height/2 + y));
		// if(itr+1 == cd.end()) {
		// 	if(cd_i->evt == '+') {
		// 		DrawLine(Point_d(x1, height/2 + y1),  Point_d(x1, height/2 + y));
		// 		DrawLine(Point_d(x1, height/2 + y),  Point_d(width, height/2 + y));
		// 	}
		// 	else {
		// 		DrawLine(Point_d(0, height/2 + y1),  Point_d(x1, height/2 + y1));
		// 		DrawLine(Point_d(x1, height/2 + y1),  Point_d(x1, height/2 + y));
		// 	}
		// }
		// else {
			DrawLine(Point_d(x1, height/2 + y1),  Point_d(x2, height/2 + y2));
			test_x1 = y1;
			test_x2 = y2;
		// }
		cd_i_prev = cd_i;
	}
	// DrawLine(Point_d(-10, 10), Point_d(20, 10));
	// Point a = VirtualToScreen(Point_d(width, 0));
	// DrawNumber(2, 4, a.X);
	// int w = GetWidth(rect);
	// DrawNumber(2, 6, w);
	// DrawNumber(2, 7, virtual_height);
	// DrawNumber(2, 9, scale_x);
	
}
void DrawingTool::DrawNumber(int x, int y, int num)
{
	WCHAR str[10];
#ifdef UNICODE
	wchar_t cstr[10];

	swprintf_s(cstr, 10, _T("%d"), num);
	Point_d disp;
	disp.x = x * width / 9;
	disp.y = y * height / 9;
	Point disp_scr = VirtualToScreen(disp);
	TextOut(hdc,
		disp_scr.X + grid_scr.X / 2 - 6, disp_scr.Y + grid_scr.Y / 2 - 6,
		cstr, _tcslen(cstr));
#else
	char cstr[10];

	sprintf_s(cstr, "%d", num);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, cstr, strlen(cstr), str, static_cast<size_t>(10));
	str[strlen(cstr)] = 0;
	Point_d disp;
	disp.x = x * width / 9;
	disp.y = y * height / 9;
	Point disp_scr = VirtualToScreen(disp);
	TextOut(hdc,
		disp_scr.X + grid_scr.X / 2 - 6, disp_scr.Y + grid_scr.Y / 2 - 6,
		cstr, _tcslen(cstr));
#endif
}
void DrawingTool::DrawNumber(int x, int y, double num)
{
	WCHAR str[10];
#ifdef UNICODE
	wchar_t cstr[10];

	swprintf_s(cstr, 10, _T("%d"), num);
	Point_d disp;
	disp.x = x * width / 9;
	disp.y = y * height / 9;
	Point disp_scr = VirtualToScreen(disp);
	TextOut(hdc,
		disp_scr.X + grid_scr.X / 2 - 6, disp_scr.Y + grid_scr.Y / 2 - 6,
		cstr, _tcslen(cstr));
#else
	char cstr[10];

	sprintf_s(cstr, "%.3f", num);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, cstr, strlen(cstr), str, static_cast<size_t>(10));
	str[strlen(cstr)] = 0;
	Point_d disp;
	disp.x = x * width / 9;
	disp.y = y * height / 9;
	Point disp_scr = VirtualToScreen(disp);
	TextOut(hdc,
		disp_scr.X + grid_scr.X / 2 - 6, disp_scr.Y + grid_scr.Y / 2 - 6,
		cstr, _tcslen(cstr));
#endif
}