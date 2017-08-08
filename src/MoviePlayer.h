#ifndef _movie_player_h_
#define _movie_player_h_

#include <SD.h>
#include "Humblesoft_LedMat.h"

/*
 * Humblesoft Led Movie format
 */

typedef struct {
  char signature[4];		/* HLM */
  uint16_t size;		/* この構造体のサイズ */
  uint16_t version;		/* タイプ */
  uint16_t type;		/* タイプ */
  uint16_t width;
  uint16_t height;
  uint16_t fps_numerator;
  uint16_t fps_denominator;
  uint32_t frames;		/* フレーム数 */
  uint32_t flags;
  char led_conf[8];		/* LED構成名 */
  uint32_t frame_size;		/* 1フレームデータのバイト数　　　 */	
  uint32_t frame_start;		/* 最初のフレームデータのオフセット */
  uint32_t frame_offset;	/* フレームごとのオフセット	　　　 */	
} hlm_header_t;


class MoviePlayer {
 protected:
  File     	m_file;
  hlm_header_t 	m_header;
  unsigned long m_start;

 public:
  MoviePlayer();
  bool begin(const char *path);
  bool update();
};

#endif /* _movie_player_h_ */
