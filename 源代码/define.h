#pragma once
#include <iostream>
//#include<string>
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>       
#include <ws2tcpip.h>
#include<fstream>
#include<ctime>
using namespace std;
#define SOURCE_FILE "RESULT.txt"
#pragma comment(lib, "Ws2_32.lib")


double dur;
int iTTL;
bool flag = false;
bool fail = false;
//IP报头
typedef struct IP_HEADER
{
	unsigned char Head_Len : 4;              //4位头部长度
	unsigned char Version : 4;               //4位版本号
	unsigned char Tos;                       //8位服务类型
	unsigned short Total_Len;                //16位总长度
	unsigned short Identifier;               //16位标识符
	unsigned short Frag_and_Flag;            //3位标志加13位片偏移
	unsigned char TTL;                       //8位生存时间
	unsigned char Protocol;                  //8位上层协议号
	unsigned short Check_Sum;                //16位校验和
	unsigned long Sour_IPAddress;            //32位源IP地址
	unsigned long Dest_IPAddress;            //32位目的IP地址
} IP_HEADER;

//ICMP报头
typedef struct ICMP_HEADER
{
	BYTE type;            //8位类型字段
	BYTE code;            //8位代码字段
	USHORT Check_Sum;     //16位校验和
	USHORT id;            //16位标识符
	USHORT seq;           //16位序列号
} ICMP_HEADER;

//报文解码结构
typedef struct DECODE_RESULT
{
	USHORT usSeqNo;        //序列号
	DWORD dwRoundTripTime; //往返时间
	in_addr dwIPaddr;      //返回报文的IP地址
}DECODE_RESULT;

//计算网际校验和函数
USHORT checksum(USHORT* pBuf, int iSize)
{
	unsigned long cksum = 0;
	while (iSize > 1)
	{
		cksum += *pBuf++;
		iSize -= sizeof(USHORT);
	}
	if (iSize)//如果 iSize 为正，即为奇数个字节
	{
		cksum += *(UCHAR*)pBuf; //则在末尾补上一个字节，使之有偶数个字节
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (USHORT)(~cksum);
}

//对数据包进行解码
BOOL DecodeIcmpResponse(char* pBuf, int iPacketSize, DECODE_RESULT& DecodeResult,
	BYTE ICMP_ECHO_REPLY, BYTE  ICMP_TIMEOUT, fstream &file)
{
	//检查数据报大小的合法性
	IP_HEADER* pIpHdr = (IP_HEADER*)pBuf;
	int iIpHdrLen = pIpHdr->Head_Len * 4;    //ip报头的长度是以4字节为单位的

	//若数据包大小 小于 IP报头 + ICMP报头，则数据报大小不合法
	if (iPacketSize < (int)(iIpHdrLen + sizeof(ICMP_HEADER)))
		return FALSE;

	//根据ICMP报文类型提取ID字段和序列号字段
	ICMP_HEADER* pIcmpHdr = (ICMP_HEADER*)(pBuf + iIpHdrLen);//ICMP报头 = 接收到的缓冲数据 + IP报头
	USHORT usID, usSquNo;

	if (pIcmpHdr->type == ICMP_ECHO_REPLY)    //ICMP回显应答报文
	{
		usID = pIcmpHdr->id;        //报文ID
		usSquNo = pIcmpHdr->seq;    //报文序列号
	}
	else if (pIcmpHdr->type == ICMP_TIMEOUT)//ICMP超时差错报文
	{
		char* pInnerIpHdr = pBuf + iIpHdrLen + sizeof(ICMP_HEADER); //载荷中的IP头
		int iInnerIPHdrLen = ((IP_HEADER*)pInnerIpHdr)->Head_Len * 4; //载荷中的IP头长
		ICMP_HEADER* pInnerIcmpHdr = (ICMP_HEADER*)(pInnerIpHdr + iInnerIPHdrLen);//载荷中的ICMP头

		usID = pInnerIcmpHdr->id;        //报文ID
		usSquNo = pInnerIcmpHdr->seq;    //序列号
	}
	else
	{
		return false;
	}

	//检查ID和序列号以确定收到期待数据报
	if (usID != (USHORT)GetCurrentProcessId() || usSquNo != DecodeResult.usSeqNo)
	{
		return false;
	}
	//记录IP地址并计算往返时间
	DecodeResult.dwIPaddr.s_addr = pIpHdr->Sour_IPAddress;
	DecodeResult.dwRoundTripTime = GetTickCount() - DecodeResult.dwRoundTripTime;

	//处理正确收到的ICMP数据报
	if (pIcmpHdr->type == ICMP_ECHO_REPLY || pIcmpHdr->type == ICMP_TIMEOUT)
	{
		//输出往返时间信息
		if (DecodeResult.dwRoundTripTime)
			file << "      " << DecodeResult.dwRoundTripTime << "ms";
		else
			file << "      " << "<1ms";
	}
	return true;
}
void Start_TraceRoute(char* IpAddress)
{
	clock_t start, end;
	fail = false;
	fstream file;
	file.open(SOURCE_FILE, ios::out);
	//初始化Windows sockets网络环境
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)   //初始化失败
	{
		fail = true;
		file << "Initial Error!";
		return ;
	}
	

	//cout << "请输入一个IP地址或域名：";
	//cin >> IpAddress;
	
	//得到要查询的IP地址
	unsigned long Dest_IPAddress = inet_addr(IpAddress);
	

	//转换不成功时按域名解析
	if (Dest_IPAddress == INADDR_NONE)
	{
		hostent* pHostent = gethostbyname(IpAddress);
		if (pHostent)
		{
			Dest_IPAddress = (*(in_addr*)pHostent->h_addr).s_addr;
		}
		else
		{
			fail = true;
			file << "输入的IP地址或域名无效!" << endl;
			file.close();
			WSACleanup();
			return;
		}
	}

	start = clock();
	file << "追踪到 " << IpAddress << "的路由信息如下(点击下一步直至追踪结束)：" << endl;

	//填充目的端socket套接字地址
	sockaddr_in destSockAddr;
	ZeroMemory(&destSockAddr, sizeof(sockaddr_in));
	destSockAddr.sin_family = AF_INET;
	destSockAddr.sin_addr.s_addr = Dest_IPAddress;

	//创建原始套接字
	SOCKET sockRaw = WSASocketW(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);

	//超时时间
	int iTimeout = 1000;

	//设置接收超时时间
	setsockopt(sockRaw, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeout, sizeof(iTimeout));

	//设置发送超时时间
	setsockopt(sockRaw, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeout, sizeof(iTimeout));

	//构造ICMP回显请求消息，并以TTL递增的顺序发送报文
	//ICMP类型字段
	const BYTE ICMP_ECHO_REQUEST = 8;    //请求回显
	const BYTE ICMP_ECHO_REPLY = 0;    //回显应答
	const BYTE ICMP_TIMEOUT = 11;   //传输超时

	//其他常量定义
	const int DEF_ICMP_DATA_SIZE = 32;    //ICMP报文默认数据字段长度
	const int MAX_ICMP_PACKET_SIZE = 1024;  //ICMP报文最大长度（包括报头）
	const DWORD DEF_ICMP_TIMEOUT = 1000;  //回显应答超时时间
	const int DEF_MAX_HOP = 30;    //最大跳站数

	//填充ICMP报文中每次发送时不变的字段
	char IcmpSendBuf[sizeof(ICMP_HEADER) + DEF_ICMP_DATA_SIZE];//发送缓冲区
	memset(IcmpSendBuf, 0, sizeof(IcmpSendBuf));               //初始化发送缓冲区
	char IcmpRecvBuf[MAX_ICMP_PACKET_SIZE];                      //接收缓冲区
	memset(IcmpRecvBuf, 0, sizeof(IcmpRecvBuf));               //初始化接收缓冲区

	ICMP_HEADER* pIcmpHeader = (ICMP_HEADER*)IcmpSendBuf;
	pIcmpHeader->type = ICMP_ECHO_REQUEST; //类型为请求回显
	pIcmpHeader->code = 0;                //代码字段为0
	pIcmpHeader->id = (USHORT)GetCurrentProcessId();    //ID字段为当前进程号
	memset(IcmpSendBuf + sizeof(ICMP_HEADER), 'E', DEF_ICMP_DATA_SIZE);//数据字段

	USHORT usSeqNo = 0;            //ICMP报文序列号

	iTTL = 1;            //TTL初始值为1
	flag = false;
	

	BOOL bReachDestHost = FALSE;        //循环退出标志
	int iMaxHot = DEF_MAX_HOP;  //循环的最大次数
	DECODE_RESULT DecodeResult;    //传递给报文解码函数的结构化参数
	
	while (!bReachDestHost && iMaxHot--)
	{
		//设置IP报头的TTL字段
		setsockopt(sockRaw, IPPROTO_IP, IP_TTL, (char*)&iTTL, sizeof(iTTL));
		file << iTTL;    //输出当前序号

		//填充ICMP报文中每次发送变化的字段
		((ICMP_HEADER*)IcmpSendBuf)->Check_Sum = 0;                   //校验和先置为0
		((ICMP_HEADER*)IcmpSendBuf)->seq = htons(usSeqNo++);    //填充序列号
		((ICMP_HEADER*)IcmpSendBuf)->Check_Sum =
			checksum((USHORT*)IcmpSendBuf, sizeof(ICMP_HEADER) + DEF_ICMP_DATA_SIZE); //计算校验和

		//记录序列号和当前时间
		DecodeResult.usSeqNo = ((ICMP_HEADER*)IcmpSendBuf)->seq;    //当前序号
		DecodeResult.dwRoundTripTime = GetTickCount();                          //当前时间

		//发送TCP回显请求信息
		sendto(sockRaw, IcmpSendBuf, sizeof(IcmpSendBuf), 0, (sockaddr*)&destSockAddr, sizeof(destSockAddr));

		//接收ICMP差错报文并进行解析处理
		sockaddr_in from;           //对端socket地址
		int iFromLen = sizeof(from);//地址结构大小
		int iReadDataLen;           //接收数据长度
		while (1)
		{
			//接收数据
			iReadDataLen = recvfrom(sockRaw, IcmpRecvBuf, MAX_ICMP_PACKET_SIZE, 0, (sockaddr*)&from, &iFromLen);
			if (iReadDataLen != SOCKET_ERROR)//有数据到达
			{
				//对数据包进行解码
				if (DecodeIcmpResponse(IcmpRecvBuf, iReadDataLen, DecodeResult, ICMP_ECHO_REPLY, ICMP_TIMEOUT,file))
				{
					//到达目的地，退出循环
					if (DecodeResult.dwIPaddr.s_addr == destSockAddr.sin_addr.s_addr)
					{
						bReachDestHost = true;
						flag = true;
						end = clock();
						dur = (double)(end - start);
					}
					//输出IP地址
					file << '\t' << inet_ntoa(DecodeResult.dwIPaddr) << endl;
					
					break;
				}
			}
			else if (WSAGetLastError() == WSAETIMEDOUT)    //接收超时，输出*号
			{
				file << "         *" << '\t' << "请求超时" << endl;
				break;
			}
			else
			{
				break;
			}
		}
		iTTL++;    //递增TTL值
	}
	/*if (iTTL > 30)
	{
		file << "NO\0";
	}*/
	file.close();
	WSACleanup();
	//system("pause");
	
	return ;
}