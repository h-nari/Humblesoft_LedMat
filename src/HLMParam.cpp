#include "Humblesoft_LedMat.h"

HLMParam::HLMParam()
{
  set(LMMT32x32s16, 1, 1, LT_Turnback);
}

uint16_t HLMParam::getWidth() const
{
  return m_moduleWidth * m_moduleCol;
}

uint16_t HLMParam::getHeight()const
{
  return m_moduleHeight * m_moduleRow;
}


bool HLMParam::set(LMModuleType mtype, uint8_t col, uint8_t row,
		   LMLayoutType lt)
{
  m_moduleRow = row;
  m_moduleCol = col;
  m_layoutType = lt;
  uint8_t w,h,s;
  switch(mtype){
  case LMMT32x16s8:  w = 32, h = 16, s =  8; break;
  case LMMT32x32s16: w = 32, h = 32, s = 16; break;
  case LMMT64x32s16: w = 64, h = 32, s = 16; break;
  default:
    return false;
  }
  if(col * w > HLM_WIDTH_MAX || row * h > HLM_HEIGHT_MAX)
    return false;
  m_moduleWidth  = w;
  m_moduleHeight = h;
  m_scan         = s;
  return true;
}

uint32_t HLMParam::bufWidth() const	// 1行換算幅
{
  return m_moduleRow * m_moduleCol * m_moduleWidth;
}

uint32_t HLMParam::planeSize()	const // 1planeのbyte数
{
  return bufWidth() * m_scan;
}

bool HLMParam::getPixelPos(int16_t x, int16_t y,
			   uint32_t *pOff, uint8_t *pShift) const
{
  int16_t w = m_moduleWidth  * m_moduleCol;
  int16_t h = m_moduleHeight * m_moduleRow;
  
  if(x < 0 || y < 0 || x >= w || y >= h) return false;
  
  uint32_t off = 0;
  int8_t shift = 0, yy, sc;
  int16_t hh = m_moduleHeight / 2;
  int16_t rc = hh  / m_scan;
  int16_t ww = w * rc;
  int16_t www = ww * m_moduleRow;
  
  if(m_layoutType == LT_Normal){
    off = x;
    yy = y % m_moduleHeight;
    if(yy >= hh){
      yy -= hh;
      shift = LM_SHIFT;		
    }
    sc = yy / m_scan;
    off += sc * w;
    off += (y / m_moduleHeight) * ww;
    
    yy = yy % m_scan;
    off += yy * www;
  }
  else if(m_layoutType == LT_Turnback){
    int8_t turn = ((h - y - 1) / m_moduleHeight) & 1;
    
    off = turn ? m_moduleWidth - x - 1 : x;
    yy = y % m_moduleHeight;
    if(turn) yy = m_moduleHeight - yy - 1;
    if(yy >= hh){
      yy -= hh;
      shift = LM_SHIFT;		
    }
    sc = yy / m_scan;
    off += sc * w;
    off += (y / m_moduleHeight) * ww;
    
    yy = yy % m_scan;
    off += yy * www;
  }
  else {
    Serial.printf("Bad layoutType:%d\n", m_layoutType); 
    return false;
  }

  // Serial.printf("off:%u shift:%d\n",off,shift);
  if(pOff)   *pOff = off;
  if(pShift) *pShift = shift;
  return true;
}
  
