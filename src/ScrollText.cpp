#include "ScrollText.h"

ScrollText::ScrollText(Humblesoft_LedMat *led) :
  m_bitmap(SC_IMAGE_WIDTH,SC_IMAGE_HEIGHT, m_bitmapBuf, sizeof(m_bitmapBuf))
{
  m_led = led; 
  m_ri = m_gw = m_gh = 0;
  m_scx = m_scy = 0;
  m_scw = led->width();
  m_sch = led->height();
  m_cBlank = 0;
  m_oy  = 0;
  m_xi = 0;
  m_cText = 0xffff;
  m_cBg   = 0;
  m_bitmap.setTextColor(1,0);
  m_tUpdate = 0;
}

void ScrollText::clear()
{
  m_xi = 0;
  m_str = "";
  m_ri = m_gw = 0;
}


size_t ScrollText::write(uint8_t c)
{
  m_str += (char)c;
  m_tUpdate = millis();
  return 1;
}


bool ScrollText::update(bool bInit)
{
  uint16_t draww;			// draw width
  unsigned long now = millis();

  if(bInit || !m_tUpdate){
    m_tUpdate = now;
    return true;
  }
    
  if(!m_period)
    draww = 1;
  else 
    draww = (now - m_tUpdate) / m_period;

  if(draww > 16){
    m_tUpdate = now;
    return true;
  }

  if(m_xi < m_scx)              m_xi = m_scx;
  else if(m_xi > m_scx + m_scw) m_xi = m_scx + m_scw;
  
  while(draww > 0){
    if(m_ri >= m_gw){
      char str[8];
      if(get_a_char(str, sizeof str)){
	int16_t x1,y1;
	uint16_t w, h;
	m_bitmap.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
	m_gw = w + 1;
	m_gh = h + 1;
	m_bitmap.fillRect(0,0,m_gw,m_gh, 0);
	m_bitmap.setCursor(0,0);
	m_bitmap.print(str);
	m_ri = 0;
      } 
    }

    int shift = m_xi + draww - (m_scx + m_scw);
    if(shift > 0){
      int tw = m_gw - m_ri;
      if(tw > 0){
	if(shift > tw) shift = tw;
      } else if(m_cBlank > 0) {
	if(shift > m_cBlank) shift = m_cBlank;
      }
      else return false;

      m_led->shiftLeft(m_scx, m_scy, m_scw, m_sch, shift);
      m_xi -= shift;
    }

    for(int y = 0; y < m_sch; y++)
      m_led->drawPixel(m_xi, y + m_scy, m_cBg);
    if(m_ri < m_gw){
      for(int y = 0; y < m_gh; y++)
	if(m_oy +y >= 0 && m_oy + y < m_sch) {
	  m_led->drawPixel(m_xi, m_scy + m_oy + y,
			   m_bitmap.getPixel(m_ri, y) ? m_cText : m_cBg);
	}
      m_ri++;
    }
    else if(m_cBlank > 0) 
      m_cBlank--;

    draww--;
    m_xi++;
    m_tUpdate += m_period;
  }
  m_led->display();
  
  return true;
}

bool ScrollText::get_a_char(char *buf, size_t buf_len)
{
  if(m_str.length() == 0 || buf_len < 2)
    return false;

  unsigned int i,j;
  char c = m_str[0];
  if((c & 0x80)== 0) {    /* ANK character */
    i = 1;
  } else if((c & 0xc0) != 0xc0){
    Serial.printf("Bad utf8 first character 0x%02x found.\n", c & 0xff);
    m_str.remove(0,1);
    return false;
  }
  else { /* utf-8 character */
    for(i=1; (m_str[i] & 0xc0) == 0x80;i++){
      if(i >= m_str.length()){
	Serial.println("Bad utf-8 string, not end");
	m_str.remove(0);	// remove all
	return false;
      }
    }
	
    if(i >= buf_len-1){
      Serial.printf("utf-8 char length too long:%d ,buf_len:%u\n",i,buf_len);
      m_str.remove(0, i+1);
      return false;
    }
  }
  for(j=0; j<i; j++)
    buf[j] = m_str[j];
  buf[j] = 0;
  m_str.remove(0, i);
  return true;
}

void ScrollText::setScrollArea(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  m_scx = x;
  m_scy = y;
  m_scw = w ? w : m_led->width();
  m_sch = h ? h : m_led->height();
}

void ScrollText::setXPos(int16_t x)
{
  m_xi = x;
}


void ScrollText::setYPos()
{
  int16_t x1,y1;
  uint16_t w, h;
  char str[2] = "A";
  m_bitmap.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);

  m_oy = (m_sch - h + 1)/2;
}

void ScrollText::setYPos(int8_t oy)
{
  m_oy = oy;
}
  
void ScrollText::setTextColor(uint16_t color)
{
  m_cText = color;
}

void ScrollText::setBgColor(uint16_t color)
{
  m_cBg = color;
}

void ScrollText::setTextColor(const char *color)
{
  m_cText = m_led->rgb(color);
}

void ScrollText::setBgColor(const char *color)
{
  m_cBg = m_led->rgb(color);
}


void ScrollText::scrollOut(int16_t count)
{
  m_tUpdate = millis();
  m_xi = m_scx + m_scw;
  if(count)
    m_cBlank = count;
  else
    m_cBlank = m_scw;
}

float ScrollText::setSpeed(float dotPerSec)
{
  float speed0 = m_dotPerSec;
  m_dotPerSec = dotPerSec;
  if(dotPerSec > 0.1)
    m_period = 1000 / dotPerSec;
  else
    m_period = 0;
  return speed0;
}
