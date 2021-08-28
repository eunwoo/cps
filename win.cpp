// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <string>
#include <tchar.h>
#include <strsafe.h>
#include <thread>
#include <chrono>
#include <vector>
#include <iostream>
#include "cps.h"
#pragma comment( lib, "user32.lib") 
#pragma comment( lib, "gdi32.lib")
#pragma comment( lib, "winmm.lib")

using namespace std;

#define NUMHOOKS 7 
#define  IDM_MOUSE 0

// Global variables 
CRITICAL_SECTION CriticalSection;
DrawingTool dt;


vector<ClickData *> click_data;

typedef struct _MYHOOKDATA 
{ 
    int nType; 
    HOOKPROC hkprc; 
    HHOOK hhook; 
} MYHOOKDATA; 
 
MYHOOKDATA myhookdata[NUMHOOKS]; 


// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("CPS ÃøÁ¤±â");

HINSTANCE hInst;
HWND hWnd;
HWND gh_hwndMain;
// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI MouseProc(int, WPARAM, LPARAM);
void LookUpTheMessage(PMSG msg, LPTSTR str)
{
   str[0] = 'a';
   str[1] = 0;
}
HWND hWndEdit;
void  UpdateEditWindow()
{
   return;
#ifdef UNICODE
   wstring str;
   for (vector<ClickData*>::iterator itr = click_data.begin();
       itr != click_data.end(); ++itr) {
       str += _T("\r\n") + to_wstring((static_cast<ClickData*>(*itr))->time)
           + _T(", ") + to_wstring((static_cast<ClickData*>(*itr))->count)
           + _T(", ") + to_wstring((static_cast<ClickData*>(*itr))->evt);
   }
   SetWindowText(hWndEdit, str.c_str());
#else
#endif

}
int quit_request;
int click, click_old;
DWORD t_now, t_old;
double cps;
int CriticalSectionInit = 0;
void  thread_function1(int num)
{
   RECT r;
   while(CriticalSectionInit == 0);
   while(1) {

      t_now = timeGetTime();
      cps = (click - click_old)/((t_now - t_old)*0.01);
      click_old = click;
      t_old = t_now;

      EnterCriticalSection(&CriticalSection);
      vector<ClickData*> new_click_data;
      int delete_cnt = 0;
      vector<ClickData *>::iterator itr, itr_del;
      ClickData *cd;
      
      for(itr = click_data.begin(), itr_del = itr;
         itr != click_data.end(); ++itr) {
         cd = static_cast<ClickData*>(*itr);
         if(t_now - cd->time > 2000) {
            delete_cnt++;
            itr_del = itr;
         }
         else if(t_now - cd->time > 1000) {
            // delete_cnt++;
            if(cd->evt == '+' && click > 0) {
               click--;
               new_click_data.push_back(new ClickData(t_now, click, '-'));
               cd->evt = '-';
            }
         }
         else {
            break;
         }
      }
      if(delete_cnt > 0) {
         if(itr_del > click_data.begin()) {
            click_data.erase(click_data.begin(), itr_del - 1);
         }
         // click_data.erase(click_data.begin(), itr);
      }
      click_data.insert(click_data.end(), new_click_data.begin(), new_click_data.end());
      LeaveCriticalSection(&CriticalSection);

      UpdateEditWindow();

      GetClientRect(hWnd, &r);
      InvalidateRect(hWnd, &r, true);
      this_thread::sleep_for(chrono::milliseconds(10));
      if(quit_request == 1) {
         break;
      }
   }
}
std::thread _t1(thread_function1, 0);
// std::thread _t2(thread_function2, 0);

int WINAPI WinMain(
   _In_ HINSTANCE hInstance,
   _In_opt_ HINSTANCE hPrevInstance,
   _In_ LPSTR     lpCmdLine,
   _In_ int       nCmdShow
)
{
   WNDCLASSEX wcex;
   GdiplusStartupInput gdiplusStartupInput;
   ULONG_PTR           gdiplusToken;

   // Initialize GDI+.
   GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
   if (!InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x00000400)) {
      return 1;
   }
   CriticalSectionInit = 1;

   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.style          = CS_HREDRAW | CS_VREDRAW;
   wcex.lpfnWndProc    = WndProc;
   wcex.cbClsExtra     = 0;
   wcex.cbWndExtra     = 0;
   wcex.hInstance      = hInstance;
   wcex.hIcon          = LoadIcon(wcex.hInstance, IDI_APPLICATION);
   wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
   wcex.lpszMenuName   = NULL;
   wcex.lpszClassName  = szWindowClass;
   wcex.hIconSm        = LoadIcon(wcex.hInstance, IDI_APPLICATION);

   if (!RegisterClassEx(&wcex))
   {
      MessageBox(NULL,
         _T("Call to RegisterClassEx failed!"),
         _T("Windows Desktop Guided Tour"),
         NULL);

      return 1;
   }

   // Store instance handle in our global variable
   hInst = hInstance;

   // The parameters to CreateWindowEx explained:
   // WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
   // szWindowClass: the name of the application
   // szTitle: the text that appears in the title bar
   // WS_OVERLAPPEDWINDOW: the type of window to create
   // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
   // 500, 100: initial size (width, length)
   // NULL: the parent of this window
   // NULL: this application does not have a menu bar
   // hInstance: the first parameter from WinMain
   // NULL: not used in this application
   hWnd = CreateWindowEx(
      WS_EX_OVERLAPPEDWINDOW,
      szWindowClass,
      szTitle,
      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
      CW_USEDEFAULT, CW_USEDEFAULT,
      300, 300,
      NULL,
      NULL,
      hInstance,
      NULL
   );
   gh_hwndMain = hWnd;

   // hWndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), TEXT("test"),
   //                             WS_CHILD | WS_VISIBLE | WS_VSCROLL 
   //                             | ES_MULTILINE | ES_AUTOVSCROLL, 
   //                             10, 200, 300, 200, hWnd, NULL, NULL, NULL);
   if (!hWnd)
   {
      MessageBox(NULL,
         _T("Call to CreateWindow failed!"),
         _T("Windows Desktop Guided Tour"),
         NULL);

      return 1;
   }

   // The parameters to ShowWindow explained:
   // hWnd: the value returned from CreateWindow
   // nCmdShow: the fourth parameter from WinMain
   ShowWindow(hWnd,
      nCmdShow);
   UpdateWindow(hWnd);

   // Main message loop:
   MSG msg;
   while (GetMessage(&msg, NULL, 0, 0))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
   DeleteCriticalSection(&CriticalSection);

   return (int) msg.wParam;
}

HHOOK MouseHook;

TCHAR szBufMouseProc[128]; 
size_t cch; 
char large_buf[1000];

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) 
{ 
    TCHAR szMsg[16]; 
    HDC hdc; 
    static int c = 0; 
    HRESULT hResult;
 
    if (nCode < 0)  // do not process the message 
        return CallNextHookEx(myhookdata[IDM_MOUSE].hhook, nCode, 
            wParam, lParam); 
 
    // Call an application-defined function that converts a message 
    // constant to a string and copies it to a buffer. 
 
    LookUpTheMessage((PMSG) lParam, szMsg); 
 
    if(wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN) {
       EnterCriticalSection(&CriticalSection);
       c++;
       click++;
       t_now = timeGetTime();
       if(click == 1) {
          if(click_data.size() == 0) {
             click_data.push_back(new ClickData(t_now - 1000, 0, '-'));
          }
          else {
             click_data.push_back(new ClickData(click_data.at(click_data.size()-1)->time, 0, '-'));
          }

       }
       click_data.push_back(new ClickData(t_now, click, '+'));
      LeaveCriticalSection(&CriticalSection);
      UpdateEditWindow();
    }
   //  hdc = GetDC(gh_hwndMain);
    hResult = StringCchPrintf(szBufMouseProc, 128/sizeof(TCHAR), 
        _T("MOUSE - nCode: %d, msg: %s, x: %d, y: %d, %d times   "), 
        nCode, szMsg, LOWORD(lParam), HIWORD(lParam), c);
    if (FAILED(hResult))
    {
    // TODO: write error handler
    }
    hResult = StringCchLength(szBufMouseProc, 128/sizeof(TCHAR), &cch);
    if (FAILED(hResult))
    {
    // TODO: write error handler
    }
   //  TextOut(hdc, 2, 95, szBuf, cch); 
   //  ReleaseDC(gh_hwndMain, hdc); 
    
    return CallNextHookEx(myhookdata[IDM_MOUSE].hhook, nCode, wParam, lParam); 
} 


//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   PAINTSTRUCT ps;
   HDC hdc;
   TCHAR greeting[] = _T("Hello, Windows desktop!");
   HDC hdcBuffer;
   RECT rect;
   int width, height;
   TCHAR szBuf[128]; 
   HFONT hFont, hTmp;

   switch (message)
   {
   case WM_CREATE:
      // myhookdata[IDM_MOUSE].nType = WH_MOUSE; 
      myhookdata[IDM_MOUSE].nType = WH_MOUSE_LL; 
      myhookdata[IDM_MOUSE].hkprc = MouseProc;
      // myhookdata[IDM_MOUSE].hhook = SetWindowsHookEx( 
      //       myhookdata[IDM_MOUSE].nType, 
      //       myhookdata[IDM_MOUSE].hkprc, 
      //       (HINSTANCE) NULL, GetCurrentThreadId()); 
      myhookdata[IDM_MOUSE].hhook = SetWindowsHookEx( 
            myhookdata[IDM_MOUSE].nType, 
            myhookdata[IDM_MOUSE].hkprc, 
            (HINSTANCE) hInst, 0);       
      return 0;
   case WM_ERASEBKGND:
      return 0;
   case WM_PAINT:
      {
         hdc = BeginPaint(hWnd, &ps);
         hdcBuffer = CreateCompatibleDC(hdc);
         GetClientRect(hWnd, &rect);
         width = rect.right;
         height = rect.bottom;
         HBITMAP backbuffer = CreateCompatibleBitmap( hdc, width, height);
         int savedDC = SaveDC(hdcBuffer);
         SelectObject(hdcBuffer, backbuffer);

         double color_blend_r, color_blend_g;
         color_blend_g = (click-7)/7.;
         color_blend_r = -color_blend_g;
         if(color_blend_g > 1.0) color_blend_g = 1.0;
         else if(color_blend_g < 0.0) color_blend_g = 0.0;
         color_blend_g = 1 - color_blend_g;
         if(color_blend_r > 1.0) color_blend_r = 1.0;
         else if(color_blend_r < 0.0) color_blend_r = 0.0;
         color_blend_r = 1 - color_blend_r;

         COLORREF rgb = RGB(255*color_blend_r, 255*color_blend_g, 0);
         HBRUSH hBrush = CreateSolidBrush(rgb);
         FillRect(hdcBuffer, &rect, hBrush);
         DeleteObject(hBrush);
         // Here your application is laid out.
         // For this introduction, we just print out "Hello, Windows desktop!"
         // in the top left corner.
         // TextOut(hdcBuffer,
         //    5, 5,
         //    greeting, _tcslen(greeting));
         StringCchPrintf(szBuf, 128/sizeof(TCHAR), 
             _T("%d"), click);
         hFont = CreateFont(80, 0, 0, // Height, Width, Escapment, 
            1, FW_BOLD, 0, true, //Orientation, Weight, bItalic, bUnderline
            0, 0, 0, 0, 2, 0, _T("SYSTEM_FIXED_FONT"));
         hTmp = (HFONT)SelectObject(hdcBuffer, hFont);
         // TextOut(hdcBuffer,
         //    5, 35,
         //    szBuf, _tcslen(szBuf));
         SetBkColor(hdcBuffer, rgb);
         RECT rectText = rect;
         rectText.top += rect.bottom/2;
         DrawText(hdcBuffer, szBuf, _tcslen(szBuf), &rectText, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

         // // debug print -s
         // hFont = CreateFont(16, 0, 0, // Height, Width, Escapment, 
         //    1, FW_THIN, 0, 0, //Orientation, Weight, bItalic, bUnderline
         //    0, 0, 0, 0, 2, 0, _T("SYSTEM_FIXED_FONT"));
         // SelectObject(hdcBuffer, hFont);
         // if(click_data.size()> 1) {
         //    StringCchPrintf(szBuf, 128/sizeof(TCHAR), 
         //       _T("click_data.size() = %d, %d, %d"), click_data.size(), click_data.at(0)->count, click_data.at(1)->count);
         // }
         // else {
         //    StringCchPrintf(szBuf, 128/sizeof(TCHAR), 
         //       _T("click_data.size() = %d"), click_data.size());
         // }
         // rectText.left = 10;
         // DrawText(hdcBuffer, szBuf, _tcslen(szBuf), &rectText, DT_LEFT | DT_TOP);
         // // debug print -e

         // TextOut(hdcBuffer, 2, 95, szBufMouseProc, cch); 
         // End application-specific layout section.
         {
            RECT rect;
            GetClientRect(hWnd, &rect);
            dt.SetHDC(hdcBuffer);
            dt.SetRect(rect);
            dt.DrawStatistic(click_data);
         }

         BitBlt(hdc, 0, 0, width, height, hdcBuffer, 0, 0, SRCCOPY);

         RestoreDC(hdcBuffer, savedDC);
         DeleteObject(backbuffer);
         DeleteObject(SelectObject(hdcBuffer, hTmp)); // Delete hFont
         DeleteDC(hdcBuffer);
         EndPaint(hWnd, &ps);
      }

      break;
   case WM_DESTROY:
      quit_request = 1;
      _t1.join();
      // _t2.join();
      PostQuitMessage(0);
      break;
   default:
      return DefWindowProc(hWnd, message, wParam, lParam);
      break;
   }

   return 0;
}