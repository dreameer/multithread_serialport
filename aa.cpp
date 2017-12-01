// aa.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include <Windows.h>
#define READ_TIMEOUT      500      // milliseconds
#define READ_BUF_SIZE     14        // byte
#define WRITE_BUF_SIZE     13        // byte

void HandleASuccessfulRead(char *lpBuf, int dwRead);
BOOL ReadABuffer(HANDLE hComm);
BOOL WriteABuffer(HANDLE hComm,unsigned char * lpBuf, DWORD dwToWrite);

typedef struct data
{
	HANDLE serailport;
	int flag;
}*PDATA;

DWORD WINAPI funr(void *param)
{
	PDATA mdata = (PDATA)param;
	for(;;)
	{
		if(mdata->flag==1)
		{
			if(ReadABuffer(mdata->serailport))
			{
				//Sleep(500);
			}
		}
		else
		{
			break;
		}
	}
	return 0;
}
DWORD WINAPI funw(void *param)
{
	PDATA mdata = (PDATA)param;
	bool writestatus = false;
	long long count = 0;
	for(;;)
	{
		if(mdata->flag==1)
		{  
			printf("give a cmd from key board:\n"); 
			unsigned char buff[WRITE_BUF_SIZE] = {0};
			buff[0] = 0x55;
			buff[1] = 0xaa;
			signed short s16 = 0;
			unsigned short u16 = 0;
			int datalength = 0;
			char cmd = getchar();
			char enter = getchar();
			switch(cmd)
			{
			case 'q':
				buff[2] = 0x01;
				buff[4] = 0x01;
				buff[6] = 0x01;
				datalength = 1;
				break;
			case 'w':
				buff[2] = 0x01;
				buff[4] = 0x01;
				buff[6] = 0x00;
				datalength = 1;
				break;
			case 'h':
				buff[2] = 0x0002;
				buff[4] = 0x02;
				s16 = -10;
				memcpy(&buff[6],&s16,sizeof(short));
				datalength = 2;
				break;
			case 'k':
				buff[2] = 0x0002;
				buff[4] = 0x02;
				s16 = 10;
				memcpy(&buff[6],&s16,sizeof(short));
				datalength = 2;
				break;
			case 'u':
				buff[2] = 0x0003;
				buff[4] = 0x02;
				s16 = -10;
				memcpy(&buff[6],&s16,sizeof(short));
				datalength = 2;
				break;
			case 'j':
				buff[2] = 0x0003;
				buff[4] = 0x02;
				s16 = 10;
				memcpy(&buff[6],&s16,sizeof(short));
				datalength = 2;
				break;
			case 'a':
				buff[2] = 0x0004;
				buff[4] = 0x02;
				u16 = 40;
				memcpy(&buff[6],&u16,sizeof(short));
				datalength = 2;
				break;
			case 's':
				buff[2] = 0x0005;
				buff[4] = 0x02;
				u16 = 40;
				memcpy(&buff[6],&u16,sizeof(short));
				datalength = 2;
				break;
			default:break;
			}
			printf("get cmd:%x\n",cmd);
			unsigned int cyc = 0;
			unsigned char cl,ch;
			for(int i=0;i<6+datalength;i++)
			{
				cyc = cyc + (unsigned int)(buff[i]);
			}
			cl = (cyc&0x000000ff);
			ch = ((cyc>>8)&0x000000ff);
			buff[6+datalength] = cl;
			buff[7+datalength] = ch;
			buff[8+datalength] = 0x13;
			writestatus = WriteABuffer(mdata->serailport,buff,9+datalength);
			if(writestatus){
				for(int i=0;i<WRITE_BUF_SIZE;i++)
				{
					printf("%X ",buff[i]);
				}
				printf("\n");
			}
			else
			{
				printf("one faied write:%d\n",count);
				count++;
			}
		}
		else
		{
			break;
		}
	}
	return 0;
}



void HandleASuccessfulRead(char *lpBuf, int dwRead)
{
	
	if(dwRead==READ_BUF_SIZE)

	{
		for(int i=0;i<dwRead;i++)
		{
			//printf("%x ",lpBuf[i]&0x000000ff);
		}
		//printf("\n");
	}
	
}
BOOL ReadABuffer(HANDLE hComm)
{
    DWORD dwRead;
	BOOL fWaitingOnRead = FALSE;
	OVERLAPPED osReader = {0};
	char lpBuf[READ_BUF_SIZE];
	// Create the overlapped event. Must be closed before exiting
	// to avoid a handle leak.
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (osReader.hEvent == NULL)
		// Error creating overlapped event; abort.
	{
		printf("Error creating overlapped event; abort.\n");
	}
	if (!fWaitingOnRead) {
		// Issue read operation.
		if (!ReadFile(hComm, lpBuf, READ_BUF_SIZE, &dwRead, &osReader)) {
			if (GetLastError() != ERROR_IO_PENDING)     // read not delayed?
				// Error in communications; report it.
			{
			}
			else
			{
				fWaitingOnRead = TRUE;
			}
		}
		else 
		{    
			// read completed immediately
			HandleASuccessfulRead(lpBuf, dwRead);
			fWaitingOnRead = FALSE;
		}
	}
	DWORD dwRes;

	if (fWaitingOnRead) {

		dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
		switch(dwRes)
		{
			// Read completed.
		case WAIT_OBJECT_0:
			if (!GetOverlappedResult(hComm, &osReader, &dwRead, FALSE))
				// Error in communications; report it.
			{
			}
			else
				// Read completed successfully.
			{
				HandleASuccessfulRead(lpBuf, dwRead);

				//  Reset flag so that another opertion can be issued.
				fWaitingOnRead = FALSE;
			}
			break;

		case WAIT_TIMEOUT:
			// Operation isn't complete yet. fWaitingOnRead flag isn't
			// changed since I'll loop back around, and I don't want
			// to issue another read until the first one finishes.
			//
			// This is a good time to do some background work.
			fWaitingOnRead = TRUE;
			break;                       

		default:
			// Error in the WaitForSingleObject; abort.
			// This indicates a problem with the OVERLAPPED structure's
			// event handle.
			break;
		}
	}
	return fWaitingOnRead;
}
BOOL WriteABuffer(HANDLE hComm,unsigned char * lpBuf, DWORD dwToWrite)
{
   OVERLAPPED osWrite = {0};
   DWORD dwWritten;
   DWORD dwRes;
   BOOL fRes;

   // Create this write operation's OVERLAPPED structure's hEvent.
   osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
   if (osWrite.hEvent == NULL)
      // error creating overlapped event handle
   {
      return FALSE;
   }
   // Issue write.
   if (!WriteFile(hComm, lpBuf, dwToWrite, &dwWritten, &osWrite))
   {
      if (GetLastError() != ERROR_IO_PENDING) 
	  { 
         // WriteFile failed, but isn't delayed. Report error and abort.
         fRes = FALSE;
      }
      else
	  {
         // Write is pending.

		  dwRes = WaitForSingleObject(osWrite.hEvent, INFINITE);
         switch(dwRes)
         {
            // OVERLAPPED structure's event has been signaled. 
            case WAIT_OBJECT_0:
                 if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, FALSE))
                       fRes = FALSE;
                 else
                  // Write operation completed successfully.
                  fRes = TRUE;
                 break;
            
            default:
                 // An error has occurred in WaitForSingleObject.
                 // This usually indicates a problem with the
                // OVERLAPPED structure's event handle.
                 fRes = FALSE;
                 break;
		 }
      }
   }
   else
      // WriteFile completed immediately.
   {
      fRes = TRUE;
   }

   CloseHandle(osWrite.hEvent);
   return fRes;
}
int _tmain(int argc, _TCHAR* argv[])
{
	PDATA paramr = (PDATA)malloc(sizeof(data));
	PDATA paramw = (PDATA)malloc(sizeof(data));
	DWORD threadidr,threadidw;
	HANDLE threadhandler,threadhandlew;
	HANDLE hComm;
	char gszPort[] = "COM4";
	hComm = CreateFile( gszPort,  GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0);
	if (hComm == INVALID_HANDLE_VALUE)
	{
		printf("error opening port; abort\n");
		getchar();
		return FALSE;
	}
	DCB dcb;
	FillMemory(&dcb, sizeof(dcb), 0);
	dcb.DCBlength = sizeof(dcb);
	if (!BuildCommDCB("115200,n,8,1", &dcb)) {   
		getchar();
		return FALSE;
	}
	else
	{	
	}
	if (!SetCommState(hComm, &dcb))
	{
		printf("Error in SetCommState. Possibly a problem with the communications\n");
		printf("port handle or a problem with the DCB structure itself.\n");
		getchar();
		return FALSE;
	}
	paramr->flag = 1;
	paramw->flag = 1;
	paramr->serailport = hComm;
	paramw->serailport = hComm;

	threadhandler = CreateThread(NULL,0,funr,paramr,0,&threadidr);
	threadhandlew = CreateThread(NULL,0,funw,paramw,0,&threadidw);

	//Sleep(2000);
	//paramr->flag = 0;
	//Sleep(2000);
	//paramw->flag = 0;
	WaitForSingleObject(threadhandler,INFINITE);
	WaitForSingleObject(threadhandlew,INFINITE);
	CloseHandle(hComm);
	getchar();
	return 0;
}