#include "Utils.h"

#include <string>
#include <locale>
#include <iostream>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Util.h"
#include "dcpp/Text.h"

using namespace Wt;

std::map<WString,WString> Utils::fileTypes = std::map<WString,WString>();

void Utils::init(){
    // MP3 2
    fileTypes[".A52"]  = "resources/audio-x-generic.png";
    fileTypes[".AAC"]  = "resources/audio-x-generic.png";
    fileTypes[".AC3"]  = "resources/audio-x-generic.png";
    fileTypes[".APE"]  = "resources/audio-x-generic.png";
    fileTypes[".AIFF"] = "resources/audio-x-generic.png";
    fileTypes[".AU"]   = "resources/audio-x-generic.png";
    fileTypes[".DTS"]  = "resources/audio-x-generic.png";
    fileTypes[".FLA"]  = "resources/audio-x-generic.png";
    fileTypes[".FLAC"] = "resources/audio-x-generic.png";
    fileTypes[".MID"]  = "resources/audio-x-generic.png";
    fileTypes[".MOD"]  = "resources/audio-x-generic.png";
    fileTypes[".M4A"]  = "resources/audio-x-generic.png";
    fileTypes[".M4P"]  = "resources/audio-x-generic.png";
    fileTypes[".MPC"]  = "resources/audio-x-generic.png";
    fileTypes[".MP1"]  = "resources/audio-x-generic.png";
    fileTypes[".MP2"]  = "resources/audio-x-generic.png";
    fileTypes[".MP3"]  = "resources/audio-x-generic.png";
    fileTypes[".OGG"]  = "resources/audio-x-generic.png";
    fileTypes[".RA"]   = "resources/audio-x-generic.png";
    fileTypes[".SHN"]  = "resources/audio-x-generic.png";
    fileTypes[".SPX"]  = "resources/audio-x-generic.png";
    fileTypes[".WAV"]  = "resources/audio-x-generic.png";
    fileTypes[".WMA"]  = "resources/audio-x-generic.png";
    fileTypes[".WV"]   = "resources/audio-x-generic.png";

    // ARCHIVE 3
    fileTypes[".7Z"]  = "resources/application-x-archive.png";
    fileTypes[".ACE"] = "resources/application-x-archive.png";
    fileTypes[".BZ2"] = "resources/application-x-archive.png";
    fileTypes[".CAB"] = "resources/application-x-archive.png";
    fileTypes[".EX_"] = "resources/application-x-archive.png";
    fileTypes[".GZ"]  = "resources/application-x-archive.png";
    fileTypes[".JAR"] = "resources/application-x-archive.png";
    fileTypes[".LZH"] = "resources/application-x-archive.png";
    fileTypes[".RAR"] = "resources/application-x-archive.png";
    fileTypes[".RPM"] = "resources/application-x-archive.png";
    fileTypes[".TAR"] = "resources/application-x-archive.png";
    fileTypes[".TGZ"] = "resources/application-x-archive.png";
    fileTypes[".ZIP"] = "resources/application-x-archive.png";
    fileTypes[".ZOO"] = "resources/application-x-archive.png";
    fileTypes[".Z"]   = "resources/application-x-archive.png";

    // DOCUMENT 4
    fileTypes[".CFG"]   = "resources/text-x-generic.png";
    fileTypes[".CONF"]  = "resources/text-x-generic.png";
    fileTypes[".CPP"]   = "resources/text-x-generic.png";
    fileTypes[".CSS"]   = "resources/text-x-generic.png";
    fileTypes[".C"]     = "resources/text-x-generic.png";
    fileTypes[".DIZ"]   = "resources/text-x-generic.png";
    fileTypes[".DOC"]   = "resources/text-x-generic.png";
    fileTypes[".DOCX"]  = "resources/text-x-generic.png";
    fileTypes[".H"]     = "resources/text-x-generic.png";
    fileTypes[".HLP"]   = "resources/text-x-generic.png";
    fileTypes[".HTM"]   = "resources/text-x-generic.png";
    fileTypes[".HTML"]  = "resources/text-x-generic.png";
    fileTypes[".INI"]   = "resources/text-x-generic.png";
    fileTypes[".INF"]   = "resources/text-x-generic.png";
    fileTypes[".LOG"]   = "resources/text-x-generic.png";
    fileTypes[".NFO"]   = "resources/text-x-generic.png";
    fileTypes[".ODG"]   = "resources/text-x-generic.png";
    fileTypes[".ODP"]   = "resources/text-x-generic.png";
    fileTypes[".ODS"]   = "resources/text-x-generic.png";
    fileTypes[".ODT"]   = "resources/text-x-generic.png";
    fileTypes[".PDF"]   = "resources/text-x-generic.png";
    fileTypes[".PHP"]   = "resources/text-x-generic.png";
    fileTypes[".PPT"]   = "resources/text-x-generic.png";
    fileTypes[".PS"]    = "resources/text-x-generic.png";
    fileTypes[".PDF"]   = "resources/text-x-generic.png";
    fileTypes[".SHTML"] = "resources/text-x-generic.png";
    fileTypes[".SXC"]   = "resources/text-x-generic.png";
    fileTypes[".SXD"]   = "resources/text-x-generic.png";
    fileTypes[".SXI"]   = "resources/text-x-generic.png";
    fileTypes[".SXW"]   = "resources/text-x-generic.png";
    fileTypes[".TXT"]   = "resources/text-x-generic.png";
    fileTypes[".RFT"]   = "resources/text-x-generic.png";
    fileTypes[".RDF"]   = "resources/text-x-generic.png";
    fileTypes[".XML"]   = "resources/text-x-generic.png";
    fileTypes[".XLS"]   = "resources/text-x-generic.png";

    // APPL 5
    fileTypes[".BAT"] = "resources/application-x-executable.png";
    fileTypes[".CGI"] = "resources/application-x-executable.png";
    fileTypes[".COM"] = "resources/application-x-executable.png";
    fileTypes[".DLL"] = "resources/application-x-executable.png";
    fileTypes[".EXE"] = "resources/application-x-executable.png";
    fileTypes[".HQX"] = "resources/application-x-executable.png";
    fileTypes[".JS"]  = "resources/application-x-executable.png";
    fileTypes[".SH"]  = "resources/application-x-executable.png";
    fileTypes[".SO"]  = "resources/application-x-executable.png";
    fileTypes[".SYS"] = "resources/application-x-executable.png";
    fileTypes[".VXD"] = "resources/application-x-executable.png";

    // PICTURE 6
    fileTypes[".3DS"]  = "resources/image-x-generic.png";
    fileTypes[".A11"]  = "resources/image-x-generic.png";
    fileTypes[".ACB"]  = "resources/image-x-generic.png";
    fileTypes[".ADC"]  = "resources/image-x-generic.png";
    fileTypes[".ADI"]  = "resources/image-x-generic.png";
    fileTypes[".AFI"]  = "resources/image-x-generic.png";
    fileTypes[".AI"]   = "resources/image-x-generic.png";
    fileTypes[".AIS"]  = "resources/image-x-generic.png";
    fileTypes[".ANS"]  = "resources/image-x-generic.png";
    fileTypes[".ART"]  = "resources/image-x-generic.png";
    fileTypes[".B8"]   = "resources/image-x-generic.png";
    fileTypes[".BMP"]  = "resources/image-x-generic.png";
    fileTypes[".CBM"]  = "resources/image-x-generic.png";
    fileTypes[".EPS"]  = "resources/image-x-generic.png";
    fileTypes[".GIF"]  = "resources/image-x-generic.png";
    fileTypes[".ICO"]  = "resources/image-x-generic.png";
    fileTypes[".IMG"]  = "resources/image-x-generic.png";
    fileTypes[".JPEG"] = "resources/image-x-generic.png";
    fileTypes[".JPG"]  = "resources/image-x-generic.png";
    fileTypes[".PCT"]  = "resources/image-x-generic.png";
    fileTypes[".PCX"]  = "resources/image-x-generic.png";
    fileTypes[".PIC"]  = "resources/image-x-generic.png";
    fileTypes[".PICT"] = "resources/image-x-generic.png";
    fileTypes[".PNG"]  = "resources/image-x-generic.png";
    fileTypes[".PS"]   = "resources/image-x-generic.png";
    fileTypes[".PSP"]  = "resources/image-x-generic.png";
    fileTypes[".RLE"]  = "resources/image-x-generic.png";
    fileTypes[".TGA"]  = "resources/image-x-generic.png";
    fileTypes[".TIF"]  = "resources/image-x-generic.png";
    fileTypes[".TIFF"] = "resources/image-x-generic.png";
    fileTypes[".XPM"]  = "resources/image-x-generic.png";

    // VIDEO 7
    fileTypes[".AVI"]   = "resources/video-x-generic.png";
    fileTypes[".ASF"]   = "resources/video-x-generic.png";
    fileTypes[".ASX"]   = "resources/video-x-generic.png";
    fileTypes[".DAT"]   = "resources/video-x-generic.png";
    fileTypes[".DIVX"]  = "resources/video-x-generic.png";
    fileTypes[".DV"]    = "resources/video-x-generic.png";
    fileTypes[".FLV"]   = "resources/video-x-generic.png";
    fileTypes[".M1V"]   = "resources/video-x-generic.png";
    fileTypes[".M2V"]   = "resources/video-x-generic.png";
    fileTypes[".M4V"]   = "resources/video-x-generic.png";
    fileTypes[".MKV"]   = "resources/video-x-generic.png";
    fileTypes[".MOV"]   = "resources/video-x-generic.png";
    fileTypes[".MOVIE"] = "resources/video-x-generic.png";
    fileTypes[".MP4"]   = "resources/video-x-generic.png";
    fileTypes[".MPEG"]  = "resources/video-x-generic.png";
    fileTypes[".MPEG1"] = "resources/video-x-generic.png";
    fileTypes[".MPEG2"] = "resources/video-x-generic.png";
    fileTypes[".MPEG4"] = "resources/video-x-generic.png";
    fileTypes[".MPG"]   = "resources/video-x-generic.png";
    fileTypes[".OGM"]   = "resources/video-x-generic.png";
    fileTypes[".PXP"]   = "resources/video-x-generic.png";
    fileTypes[".QT"]    = "resources/video-x-generic.png";
    fileTypes[".RM"]    = "resources/video-x-generic.png";
    fileTypes[".RMVB"]  = "resources/video-x-generic.png";
    fileTypes[".VIV"]   = "resources/video-x-generic.png";
    fileTypes[".VOB"]   = "resources/video-x-generic.png";
    fileTypes[".WMV"]   = "resources/video-x-generic.png";
}

WString Utils::formatBytes(long long aBytes){
    return WString::fromUTF8(dcpp::Util::formatBytes(aBytes), false);
}

WString Utils::getFileImage(const Wt::WString &file){
    std::string ext = dcpp::Util::getFileExt(file.toUTF8());

    std::transform(ext.begin(), ext.end(), ext.begin(), (int (*)(int))std::toupper);

    if (fileTypes.find(ext) != fileTypes.end())
        return ("/"+fileTypes[ext]);
    else
        return ("/resources/unknown.png");
}
