/*
 * utility.h
 *
 *  Created on: 17.08.2009
 *      Author: alex
 */

#ifndef UTILITY_H_
#define UTILITY_H_
//---------------------------------------------------------------------------

extern string PATH, sTitle;

bool DirExist(char * sPath);
void AppendSpecialLog(const string & sData);
string toString(short val);

#endif /* UTILITY_H_ */
