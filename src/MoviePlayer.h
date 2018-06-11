#ifndef _movie_player_h_
#define _movie_player_h_

#include <SD.h>
#include "Humblesoft_LedMat.h"
#include "hlm.h"

class MoviePlayer {
 protected:
  File     	m_file;
  hlm3_header_t m_header;
  unsigned long m_start;	// 最後のframeの表示予定時刻 [usec]
  bool          m_vEndFrame;
  uint32_t      m_endFrame;
  uint32_t	m_cFrame;	// frame count
  uint32_t	m_cSkip;	// skipされたフレーム数
  float         m_fSpeed;
  
 public:
  MoviePlayer();
  bool begin(const char *path);
  bool update();
  void close();
  hlm3_header_t *header() { return &m_header;}
  bool seek(float fTime);
  bool seek(uint32_t frame); 
  bool setEnd(float fTime);
  bool setEnd(uint32_t frame);
  void setSpeed(float fSpeed);
};

#endif /* _movie_player_h_ */
