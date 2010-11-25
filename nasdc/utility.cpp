/*
 * utility.cpp
 *
 *  Created on: 17.08.2009
 *      Author: alex
 */

//---------------------------------------------------------------------------
#include "stdafx.h"
#include "utility.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Util.h"
#include "dcpp/StringTokenizer.h"
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


bool splitMagnet(const string &magnet, string &name, int64_t &size, string &tth) {
    name = "Unknown";
    size = 0;
    tth = "Unknown";
    string tmp;

    if (!magnet.empty() && magnet.find("urn:tree:tiger")!=string::npos)
        return false;
    
    fprintf(stderr,"split:%s\n",magnet.c_str());
    fflush(stderr);
    tmp = magnet.substr(8);	//magnet:?
    fprintf(stderr,"split:%s\n",tmp.c_str());
    fflush(stderr);
    StringTokenizer<string> st(tmp, "&");
    for (StringIter i = st.getTokens().begin(); i != st.getTokens().end(); ++i) {
	string str;
	str=*i;
	if (str.compare(0, 3, "xt=") == 0)
	    tth=str.substr(3+18);
	else if (str.compare(0, 3, "xl=") == 0)
	    size = Util::toInt64(str.substr(3));
	else if (str.compare(0, 3, "dn=") == 0)
	    name = Util::encodeURI(str.substr(3), true);
    }
    //string::size_type nextpos = 0;
    //for (string::size_type pos = 0; pos < magnet.size(); pos = nextpos + 1)
    //{
        //nextpos = magnet.find('&', pos);
        //if (nextpos == string::npos)
            //nextpos = magnet.size();

        //if (magnet.compare(pos, 3, "xl=") == 0)
            //tth = magnet.substr(magnetSignature.length(), nextpos - magnetSignature.length());
        //else if (magnet.compare(pos, 3, "xl=") == 0)
            //size = Util::toInt64(magnet.substr(pos + 3, nextpos - pos - 3));
        //else if (magnet.compare(pos, 3, "dn=") == 0)
            //name = Util::encodeURI(magnet.substr(pos + 3, nextpos - pos - 3), true);
    //}

    return true;
}
