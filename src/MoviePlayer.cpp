#include "MoviePlayer.h"

MoviePlayer::MoviePlayer()
{
}

bool MoviePlayer::begin(const char *path)
{
  m_file = SD.open(path);
  if(!m_file)
    return LedMat.error("%s not found.\n",path);
  int n = m_file.read((uint8_t *)&m_header, sizeof(m_header));
  if(n != sizeof(m_header))
    return LedMat.error("hlm read header failed. (%d)",n);
  
  if(m_header.width != LedMat.width() || m_header.height != LedMat.height())
    return LedMat.error("hlm size error");
  m_file.seek(m_header.frame_start);
  m_start = micros();
    
  return true;
}

bool MoviePlayer::update()
{
  if(!m_file)
    return false;
  
  unsigned long now = micros();
  uint16_t fps_n = m_header.fps_numerator;
  uint16_t fps_d = m_header.fps_denominator;
  uint16_t f = (now - m_start) * fps_n / fps_d / 1000000;

  // Serial.printf("%luus f:%u fps_n:%u fps_d:%u\n",now-m_start,f,fps_n,fps_d);
  
  if(f > 1) {
    uint32_t pos = m_file.position();
    pos += (f - 1)* m_header.frame_offset;
    m_file.seek(pos);
  }
  if(f > 0) {
    uint32_t bufLen;
    uint8_t  *p = LedMat.getImgBuf(&bufLen);

    uint32_t dataLen = m_header.width * m_header.height / 2 * LedMat.getPlane();
    if(dataLen > bufLen) dataLen = bufLen;
    uint32_t skipLen = m_header.frame_offset - dataLen;
    
    while(dataLen > 0){
      int n = m_file.read(p, dataLen);
      if(n < 0) return LedMat.error("hlm file read error");
      if(n == 0){
	m_file.close();
	return false;
      }
      dataLen -= n;
    }
	
    if(skipLen > 0)
      m_file.seek(m_file.position() + skipLen);
    LedMat.display();

    m_start += f * 1000000 * fps_d / fps_n;
  }
  return true;
}
