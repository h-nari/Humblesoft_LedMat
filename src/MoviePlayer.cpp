#include "MoviePlayer.h"

MoviePlayer::MoviePlayer()
{
  m_fSpeed = 1.0f;
  
}

bool MoviePlayer::begin(const char *path)
{
  m_file = SD.open(path);
  if(!m_file)
    return LedMat.error("%s not found.\n",path);
  int n = m_file.read((uint8_t *)&m_header, sizeof(m_header));
  if(n != sizeof(m_header)) goto error;

  if(strcmp(m_header.signature,"HLM")!=0) goto error;
  if(m_header.version != 3) goto error;
  if(m_header.width != LedMat.width() || m_header.height != LedMat.height())
    goto error;
  if(m_header.scan != LedMat.scan()) goto error;
  // m_file.seek(m_header.frame_start);
  seek(0U);
  m_vEndFrame = false;		// m_endFrame無効
  m_cSkip = 0;
  
  return true;
 error:
  m_file.close();
  return LedMat.error("header error");
}

bool MoviePlayer::update()
{
  if(!m_file)
    return false;
  
  unsigned long now = micros();
  uint16_t fps_n = m_header.fps_numerator;
  uint16_t fps_d = m_header.fps_denominator;
  uint16_t f = (now - m_start) * m_fSpeed * fps_n / fps_d / 1000000; 

  // Serial.printf("%luus f:%u fps_n:%u fps_d:%u\n",now-m_start,f,fps_n,fps_d);
  
  if(f > 1) {
    uint32_t pos = m_file.position();
    pos += (f - 1)* m_header.frame_offset;
    m_file.seek(pos);
    m_cSkip += f - 1;
  }
  if(f > 0) {
    uint32_t bufLen;
    uint8_t  *p = LedMat.getImgBuf(&bufLen);

    uint32_t dataLen = m_header.width * m_header.height/2 * LedMat.getPlane();
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

    m_start += f * 1000000 * fps_d / fps_n / m_fSpeed;
  }
  m_cFrame += f;

  if(m_vEndFrame && m_cFrame >= m_endFrame)
    return false;
  else
    return true;
}

void MoviePlayer::close()
{
  if(m_file) m_file.close();
}

bool MoviePlayer::seek(float fTime)
{
  int n = m_header.fps_numerator;
  int d = m_header.fps_denominator; 
  uint32_t f = (int)(fTime * n / d + 0.5);
  
  return seek(f);
}

bool MoviePlayer::seek(uint32_t frame)
{
  uint32_t pos = m_header.frame_start + m_header.frame_offset * frame;
  m_cFrame = frame;
  m_start = micros();
  
  return m_file.seek(pos);
}

bool MoviePlayer::setEnd(float fTime)
{
  int n = m_header.fps_numerator;
  int d = m_header.fps_denominator; 
  uint32_t f = (int)(fTime * n / d + 0.5);
  return setEnd(f);
}

bool MoviePlayer::setEnd(uint32_t frame)
{
  
  m_vEndFrame = true;
  m_endFrame = frame;
  return true;
}

void MoviePlayer::setSpeed(float fSpeed)
{
  m_fSpeed = fSpeed;
}

