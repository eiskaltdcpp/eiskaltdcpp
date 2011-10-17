#ifndef MEDIAINFO_H
#define MEDIAINFO_H

#include <string>

struct MediaInfo{
  std::string video_info;
  std::string audio_info;
  std::string resolution;
  uint16_t bitrate;
};

#endif // MEDIAINFO_H
