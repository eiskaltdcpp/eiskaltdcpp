/*
 * utility.cpp
 *
 *  Created on: 17.08.2009
 *      Author: alex
 */

//---------------------------------------------------------------------------
#include "stdafx.h"
#include "dcpp/DCPlusPlus.h"
//---------------------------------------------------------------------------

string PATH = "", sTitle = "";

bool DirExist(char * sPath) {
#ifdef _WIN32
	DWORD code = GetFileAttributes(sPath);
	if(code != INVALID_FILE_ATTRIBUTES && code == FILE_ATTRIBUTE_DIRECTORY) {
#else
    struct stat st;
	if(stat(sPath, &st) == 0 && S_ISDIR(st.st_mode)) {
#endif
		return true;
	}

	return false;
}

void AppendSpecialLog(const string & sData) {
#ifdef _WIN32
	FILE * fw = fopen((PATH + "\\logs\\debug.log").c_str(), "a");
#else
	FILE * fw = fopen((PATH + "/Logs/debug.log").c_str(), "a");
#endif

	if(fw == NULL) {
		return;
	}

	time_t acc_time;
	time(&acc_time);

	struct tm * acc_tm;
	acc_tm = localtime(&acc_time);

	char sBuf[64];
	strftime(sBuf, 64, "%d.%m.%Y %H:%M:%S", acc_tm);

	string sTmp = string(sBuf) + " - " + sData + "\n";

	fprintf(fw, sTmp.c_str());

	fclose(fw);
}

//---------------------------------------------------------------------------
