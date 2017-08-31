#include "Humblesoft_LedMat.h"
#include <SPI.h>

Humblesoft_LedMat LedMat;

Humblesoft_LedMat::Humblesoft_LedMat()
  : Humblesoft_GFX(HLM_WIDTH_MAX,HLM_HEIGHT_MAX)
{
  m_enable  = false;
  m_verbose = 0;
  m_cPlane  = 4;
  m_bright  = 16;
  m_bright_max = 100;
  m_configId = 1;
  m_bGamma = true;
  m_imgBuf = NULL;
  m_imgBufSize = 0;
}

int16_t Humblesoft_LedMat::height(void) const {
  return rotation & 1 ? m_param.getWidth() : m_param.getHeight();
}

int16_t Humblesoft_LedMat::width(void) const {
  return rotation & 1 ? m_param.getHeight() : m_param.getWidth();
}


bool Humblesoft_LedMat::begin(LMModuleType mtype, uint8_t col, uint8_t row,
			      LMLayoutType lt)
{
  digitalWrite(LM_CS0, HIGH);
  digitalWrite(LM_CS1, HIGH);
  digitalWrite(SD_CS,  HIGH);
  
  pinMode(MCLR_PIN, INPUT_PULLUP);
  pinMode(LM_CS0, OUTPUT);
  pinMode(LM_CS1, OUTPUT);
  pinMode(SD_CS,  OUTPUT);
  pinMode(LM_BUSY, INPUT_PULLUP);

  cmd_init();	// init busy wait interrupt
  setGamma(3.0f);
  
  SPI.begin();
  if(SD.begin(SD_CS)) {
    Serial.printf("initialize SD Card done.\n");
  } else {
    Serial.printf("initialise SD Card failed.\n");
  }

  m_param.set(mtype, col, row, lt);
  resetSubcon();
  return checkSubcon();
}

bool Humblesoft_LedMat::setConf(LMModuleType mtype, uint8_t col, uint8_t row,
				LMLayoutType lt)
{
  m_param.set(mtype, col, row, lt);
  return resetSubconParams();
}


void Humblesoft_LedMat::setImgBuf(uint8_t *buf, uint32_t length)
{
  m_imgBuf = buf;
  m_imgBufSize = length;
}

uint8_t *Humblesoft_LedMat::getImgBuf(uint32_t *pLength)
{
  if(pLength) *pLength = m_imgBufSize;
  return m_imgBuf;
}


void Humblesoft_LedMat::setPlane(uint8_t cPlane)
{
  m_cPlane = cPlane;
}

void Humblesoft_LedMat::resetSubcon()
{
  Serial.println("Reset LedCon");
  
  m_enable = false;
  digitalWrite(MCLR_PIN, LOW);
  pinMode(MCLR_PIN, OUTPUT);
  delay(10);		
  digitalWrite(MCLR_PIN, HIGH);
  pinMode(MCLR_PIN, INPUT);
  delay(100);
  // Serial.printf("MCLR:%d\n", digitalRead(MCLR_PIN));
}

bool Humblesoft_LedMat::checkSubcon()
{
  uint8_t status;
  uint32_t fver, configId;
  
  m_enable = false;
  
  if(!readSubconStatus(&status, &fver, &configId)){
    Serial.println("readSubconStatus failed.");
    return false;
  }
  
  if(status & HSM_BOOTMODE){
    Serial.println("subcon is in bootmode.");
    return true;
  }

  if(status & HSM_BOR){
    int bright0 = m_bright;
    m_bright /= 2;
    if(m_bright == 0)
      m_bright = 1;
    Serial.printf("BOR detected, bright:%d, change bright_max:%d to %d\n",
		  bright0, m_bright_max, m_bright);
    m_bright_max = m_bright;
  }
  
  if(m_verbose > 1)
    Serial.printf(" FVER: %d-%d-%d\n", fver & 0xff,
		  (fver>>8) & 0xff,(fver >> 16)&0xff);

  if((int)configId != m_configId || (status & HSM_BOR)){
    Serial.printf("m_configId differ (%d:%d),reset subcon params\n",
		  m_configId, (int)configId); 
    resetSubconParams();
  }
  m_enable = true;
  return true;
}



void Humblesoft_LedMat::display()
{
  if(!m_imgBuf)
    Serial.println("imgBuf is NULL");
  else if(!m_enable)
    Serial.println("LedMat not enable");
  else {
    uint32_t length = m_param.planeSize() * m_cPlane;
    if(length > m_imgBufSize) length = m_imgBufSize;
    set_write_pos_cmd();
    
    write_data(m_imgBuf, length);
  }
}

void Humblesoft_LedMat::posRotate(int16_t x, int16_t y, uint8_t r,
				  int16_t *px, int16_t *py)
{
  switch(r & 3){
  case 0:
    *px = x;
    *py = y;
    break;
  case 1:
    *px = m_param.getWidth()  - y - 1;
    *py = x;
    break;
  case 2:
    *px = m_param.getWidth()  - x - 1;
    *py = m_param.getHeight() - y - 1;
    break;
  case 3:
    *px = y;
    *py = m_param.getHeight() - x - 1;
    break;
  }
}

void Humblesoft_LedMat::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  uint8_t r = (color >> 8) & 0xf8;
  uint8_t g = (color >> 3) & 0xfc;
  uint8_t b = (color << 3) & 0xf8;
  drawPixelRGB(x, y, r, g, b);
}

void Humblesoft_LedMat::drawPixel24(int16_t x, int16_t y, uint32_t color)
{
  uint8_t r = color >> 16;
  uint8_t g = color >>  8;
  uint8_t b = color ;
  drawPixelRGB(x, y, r, g, b);
}


void Humblesoft_LedMat::drawPixelRGB(int16_t x0, int16_t y0,
				     uint8_t r0, uint8_t g0, uint8_t b0)
{
  uint32_t offset;
  uint8_t shift, r,g,b;
  int16_t x, y;

  if(m_bGamma){
    r = m_aGamma[r0];
    g = m_aGamma[g0];
    b = m_aGamma[b0];
  } else {
    r = r0;
    g = g0;
    b = b0;
  }

  posRotate(x0, y0, rotation, &x, &y);
  if(m_imgBuf && m_param.getPixelPos(x, y, &offset, &shift)){
#if 0
    Serial.printf("%s:%d x:%d y:%d offset:%u shift:%u rgb:%02x%02x%02x\n",
		  __FUNCTION__,__LINE__,x, y, offset, shift, r,g,b);
#endif
    uint8_t *p = m_imgBuf + offset;
    uint8_t mask = 0x80;
    uint8_t bitR = LEDBIT_R << shift;
    uint8_t bitG = LEDBIT_G << shift;
    uint8_t bitB = LEDBIT_B << shift;

    for(int plane=0; plane < m_cPlane && p < m_imgBuf + m_imgBufSize;
	plane++){
      if(r & mask) *p |=  bitR; else *p &= ~bitR;
      if(g & mask) *p |=  bitG; else *p &= ~bitG;
      if(b & mask) *p |=  bitB; else *p &= ~bitB;
      mask >>= 1;
      p += m_param.planeSize();
    }
  }
}

uint32_t Humblesoft_LedMat::getPixel24(int16_t x0, int16_t y0)
{
  uint32_t pixel = 0;
  uint32_t offset;
  uint8_t shift;
  int16_t x, y;

  posRotate(x0, y0, rotation, &x, &y);
  if(m_imgBuf && m_param.getPixelPos(x, y, &offset, &shift)){
    uint8_t *p = m_imgBuf + offset;
    uint32_t maskR = 0x800000;
    uint32_t maskG = 0x008000;
    uint32_t maskB = 0x000080;
    uint8_t  bitR = LEDBIT_R << shift;
    uint8_t  bitG = LEDBIT_G << shift;
    uint8_t  bitB = LEDBIT_B << shift;
    for(int plane=0; plane < m_cPlane && p < m_imgBuf + sizeof(m_imgBuf);
	plane++){
      uint8_t bits = *p;
      if(bits & bitR) pixel |= maskR;
      if(bits & bitG) pixel |= maskG;
      if(bits & bitB) pixel |= maskB;
      maskR >>= 1;
      maskG >>= 1;
      maskB >>= 1;
      p += m_param.planeSize();
    }
  }
  return pixel;
}

bool Humblesoft_LedMat::setDataPos(uint32_t pos)
{
  return set_write_pos_cmd(pos);
}

void Humblesoft_LedMat::writeData(uint8_t *data, uint32_t length)
{
  if(m_enable){
    set_write_pos_cmd();
    write_data(data, length);
  }
}

bool Humblesoft_LedMat::firmwareBegin(uint32_t salt, bool bEncrypt,
				      uint32_t addr_min, uint32_t addr_max)
{
  uint8_t s;
  if(!readSubconStatus(&s)) return false;
  
  if(!(s & HSM_BOOTMODE)){
    if(!bootback_cmd()) return false;
    delay(1);
  }
  return write_firmware_cmd(salt, addr_min, addr_max, bEncrypt);
}

bool Humblesoft_LedMat::firmwareData(const uint8_t *data, uint16_t len)
{
  setBusy(true);
  digitalWrite(LM_CS1, LOW);
  SPI.write((uint8_t)len);
  SPI.write((uint8_t)(len >> 8));
  SPI.writeBytes((uint8_t *)data, len);
  for(int i=0; i< 1022 - len; i++)
    SPI.write(0);
  digitalWrite(LM_CS1, HIGH);

  if(!cmd_wait_busy(LM_FIRMDATA_TIMEOUT))
    return false;

  lm_response_t resp;
  if(!read_response(&resp))
    return false;
  
  uint8_t s = resp.status;
  if(s != (HSM_BOOTMODE | HSM_FMRCVD)){
    if(m_verbose){
      Serial.printf("firmware_data failed. status=0x%02x\n", s);
      if(~s & HSM_BOOTMODE) Serial.println(" not bootmode");
      if(~s & HSM_FMRCVD)   Serial.println(" not firmware recieved");
      if( s & HSM_ERROR)    Serial.println(" error occured");
      if( s & HSM_OVERRUN)  Serial.println(" data overun"); 
    }
    return false;
  }
  return true;
}

bool Humblesoft_LedMat::firmwareEnd()
{
  resetSubcon();
  return true;
}


bool Humblesoft_LedMat::resetSubconParams()
{
  m_configId++;
  return
    set_write_pos_cmd() &&
    set_disp_pos_cmd() && 
    writeSubconParam(PARAM_BRIGHT, m_bright) &&
    writeSubconParam(PARAM_BOR, 0) &&
    writeSubconParam(PARAM_CONFIG_ID, m_configId) &&
    writeSubconParam(PARAM_DISP_ON, 1) ;
}

uint8_t Humblesoft_LedMat::setBright(uint8_t bright)
{
  uint8_t bright0 = m_bright;

  if(bright > m_bright_max){
    if(m_verbose) 
      Serial.printf("bright %u exceed max value:%u\n" ,bright, m_bright_max);
    bright = m_bright_max;
  }
  
  m_bright = bright;
  if(!writeSubconParam(PARAM_BRIGHT, bright)){
    Serial.println("set bright failed.");
  }
  return bright0;
}

uint8_t Humblesoft_LedMat::getBright()
{
  return m_bright;
}

void Humblesoft_LedMat::addBright(int d)
{
  int b = m_bright + d;
  
  if(b < 1)        b = 1;
  else if(b > 100) b = 100;
  setBright(b);
}

uint16_t Humblesoft_LedMat::colorHSV(long hue, uint8_t sat, uint8_t val)
{
  uint8_t  r, g, b, lo;
  uint16_t s1, v1;

  // Hue
  hue %= 1536;             // -1535 to +1535
  if(hue < 0) hue += 1536; //     0 to +1535
  lo = hue & 255;          // Low byte  = primary/secondary color mix
  switch(hue >> 8) {       // High byte = sextant of colorwheel
    case 0 : r = 255     ; g =  lo     ; b =   0     ; break; // R to Y
    case 1 : r = 255 - lo; g = 255     ; b =   0     ; break; // Y to G
    case 2 : r =   0     ; g = 255     ; b =  lo     ; break; // G to C
    case 3 : r =   0     ; g = 255 - lo; b = 255     ; break; // C to B
    case 4 : r =  lo     ; g =   0     ; b = 255     ; break; // B to M
    default: r = 255     ; g =   0     ; b = 255 - lo; break; // M to R
  }

  // Saturation: add 1 so range is 1 to 256, allowig a quick shift operation
  // on the result rather than a costly divide, while the type upgrade to int
  // avoids repeated type conversions in both directions.
  s1 = sat + 1;
  r  = 255 - (((255 - r) * s1) >> 8);
  g  = 255 - (((255 - g) * s1) >> 8);
  b  = 255 - (((255 - b) * s1) >> 8);

  // Value (brightness) & 16-bit color reduction: similar to above, add 1
  // to allow shifts, and upgrade to int makes other conversions implicit.
  v1 = val + 1;
  r = (r * v1) >> 8; 
  g = (g * v1) >> 8;
  b = (b * v1) >> 8;

  return rgb(r,g,b);
}

void Humblesoft_LedMat::shiftLeft(int16_t xx, int16_t yy,
				  uint16_t w, uint16_t h, int16_t shift)
{
  if(!m_imgBuf) return;
  
  if(shift > 0){
    int16_t x0 = xx;
    int16_t y0 = yy;
    int16_t x1 = xx + w;
    int16_t y1 = yy + h;

    if(x0 < 0) x0 = 0;
    if(y0 < 0) y0 = 0;
    if(x1 > _width)  x1 = _width;
    if(y1 > _height) y1 = _height;

    bool bGamma0 = m_bGamma;
    m_bGamma = false;
    
    uint32_t planeSize = m_param.planeSize();
    
    for(int y=y0; y<y1; y++){
      for(int xs=x0+shift; xs < x1; xs++){
#if 0
	drawPixel24(xs-shift, y, getPixel24(xs, y));
#else
	int xd = xs - shift;
	uint32_t os, od;
	uint8_t  ss, sd;
	if(m_param.getPixelPos(xs, y, &os, &ss) &&
	   m_param.getPixelPos(xd, y, &od, &sd)){
	  uint8_t *ps = m_imgBuf + os;
	  uint8_t *pd = m_imgBuf + od;
	  uint8_t *pe = m_imgBuf + m_imgBufSize;
	  uint8_t mask = (LEDBIT_R|LEDBIT_G|LEDBIT_B) << sd;
	  for(int plane=0; plane < m_cPlane && ps < pe && pd < pe; plane++){
	    if(ss == sd)
	      *pd = (~mask&*pd) | (mask & *ps);
	    else if(ss > sd)
	      *pd = (~mask&*pd) | (mask&(*ps >> (ss - sd)));
	    else 
	      *pd = (~mask&*pd) | (mask&(*ps << (sd - ss)));
	    
	    ps += planeSize;
	    pd += planeSize;
	  }
	}
#endif
      }
    }
    m_bGamma = bGamma0;
  }
}


void Humblesoft_LedMat::setGamma(float fGamma)
{
  m_fGamma = fGamma;
  for(int i=0; i<256; i++){
    m_aGamma[i] = (uint8_t)(pow(i / 255.0f, fGamma) * 255 + 0.5);
    if(m_verbose > 1)
      Serial.printf("gamma[%3d]=%3u\n",i, m_aGamma[i]);
    if(i >0 && m_aGamma[i] == 0) m_aGamma[i] = 1;
  }
}

bool Humblesoft_LedMat::error(const char *fmt, ...)
{
  va_list ap;
  char buf[80];
  
  va_start(ap, fmt);
  vsnprintf(buf, sizeof buf, fmt, ap);
  va_end(ap);

  LedMat.clear();
  LedMat.setTextColor("red");
  LedMat.println("Error:");
  LedMat.setTextColor("green");
  LedMat.println(buf);
  LedMat.display();

  Serial.print("Error:");
  Serial.println(buf);

  delay(1000);
  return false;
}
