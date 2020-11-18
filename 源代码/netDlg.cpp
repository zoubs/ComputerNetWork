
// netDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "net.h"
#include "netDlg.h"
#include "afxdialogex.h"
#include"define.h"
#include<iostream>
#include<fstream>
#include<string>
//#include<string>
using namespace std;
char Address[256];

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CnetDlg 对话框



CnetDlg::CnetDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NET_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CnetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CnetDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_NEXT, &CnetDlg::OnBnClickedButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_START, &CnetDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_NEW, &CnetDlg::OnBnClickedButtonNew)
END_MESSAGE_MAP()


// CnetDlg 消息处理程序

BOOL CnetDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_MAXIMIZE);

	ShowWindow(SW_MINIMIZE);

	GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(false);

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CnetDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CnetDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CnetDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//extern void Start_TraceRoute(char* Address);
int i = 0;
fstream File_In;
bool isopen = false;
//bool flag = false;
bool Trace_Route_End = false;  //代表还没有到目的节点
int num = 0; //行数
void CnetDlg::OnBnClickedButtonNext()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!Trace_Route_End)
	{
		CString Output1;
		string temp;
		GetDlgItemText(IDC_EDIT_OUTPUT, Output1);
		Output1 += "\r\n";    //输出换行
		if (!File_In.eof())
		{
			getline(File_In, temp);
			if (File_In.eof())
			{
				if (num != 1)
				{
					if (iTTL > 30 && flag == false)
					{
						CString Output("已达到30跳，未追踪到目的节点！\n ");
						//CString str;
						//str.Format(_T("%lf"), dur);
						Output = Output1 + Output;
						//Output += str;
						//Output += " ms";
						SetDlgItemText(IDC_EDIT_OUTPUT, Output);
					}
					else
					{
						CString Output("已到达目的节点，追踪结束！\n花费时间: ");
						CString str;
						str.Format(_T("%lf"), dur);
						Output = Output1 + Output;
						Output += str;
						Output += " ms";
						SetDlgItemText(IDC_EDIT_OUTPUT, Output);
					}
				}
				GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(false);
				if (isopen)
				{
					File_In.close();
					isopen = false;
				}
			}
			else
			{
				num++;
				CString Output(temp.c_str());
				Output = Output1 + Output;
				SetDlgItemText(IDC_EDIT_OUTPUT, Output);
			}
		}
	}
	//Output.Format(_T("%d"), t);
	
}

int start_count = 0;   //开始键点击次数

void CnetDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码
	flag = false;
	start_count++;
	CString Input;
	// Output;
	string temp;
	GetDlgItemText(IDC_EDIT_INPUT, Input);
	int n = Input.GetLength(); //获取str的字符数  
	int len = WideCharToMultiByte(CP_ACP, 0, Input, n, NULL, 0, NULL, NULL); //获取宽字节字符的大小，大小是按字节计算的  
	//char* pChar = new char[len + 1]; //以字节为单位  
	WideCharToMultiByte(CP_ACP, 0, Input, n, Address, len, NULL, NULL); //宽字节编码转换成多字节编码  
	Address[len] = '\0'; //多字节字符以'\0'结束

	if (start_count == 1)
	{
		Start_TraceRoute(Address);
	}
	File_In.open(SOURCE_FILE, ios::in);
	isopen = true;
	while (!File_In.eof())
	{
		getline(File_In, temp);
		if (File_In.eof())
		{
			if (isopen)
			{
				File_In.close();
				isopen = false;
			}
			break;
		}
		CString Output(temp.c_str());
		//Output += "\r\n";
		SetDlgItemText(IDC_EDIT_OUTPUT, Output);
		num++;
		if (num == 1)
		{
			break;
		}
	}
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(false);
	GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(true);
	GetDlgItem(IDC_EDIT_INPUT)->EnableWindow(false);
	//GetDlgItem(IDC_EDIT_OUTPUT)->EnableWindow(false);
	
}


void CnetDlg::OnBnClickedButtonNew()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(true);
	GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(false);
	GetDlgItem(IDC_EDIT_INPUT)->EnableWindow(true);
	//GetDlgItem(IDC_EDIT_OUTPUT)->EnableWindow(true);
	GetDlgItem(IDC_EDIT_INPUT)->SetWindowTextW(0);
	GetDlgItem(IDC_EDIT_OUTPUT)->SetWindowTextW(0);

	start_count = 0;
	num = 0;
	if (isopen)
	{
		File_In.close();
		isopen = false;
	}
	//CString Out;
	//SetDlgItemText(IDC_EDIT_INPUT,)
}
