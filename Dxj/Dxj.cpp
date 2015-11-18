#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include "SciTE.h"
#include "Scintilla.h"
#define DXJCLASS "DxjWindow"

class DxjWindow
{
private:
	char* szIcon;
	char* szMenu;
	char* szCaption;
	HINSTANCE hInstance;
	HWND hwndDxj,hwndEditor;
public:
	DxjWindow(HINSTANCE);
	~DxjWindow();
	static LRESULT PASCAL WndStaticProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);
	LRESULT WndProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);
	int EventLoop();
//	char* GetIcon();
//	void SetIcon(const char* szNewIcon);
//	char* GetMenu();
//	void SetMenu(const char* szNewMenu);
//	char* GetCaption();
//	void SetCaption(const char* szNewCaption);
};
FILE *fLog;
DxjWindow::DxjWindow(HINSTANCE hInstance_)
{
	hInstance=hInstance_;
	szCaption = "DXJCAPTION";
	szIcon = "DXJICON";
	szMenu = "DXJMENU";
	WNDCLASS wndclass;
	
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = DxjWindow::WndStaticProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = sizeof(DxjWindow*);
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon(hInstance,szIcon);
	wndclass.hCursor       = NULL;
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName  = szMenu;
	wndclass.lpszClassName = DXJCLASS;
	
	if(!RegisterClass(&wndclass))
		exit(FALSE);

	Scintilla_RegisterClasses(hInstance);

	hwndDxj = CreateWindowEx(
							 WS_EX_CLIENTEDGE,
							 DXJCLASS,
							 szCaption,
							 WS_CAPTION | WS_SYSMENU | WS_THICKFRAME |
							 WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
							 WS_MAXIMIZE | WS_CLIPCHILDREN,
							 CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
							 NULL,
							 NULL,
							 hInstance,
							 this);
	if (!hwndDxj)
		exit(FALSE);
	ShowWindow(hwndDxj,SW_SHOWNORMAL);
	SetWindowLong(hwndDxj, 0, reinterpret_cast<LONG>(this));

	fLog = fopen("log.txt","a");
	if(fLog == NULL)
	{
		MessageBox(NULL,"open log","失败",MB_OK);
		exit(FALSE);
	}
	fputs("DxjWindow 注册并创建窗口 \n",fLog);
}

DxjWindow::~DxjWindow()
{
	fclose(fLog);
	fputs("~DxjWindow\n",fLog);
}

LRESULT DxjWindow::WndProc(
				   HWND hWnd,
				   UINT iMessage,
				   WPARAM wParam,
				   LPARAM lParam)
{
	switch(iMessage)
	{
	case WM_CREATE:
		hwndEditor = CreateWindow(
                 		"Scintilla",
                 		"Source",
                 		WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,
                 		0, 0,
                 		100, 100,
                 		hwndDxj,
                 		(HMENU)IDM_SRCWIN,
                 		hInstance,
                 		0);
		if (!hwndEditor){
			MessageBox(hwndDxj,"Editor","创建失败",MB_OK);
			exit(FALSE);			
		}
		::ShowWindow(hwndEditor, SW_SHOWNORMAL);
		::SetFocus(hwndEditor);
		fputs("DxjWindow WM_CREATE\n",fLog);
		break;
	case WM_ACTIVATE:
		SetFocus(hwndEditor);
		fputs("DxjWindow WM_ACTIVATE\n",fLog);
		break;
	case WM_SIZE:
		{
			RECT rcClient;
			GetClientRect(hwndDxj, &rcClient);
			int w = rcClient.right - rcClient.left + 1;
			int h = rcClient.bottom - rcClient.top + 1;
			::SetWindowPos(hwndEditor, 0, 0, 0, w, h, 0);
		}
		fputs("DxjWindow WM_SIZE\n",fLog);
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd,&ps);
			RECT rcClient = {0,0,0,0};
			GetClientRect(hWnd,&rcClient);
			HBRUSH brushSurface = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
			FillRect(ps.hdc, &rcClient, brushSurface);
			DeleteObject(brushSurface);
			EndPaint(hWnd,&ps);			
		}
		fputs("DxjWindow WM_PAINT\n",fLog);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		fputs("DxjWindow WM_DESTROY\n",fLog);
		break;
	default:
		fputs("DxjWindow DefWindowProc\n",fLog);
		return DefWindowProc(hWnd,iMessage,wParam,lParam);
	}
	return 0l;
}

LRESULT PASCAL DxjWindow::WndStaticProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	DxjWindow *main = (DxjWindow *)GetWindowLong(hWnd,0);
	if(main == 0) fputs("(DxjWindow *)GetWindowLong(hWnd,0)为空\n",fLog);
	char sSciDxj[20];
	fputs(itoa(iMessage,sSciDxj,10),fLog);
	fputs("\n",fLog);
	return main->WndProc(hWnd, iMessage, wParam, lParam);
}

int DxjWindow::EventLoop()
{
	MSG msg;
	HACCEL hAccTable = LoadAccelerators(hInstance,"ACCELS");
	char sMessage[20];
	while(GetMessage(&msg,NULL,0,0))
	{
		fputs("GetMessage从消息队列取到的消息 ",fLog);
		fputs(itoa(reinterpret_cast<LONG>(msg.hwnd),sMessage,10),fLog);//hwnd
		fputs(" ",fLog);
		strcpy(sMessage,"");
		fputs(itoa(msg.message,sMessage,10),fLog);	//message	
		fputs(" ",fLog);
		strcpy(sMessage,"");
		fputs(itoa(msg.lParam,sMessage,10),fLog);	//lParam
		fputs(" ",fLog);
		strcpy(sMessage,"");
		fputs(itoa(msg.wParam,sMessage,10),fLog);	//wParam
		fputs("\n",fLog);
		if(IsDialogMessage(hwndDxj, &msg))
		{
			if(TranslateAccelerator(hwndDxj,hAccTable,&msg) == 0)
			{
				fputs("TranslateAccelerator翻译加速键表 ",fLog);
				strcpy(sMessage,"");
				fputs(itoa(reinterpret_cast<LONG>(msg.hwnd),sMessage,10),fLog);//hwnd
				fputs(" ",fLog);
				strcpy(sMessage,"");
				fputs(itoa(msg.message,sMessage,10),fLog);	//message	
				fputs(" ",fLog);
				strcpy(sMessage,"");
				fputs(itoa(msg.lParam,sMessage,10),fLog);	//lParam
				fputs(" ",fLog);
				strcpy(sMessage,"");
				fputs(itoa(msg.wParam,sMessage,10),fLog);	//wParam
				fputs("\n",fLog);
				TranslateMessage(&msg);
				fputs("TranslateMessage翻译虚拟键消息到字符消息 ",fLog);
				strcpy(sMessage,"");
				fputs(itoa(reinterpret_cast<LONG>(msg.hwnd),sMessage,10),fLog);//hwnd
				fputs(" ",fLog);
				strcpy(sMessage,"");
				fputs(itoa(msg.message,sMessage,10),fLog);	//message	
				fputs(" ",fLog);
				strcpy(sMessage,"");
				fputs(itoa(msg.lParam,sMessage,10),fLog);	//lParam
				fputs(" ",fLog);
				strcpy(sMessage,"");
				fputs(itoa(msg.wParam,sMessage,10),fLog);	//wParam
				fputs("\n",fLog);
				DispatchMessage(&msg);
				fputs("DispatchMessage投递消息到消息队列 ",fLog);
				strcpy(sMessage,"");
				fputs(itoa(reinterpret_cast<LONG>(msg.hwnd),sMessage,10),fLog);//hwnd
				fputs(" ",fLog);
				strcpy(sMessage,"");
				fputs(itoa(msg.message,sMessage,10),fLog);	//message	
				fputs(" ",fLog);
				strcpy(sMessage,"");
				fputs(itoa(msg.lParam,sMessage,10),fLog);	//lParam
				fputs(" ",fLog);
				strcpy(sMessage,"");
				fputs(itoa(msg.wParam,sMessage,10),fLog);	//wParam
				fputs("\n",fLog);
			}				
		}
		else
		{
			if(TranslateAccelerator(msg.hwnd,hAccTable,&msg) == 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}			
		}
	}
	return msg.wParam;
}

int PASCAL WinMain(
					HINSTANCE hInstance,
					HINSTANCE,
					LPSTR lpszCmdLine,
					int nCmdShow)
{
	DxjWindow main(hInstance);
	main.EventLoop();
	return 0;
}