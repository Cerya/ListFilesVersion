#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "Version.lib")

BOOL GetFileVersion(char *LibName, 
	DWORD *MajVersion, 
	DWORD *MinVersion, 
	DWORD *BuildNumber, 
	DWORD *RevisionNumber);

void DisplayErrorBox(LPTSTR lpszFunction);

/*
	Usage  : ListFileVersionCheck.exe C:\Windows\System32
	return : Filename & Version
*/

int _tmain(int argc, TCHAR *argv[])
{
	WIN32_FIND_DATA wData;
	LARGE_INTEGER filesize;
	TCHAR szDir[MAX_PATH];
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	DWORD major, minor, build, revision;

	// Check argument else print usage.
	if (argc != 2)
	{
		_tprintf(TEXT("\nUsage: %s <PATH>\n"), argv[0]);
		return (-1);
	}

	// Check that the input path plus 3 is not longer than MAX_PATH.
	// Three characters are for the "\*" plus NULL appended below.

	StringCchLength(argv[1], MAX_PATH, &length_of_arg);

	if (length_of_arg > (MAX_PATH - 3))
	{
		_tprintf(TEXT("\nDirectory path is too long.\n"));
		return (-1);
	}

	_tprintf(TEXT("\nTarget directory is %s\n\n"), argv[1]);

	// Prepare string for use with FindFile functions.  First, copy the
	// string to a buffer, then append '\*' to the directory name.

	StringCchCopy(szDir, MAX_PATH, argv[1]);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

	// Find the first file in the directory.

	hFind = FindFirstFile(szDir, &wData);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		DisplayErrorBox(TEXT("FindFirstFile"));
		return dwError;
	}
	
	// List all the files in the directory with some info about them.
	do
	{
		if (wData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			_tprintf(TEXT("  %s   <DIR>\n"), wData.cFileName);
		}
		else
		{
			filesize.LowPart = wData.nFileSizeLow;
			filesize.HighPart = wData.nFileSizeHigh;
			GetFileVersion(LPSTR(wData.cFileName), &major, &minor, &build, &revision);
		}
	} while (FindNextFile(hFind, &wData) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
	{
		DisplayErrorBox(TEXT("FindFirstFile"));
	}
	FindClose(hFind);
	return dwError;
}

BOOL GetFileVersion(char *LibName, DWORD *MajVersion, DWORD *MinVersion, DWORD *BuildNumber, DWORD *RevisionNumber)
{
	DWORD dwHandle, dwLen;
	UINT BufLen;
	LPTSTR lpData;
	VS_FIXEDFILEINFO *pFileInfo;
	dwLen = GetFileVersionInfoSize(LPWSTR(LibName), &dwHandle);

	if (!dwLen) return FALSE;

	lpData = (LPTSTR)malloc(dwLen);
	if (!lpData) return FALSE;

	if (!GetFileVersionInfo(LPWSTR(LibName), dwHandle, dwLen, lpData))
	{
		free(lpData);
		return FALSE;
	}
	if (VerQueryValue(lpData, _T("\\"), (LPVOID *)&pFileInfo, (PUINT)&BufLen))
	{
		//FIX ME
		DWORD MajVersion = HIWORD(pFileInfo->dwFileVersionMS);
		DWORD MinVersion = LOWORD(pFileInfo->dwFileVersionMS);
		DWORD BuildNumber = HIWORD(pFileInfo->dwFileVersionLS);
		DWORD RevisionNumber = LOWORD(pFileInfo->dwFileVersionLS);
		free(lpData);
		//output Filename and Version
		_tprintf(TEXT("Filename:    %s\n"), LPWSTR(LibName));
		_tprintf(TEXT("Version:     %d.%d.%d.%d\n\n"), MajVersion, MinVersion,
			BuildNumber, RevisionNumber);

		return TRUE;
	}
	free(lpData);
	return false;
}

void DisplayErrorBox(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and clean up
	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40)*sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

