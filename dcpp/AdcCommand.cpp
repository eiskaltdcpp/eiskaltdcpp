/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "AdcCommand.h"
#include "ClientManager.h"

namespace dcpp {

AdcCommand::AdcCommand(uint32_t aCmd, char aType /* = TYPE_CLIENT */) : cmdInt(aCmd), from(0), type(aType) { }
AdcCommand::AdcCommand(uint32_t aCmd, const uint32_t aTarget, char aType) : cmdInt(aCmd), from(0), to(aTarget), type(aType) { }
AdcCommand::AdcCommand(Severity sev, Error err, const string& desc, char aType /* = TYPE_CLIENT */) : cmdInt(CMD_STA), from(0), type(aType) {
	addParam(Util::toString(sev * 100 + err));
	addParam(desc);
}

AdcCommand::AdcCommand(const string& aLine, bool nmdc /* = false */) throw(ParseException) : cmdInt(0), type(TYPE_CLIENT) {
	parse(aLine, nmdc);
}

void AdcCommand::parse(const string& aLine, bool nmdc /* = false */) throw(ParseException) {
	string::size_type i = 5;

	if(nmdc) {
		// "$ADCxxx ..."
		if(aLine.length() < 7)
			throw ParseException("Too short");
		type = TYPE_CLIENT;
		cmd[0] = aLine[4];
		cmd[1] = aLine[5];
		cmd[2] = aLine[6];
		i += 3;
	} else {
		// "yxxx ..."
		if(aLine.length() < 4)
			throw ParseException("Too short");
		type = aLine[0];
		cmd[0] = aLine[1];
		cmd[1] = aLine[2];
		cmd[2] = aLine[3];
	}

	if(type != TYPE_BROADCAST && type != TYPE_CLIENT && type != TYPE_DIRECT && type != TYPE_ECHO && type != TYPE_FEATURE && type != TYPE_INFO && type != TYPE_HUB && type != TYPE_UDP) {
		throw ParseException("Invalid type");
	}

	if(type == TYPE_INFO) {
		from = HUB_SID;
	}

	string::size_type len = aLine.length();
	const char* buf = aLine.c_str();
	string cur;
	cur.reserve(128);

	bool toSet = false;
	bool featureSet = false;
	bool fromSet = nmdc; // $ADCxxx never have a from CID...

	while(i < len) {
		switch(buf[i]) {
		case '\\':
			++i;
			if(i == len)
				throw ParseException("Escape at eol");
			if(buf[i] == 's')
				cur += ' ';
			else if(buf[i] == 'n')
				cur += '\n';
			else if(buf[i] == '\\')
				cur += '\\';
			else if(buf[i] == ' ' && nmdc)	// $ADCGET escaping, leftover from old specs
				cur += ' ';
			else
				throw ParseException("Unknown escape");
			break;
		case ' ':
			// New parameter...
			{
				if((type == TYPE_BROADCAST || type == TYPE_DIRECT || type == TYPE_ECHO || type == TYPE_FEATURE) && !fromSet) {
					if(cur.length() != 4) {
						throw ParseException("Invalid SID length");
					}
					from = toSID(cur);
					fromSet = true;
				} else if((type == TYPE_DIRECT || type == TYPE_ECHO) && !toSet) {
					if(cur.length() != 4) {
						throw ParseException("Invalid SID length");
					}
					to = toSID(cur);
					toSet = true;
				} else if(type == TYPE_FEATURE && !featureSet) {
					if(cur.length() % 5 != 0) {
						throw ParseException("Invalid feature length");
					}
					// Skip...
					featureSet = true;
				} else {
					parameters.push_back(cur);
				}
				cur.clear();
			}
			break;
		default:
			cur += buf[i];
		}
		++i;
	}
	if(!cur.empty()) {
		if((type == TYPE_BROADCAST || type == TYPE_DIRECT || type == TYPE_ECHO || type == TYPE_FEATURE) && !fromSet) {
			if(cur.length() != 4) {
				throw ParseException("Invalid SID length");
			}
			from = toSID(cur);
			fromSet = true;
		} else if((type == TYPE_DIRECT || type == TYPE_ECHO) && !toSet) {
			if(cur.length() != 4) {
				throw ParseException("Invalid SID length");
			}
			to = toSID(cur);
			toSet = true;
		} else if(type == TYPE_FEATURE && !featureSet) {
			if(cur.length() % 5 != 0) {
				throw ParseException("Invalid feature length");
			}
            // Skip...
			featureSet = true;
		} else {
			parameters.push_back(cur);
		}
	}

	if((type == TYPE_BROADCAST || type == TYPE_DIRECT || type == TYPE_ECHO || type == TYPE_FEATURE) && !fromSet) {
		throw ParseException("Missing from_sid");
	}

	if(type == TYPE_FEATURE && !featureSet) {
		throw ParseException("Missing feature");
	}

	if((type == TYPE_DIRECT || type == TYPE_ECHO) && !toSet) {
		throw ParseException("Missing to_sid");
	}
}

string AdcCommand::toString(const CID& aCID) const {
	return getHeaderString(aCID) + getParamString(false);
}

string AdcCommand::toString(uint32_t sid /* = 0 */, bool nmdc /* = false */) const {
	return getHeaderString(sid, nmdc) + getParamString(nmdc);
}

string AdcCommand::escape(const string& str, bool old) {
	string tmp = str;
	string::size_type i = 0;
	while( (i = tmp.find_first_of(" \n\\", i)) != string::npos) {
		if(old) {
			tmp.insert(i, "\\");
		} else {
			switch(tmp[i]) {
				case ' ': tmp.replace(i, 1, "\\s"); break;
				case '\n': tmp.replace(i, 1, "\\n"); break;
				case '\\': tmp.replace(i, 1, "\\\\"); break;
			}
		}
		i+=2;
	}
	return tmp;
}

string AdcCommand::getHeaderString(uint32_t sid, bool nmdc) const {
	string tmp;
	if(nmdc) {
		tmp += "$ADC";
	} else {
		tmp += getType();
	}

	tmp += cmdChar;

	if(type == TYPE_BROADCAST || type == TYPE_DIRECT || type == TYPE_ECHO || type == TYPE_FEATURE) {
		tmp += ' ';
		tmp += fromSID(sid);
	}

	if(type == TYPE_DIRECT || type == TYPE_ECHO) {
		tmp += ' ';
		tmp += fromSID(to);
	}

	if(type == TYPE_FEATURE) {
		tmp += ' ';
		tmp += features;
	}
	return tmp;
}

string AdcCommand::getHeaderString(const CID& cid) const {
	dcassert(type == TYPE_UDP);
	string tmp;

	tmp += getType();
	tmp += cmdChar;
	tmp += ' ';
	tmp += cid.toBase32();
	return tmp;
}

string AdcCommand::getParamString(bool nmdc) const {
	string tmp;
	for(StringIterC i = getParameters().begin(); i != getParameters().end(); ++i) {
		tmp += ' ';
		tmp += escape(*i, nmdc);
	}
	if(nmdc) {
		tmp += '|';
	} else {
		tmp += '\n';
	}
	return tmp;
}

bool AdcCommand::getParam(const char* name, size_t start, string& ret) const {
	for(string::size_type i = start; i < getParameters().size(); ++i) {
		if(toCode(name) == toCode(getParameters()[i].c_str())) {
			ret = getParameters()[i].substr(2);
			return true;
		}
	}
	return false;
}

bool AdcCommand::hasFlag(const char* name, size_t start) const {
	for(string::size_type i = start; i < getParameters().size(); ++i) {
		if(toCode(name) == toCode(getParameters()[i].c_str()) &&
			getParameters()[i].size() == 3 &&
			getParameters()[i][2] == '1')
		{
			return true;
		}
	}
	return false;
}

} // namespace dcpp
