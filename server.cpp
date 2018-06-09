#include <iostream.h>
#include <winsock2.h>
#include <stdio.h>
#include <conio.h> 
#include <string.h>
#include <windows.h>

#pragma comment(lib,"bufferoverflowU.lib")

#define UM_NUMOFUSERS 0x03

#define UM_QUIT 0x08
#define UM_MES 0x09
#define UM_ONLINE 0x10
#define UM_VER 0x11
#define USER_COUNT 40
#define BUFFER_LEN 500


HANDLE  hUserData;

	//SOCKET s[USER_COUNT];
	SOCKET server_sock ;
	sockaddr_in me;
	//sockaddr you[USER_COUNT];
	int addr_size = sizeof (sockaddr);
	int num_users = 0;

///////////////////////////////////
struct users
{	
	HANDLE  thread;
	DWORD id;
	int Number;
	int status;
	SOCKET sock ;
	sockaddr you ;
	char name[32]; 
	int ver; 
	char subver[8];
	char date[32];
};
	struct users *pU;

	COORD coord1;
HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
CONSOLE_CURSOR_INFO ConCurInf;
void Setcursor(int x,int y)
{
	coord1.Y =y  ; coord1.X = x;
	SetConsoleCursorPosition(hStdout,  coord1);
}


char mes[]="HTTP/1.1 200 OK\n\
Date: Fri, 03 Jul 2015 01:05:33 GMT\n\
Server: Microsoft-IIS/6.0\n\
X-Powered-By: ASP.NET\n\
X-AspNet-Version: 4.0.30319\n\
Location: /index.html\n\
Cache-Control: public, max-age=86400\n\
Content-Type: text/html; charset=utf-8\n\
Content-Length: 130\n\n";

char mes2[]="\
<html>\
<body>\
 Web server page \
</body></html>\n\n";


//===============================================================
unsigned long __stdcall User_receiving_message(void *Param)
{
extern	struct users *pU;
SYSTEMTIME sm;
int  ver;
try
	{
		int* point=(int*)Param;
		int number=point[0];

		char buffer[BUFFER_LEN];
		int error;
		//pU[number].status=1;

	while (1)
	{
		memset(buffer,0,BUFFER_LEN);
		error = recv (pU[number].sock,buffer,100,0);
		if ((error==0)||(error==INVALID_SOCKET))
		{
			//closesocket (pU[number].sock);//pU[number].status=0;
			//Setcursor(0,1); printf( "Error. User %d quit!\n",number);
			goto exit_t;
		}
		   //send(pU[number].sock,mes,strlen(mes),0);
		  if(error<100)
		 {
			 send(pU[number].sock,mes,strlen(mes),0);
			send(pU[number].sock,mes2,strlen(mes2),0);
		 }

		switch((unsigned char)buffer[0])
		{

			case UM_QUIT:
				{ 
				//closesocket (pU[number].sock); //pU[number].status=0;
				//printf( "User %d quit!\n",number);
				goto exit_t;
				}
				break;

			case UM_MES:
				Setcursor(0,1); printf("User %d mes  %s \n",number, &buffer[1]);
				break;

			case UM_ONLINE:
				WaitForSingleObject(hUserData, INFINITE);
				wsprintf(pU[number].name,&buffer[1]);
				GetLocalTime(&sm);
				wsprintf(pU[number].date,"%d/%d %d:%d ", sm.wDay,sm.wMonth,sm.wHour,sm.wMinute );
				//printf("User %s is online\n", pU[number].name);
				ReleaseMutex(hUserData);
				break;

			case UM_VER:
				{
				WaitForSingleObject(hUserData, INFINITE);
				char s[4];memcpy(s,&buffer[1],3); s[4]=0;
				ver=atoi (s);
				//ver=atoi (&buffer[1]);
				pU[number].ver=ver;
				strcpy(pU[number].subver, &buffer[4]);
				ReleaseMutex(hUserData);
				} break;

		
			default:
					//Setcursor(0,1); 
					printf("User %d %s \n",number, buffer);
			//	if(buffer[0]=='G')	
					
			break;
		}
	}
		
exit_t:

WaitForSingleObject(hUserData, INFINITE);
closesocket (pU[number].sock);
pU[number].status=0;
num_users--;

///--add offlinr user-----------
int ok=0,i;
for( i=USER_COUNT-1; i>num_users;i--)
		if(pU[i].status==0)
			if(!strcmp(pU[number].name, pU[i].name))
				{	
				memcpy(&pU[i],&pU[number],sizeof(struct users));
				memset(&pU[number],0,sizeof(struct users));
				ok=1;
				}
if(ok==0)
for( i=USER_COUNT-1; i>num_users;i--)
		if(pU[i].status==0)
			if(!pU[i].name[0])
				{	
				memcpy(&pU[i],&pU[number],sizeof(struct users));
				memset(&pU[number],0,sizeof(struct users));
				}
///--add offlinr user-----------			


ReleaseMutex(hUserData);
//num_users--;
//	SendMessage(hNewWnd, WM_COMMAND,G_TR_OK,number); // 
//		pGT[number].thread=0;//	h_g_transl=0;
		ExitThread(0);
	}

catch (...)
	{
		//pGT[number].thread=0;
		MessageBox(0,  "User_receiving_message:Unknown error","Warning", MB_OK);
		ExitThread(1);
		return 1;
	}
}
//===============================================================


//===============================================================
unsigned long __stdcall Print_User_list(void *Param)
{
extern	struct users *pU;
int i;
CONSOLE_SCREEN_BUFFER_INFO csbi;
GetConsoleScreenBufferInfo( hStdout, &csbi );
ConCurInf.dwSize = 10;
ConCurInf.bVisible = FALSE;
SetConsoleCursorInfo(hStdout, &ConCurInf);
try
	{		
		while (1)
		{
			
				WaitForSingleObject(hUserData, INFINITE);
				Setcursor(0,2+10);
				for( i=0; i<USER_COUNT;i++) printf("                                                        " );

				
				Setcursor(0,3+10);
				for( i=0; i<USER_COUNT;i++)
				{
					SetConsoleTextAttribute(hStdout, 6);

					if(pU[i].status==1)
						SetConsoleTextAttribute(hStdout, 10 );

					if(pU[i].name[0])
					{
						if(pU[i].ver)
							printf("%s  ver. %d.%d%s  %s\n", pU[i].name ,  pU[i].ver/100,pU[i].ver%100, pU[i].subver,pU[i].date );
						else
							printf("%s  %s\n", pU[i].name, pU[i].date );
					}
				}
			ReleaseMutex(hUserData);
			SetConsoleTextAttribute(hStdout, 7);
			Sleep(5000)	;
		}
		


	ExitThread(0);
	}

catch (...)
	{
		//pGT[number].thread=0;
		MessageBox(0,  "Print Users:Unknown error","Warning", MB_OK);
		ExitThread(1);
		return 1;
	}
}
//===============================================================

/////////////////////
DWORD PULid;
HANDLE thread;

void main ( void ) //WSOCK32.LIB
{

/////////////////////////
//SetConsoleTextAttribute(hStdout, 6);
				
//struct users *pU;
	pU=(struct users*)new struct users [USER_COUNT];
	if(pU==0)
			{
				Setcursor(0,1); printf("Cannot allocate memory ");
				return;
			}							
	memset(pU,0,sizeof(struct users)*USER_COUNT);
	//////////////////


/*	SOCKET s[USER_COUNT];
	SOCKET server_sock ;
	sockaddr_in me;
	sockaddr you[USER_COUNT];
	int addr_size = sizeof (sockaddr);
	int num_users = 0;*/
	sockaddr temp_you;
	SOCKET	temp_sock;

	int error ;
	int user=0;

	//char player1 = 0;
	//char player2 = 0;

	printf( "TM-database Server (streem version)\n");
	//printf( "By Sasha P\n\n");
	WSADATA w;
	error = WSAStartup (0x0202,&w);
	if (error)
	{
		Setcursor(0,1); printf( "Error:  You need WinSock 2.2!\n");
		goto exit;
	}
	if (w.wVersion!=0x0202)
	{
		Setcursor(0,1); printf("Error:  Wrong WinSock version!\n");
		goto exit;
	}

	server_sock = socket (AF_INET,SOCK_STREAM,0);
	me.sin_family = AF_INET;
	me.sin_port = htons (8080);
	me.sin_addr.s_addr = htonl (INADDR_ANY);
	if (bind(server_sock,(LPSOCKADDR)&me,sizeof(me))==SOCKET_ERROR)
	{
		Setcursor(0,1); printf("Error:  Unable to bind socket!\n");
		goto exit;
	}
	if (listen(server_sock,1)==SOCKET_ERROR)
	{
		Setcursor(0,1); printf("Error:  Unable to listen!\n");
		goto exit;
	}


	thread= CreateThread( NULL, 0,Print_User_list, 0, 0, &PULid );

	//char buffer[2000];
	printf( "Listening...\n");
	while (1)//num_users<USER_COUNT)
	{
		temp_sock = accept (server_sock,&temp_you,&addr_size);

		if (temp_sock==INVALID_SOCKET)
		{
			Setcursor(0,1); printf( "Error:  Unable to accept connection!\n");
			goto exit;
		}
		else
		{
			if(num_users<USER_COUNT)
			{
				
				WaitForSingleObject(hUserData, INFINITE);
				num_users++;
				ReleaseMutex(hUserData);

				for(int i=0; i<num_users;i++)
				if(pU[i].status==0)
				{	
					WaitForSingleObject(hUserData, INFINITE);
					 
					pU[i].sock=temp_sock;
					pU[i].you=temp_you;

					pU[i].status=1;
					pU[i].Number=i;
					pU[i].thread= CreateThread( NULL, 0, User_receiving_message, &pU[i].Number, 0, &pU[i].id );
					 
					ReleaseMutex(hUserData);

					//printf( "User %d joined!\n",i);
					//sprintf (buffer,"%c%d",UM_NUMOFUSERS,num_users);
					///send (s[num_users],buffer,2,0);
					/////////////////
					break;
				}
				//////////	
			}
			else
			{	Setcursor(0,1); printf( "Cannot connect User. Max users achieved\n");}
			
		}
	}

	/*
	printf( "Starting Game!\n");
	closesocket (server_sock);
	char buffer[2];
	sprintf (buffer,"%c%d",UM_STARTGAME,0);
	error = send (s[1],buffer,2,0);
	if ((error==0)||(error==SOCKET_ERROR))
	{
		printf("Error:  Player 1 quit!\n");
		goto exit;
	}
	error = send (s[2],buffer,2,0);
	if ((error==0)||(error==SOCKET_ERROR))
	{
		printf( "Error:  Player 2 quit!\n");
		goto exit;
		return;
	}

	while (true)
	{
		int error = recv (s[1],buffer,2,0);
		if ((error==0)||(error==INVALID_SOCKET))
		{
			printf( "Error:  Player 1 quit!\n");
			closesocket (s[2]);
			goto exit;
		}
		if ((buffer[0]==UM_ROCK)||(buffer[0]==UM_SCISSOR)||(buffer[0]==UM_PAPER))
		{
			player1 = buffer[0];
		}
		if (buffer[0]==UM_QUIT)
		{
			sprintf (buffer,"%c%d",UM_QUIT,0);
			send (s[2],buffer,2,0);
			closesocket (s[1]);
			closesocket (s[2]);
			
			printf( "Player 1 quit\n");
			goto exit;
		}

		error = recv (s[2],buffer,2,0);
		if ((error==0)||(error==INVALID_SOCKET))
		{
			printf("Error:  Player 2 quit!\n");
			closesocket (s[1]);
			goto exit;
		}
		if ((buffer[0]==UM_ROCK)||(buffer[0]==UM_SCISSOR)||(buffer[0]==UM_PAPER))
		{
			player2 = buffer[0];
		}
		if (buffer[0]==UM_QUIT)
		{
			sprintf (buffer,"%c%d",UM_QUIT,0);
			send (s[1],buffer,2,0);
			closesocket (s[1]);
			closesocket (s[2]);
			WSACleanup ();
			printf( "Player 2 quit\n");
			goto exit;
		}

		if (((int)player1>0)&&((int)player2>0))
		{
			sprintf (buffer,"%c%d",player2,0);
			send (s[1],buffer,2,0);
			sprintf (buffer,"%c%d",player1,0);
			send (s[2],buffer,2,0);
			printf("Round completed!\n");
		}
	}*/
	char a;
	cin >> a;
/////////
exit:
	delete(	pU);
	WSACleanup ();
	////////////////
	CloseHandle(hUserData);
}