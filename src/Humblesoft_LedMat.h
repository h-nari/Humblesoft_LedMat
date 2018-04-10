#ifndef _humblesoft_ledmat_h_
#define _humblesoft_ledmat_h_

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Humblesoft_GFX.h>
#include <SD.h>

#define LM_CS0		2
#define LM_CS1		15
#define LM_BUSY		4

#define MCLR_PIN	 5
#define SD_CS		16


#define LM_CMD_TIMEOUT		10	/* mS */
#define LM_BOOTBACK_TIMEOUT	500	/* mS */
#define LM_FIRMDATA_TIMEOUT	100	/* mS */
#define LM_FIRMBEGIN_TIMEOUT	2000	/* mS */

class Humblesoft_LedMat;

enum LMLayoutType {
  LT_Normal,
  LT_Turnback,
};

enum LMModuleType {
  LMMT32x16s8,
  LMMT32x32s16,
  LMMT64x32s16,
};

struct lm_response_t {
  uint8_t status;
  uint8_t data_size;
  uint8_t data[12];
};

class HLMParam {
 public:
  uint8_t m_moduleRow;
  uint8_t m_moduleCol;
  uint8_t m_moduleWidth;
  uint8_t m_moduleHeight;
  uint8_t m_scan;
  LMLayoutType m_layoutType;

 public:
  HLMParam();
  bool set(LMModuleType type,uint8_t col,uint8_t row,LMLayoutType=LT_Turnback);
  uint32_t bufWidth() const;		// 1行換算幅
  uint32_t planeSize()const;		// 1planeのbyte数
  uint16_t getWidth() const;
  uint16_t getHeight()const;
  bool getPixelPos(int16_t x, int16_t y, uint32_t *pOff, uint8_t *pShift) const;
};

struct PacketHeader {
  char sig[4];
  uint32_t pid;		// packet id
  uint16_t cmd;
  uint16_t flags;
};


// #ifndef HLM_IMG_BUF_SIZE
// #  define HLM_IMG_BUF_SIZE (12*1024)
// #endif
#define HLM_WIDTH_MAX 		256
#define HLM_HEIGHT_MAX 		256
#define LM_CMD_MAX		 16

#define LM_SHIFT	3

#define HSM_BOOTMODE	(1 << 0)	// bootmode
#define HSM_FMRCVD	(1 << 1)	// firmware recieved
#define HSM_ERROR	(1 << 2)	// error occurred
#define HSM_OVERRUN	(1 << 3)	// data overrun
#define HSM_BOR		(1 << 4)	// Brown Out Reset occurred

class Humblesoft_LedMat : public Humblesoft_GFX {
  enum Param_Id {
    PARAM_FIRM_VER 	= 0x0001,
    PARAM_CAPACITY 	= 0x0010,
    PARAM_BOR      	= 0x0020,
    PARAM_ADDR_GROUNDED = 0x0030,
    PARAM_CONFIG_ID	= 0x0040,
    PARAM_DISP_ON	= 0x0050,
    PARAM_SCAN		= 0x0100,
    PARAM_WIDTH		= 0x0101,
    PARAM_PLANE		= 0x0200,
    PARAM_BRIGHT	= 0x0201,
    PARAM_DISP_PERIOD	= 0x0203,
    PARAM_DISP_OFFSET	= 0x0210,
    PARAM_WRITE_OFFSET	= 0x0300,
    PARAM_WRITE_LENGTH	= 0x0301,
    PARAM_WRITE_PTR	= 0x0302,
  };
  
  // friend class HLMBuf;
  
 protected:
  bool      m_enable;
  bool      m_sdEnable;
  HLMParam  m_param;
  uint8_t   m_cPlane;
  uint8_t   m_bright;
  uint8_t   m_bright_max;
  uint32_t  m_imgBufSize;
  uint8_t   *m_imgBuf;
  uint8_t  *m_pAllocated;
  int       m_ci;
  int	    m_configId;	
  uint8_t   m_cmdBuf[LM_CMD_MAX];
  bool      m_bGamma;
  float	    m_fGamma;
  uint8_t   m_ledbit_r;	
  uint8_t   m_ledbit_g;	
  uint8_t   m_ledbit_b;	
  uint8_t   m_aGamma[256];

  bool      m_bClippingArea;
  int16_t   m_cx0, m_cy0, m_cx1, m_cy1;		// Clipping area

  int       m_verbose;
  // volatile bool m_busy;
  
 public:
  Humblesoft_LedMat();
  bool begin(LMModuleType mtype=LMMT64x32s16, uint8_t col =1, uint8_t row=1,
	     LMLayoutType lt=LT_Normal);
  void setImgBuf(uint8_t *buf, uint32_t length);
  uint8_t *getImgBuf(uint32_t *pLength = NULL);
  void resetSubcon();
  bool checkSubcon();
  
  bool setConf(LMModuleType mtype, uint8_t col=1, uint8_t row=1,
	       LMLayoutType lt=LT_Normal);
  void setPlane(uint8_t cPlane);
  uint8_t getPlane() { return m_cPlane;}
  bool enableGammaCorrection(bool b) {
    bool b0 = m_bGamma;
    m_bGamma = b;
    return b0;
  }
  void setGamma(float fGamma);
  void display();
  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  void drawPixel24(int16_t x, int16_t y, uint32_t color);
  void drawPixelRGB(int16_t x, int16_t y,
		    uint8_t r, uint8_t g, uint8_t b);
  uint32_t getPixel24(int16_t x, int16_t y);
  bool readSubconStatus(uint8_t *status, uint32_t *pFver=NULL,
			uint32_t *pConfigId = NULL);
  bool readSubconParam(Param_Id id, uint32_t *value);
  bool writeSubconParam(Param_Id id, uint32_t value, int length = 4);
  bool setDataPos(uint32_t pos);
  void writeData(uint8_t *data, uint32_t length);
  
  bool firmwareBegin(uint32_t salt, bool bEncrypt,
		      uint32_t addr_min, uint32_t addr_max);
  bool firmwareData(const uint8_t *data, uint16_t length);
  bool firmwareEnd();
  
  void setVerbose(int v){m_verbose = v;}
  uint8_t setBright(uint8_t bright);
  uint8_t getBright();
  void addBright(int d);

  int16_t height(void) const;
  int16_t width(void) const;
  uint16_t colorHSV(long hue, uint8_t sat, uint8_t val);
  void shiftLeft(int16_t x,int16_t y, uint16_t w, uint16_t h, int16_t shift);
  void setLedBits(uint8_t r, uint8_t g, uint8_t b) {
    m_ledbit_r = r;
    m_ledbit_g = g;
    m_ledbit_b = b;
  }
  bool error(const char *fmt, ...);
  void setLedMode(uint8_t mode);
  bool sdEnable() {return m_sdEnable;};
  void setClippingArea(int16_t x, int16_t y, uint16_t w, uint16_t h){
    m_bClippingArea = true;
    m_cx0 = x;
    m_cy0 = y;
    m_cx1 = x + w;
    m_cy1 = y + h;
  }
  
  void unsetClippingArea()
  {
    m_bClippingArea = false;
  }
  
 protected:
  void cmd_init();
  bool cmd_wait_busy(uint32_t ms = LM_CMD_TIMEOUT);
  void cmd_put(uint8_t data);
  void cmd_put16(uint16_t data);
  void cmd_put24(uint32_t data);
  void cmd_put32(uint32_t data);
  void cmd_open(uint8_t cmd);
  void cmd_close();
  bool cmd_send0(uint32_t ms = LM_CMD_TIMEOUT);
  bool cmd_send(lm_response_t *resp = NULL, uint32_t ms = LM_CMD_TIMEOUT);
  bool read_response(lm_response_t *resp);
  uint8_t calc_checksum(uint8_t *buf, int len);
  bool is_response_ok(uint8_t *buf, int *pStart, int *pLen);
  bool set_disp_pos_cmd(uint32_t offset=0);
  bool set_write_pos_cmd(uint32_t offset=0);
  bool write_firmware_cmd(uint32_t salt, uint32_t addr_min,uint32_t addr_max,
			  bool bEncrypt = true );
  bool boot_cmd();
  bool bootback_cmd();
  void write_data(uint8_t *buf, uint32_t length);
  void reply_ack(PacketHeader *h,bool bAck = true, const char *mes=NULL);
  void udp_cmd_leddata(PacketHeader *h);
  void udp_cmd_firm_begin(PacketHeader *h);
  void udp_cmd_firm_end(PacketHeader *h);
  void udp_cmd_firm_data(PacketHeader *h);
  void udp_cmd_firm_status(PacketHeader *h);
  bool resetSubconParams();
  void posRotate(int16_t x, int16_t y, uint8_t r, int16_t *px, int16_t *py);
  void setBusy(bool b);

  inline void spi0_begin() {
    digitalWrite(LM_CS0, LOW);
    SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0)); //20MHz
  }
  inline void spi0_end() {
    SPI.endTransaction();
    digitalWrite(LM_CS0, HIGH);
  }
  inline void spi1_begin() {
    digitalWrite(LM_CS1, LOW);
    SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0)); //20MHz
  }
  inline void spi1_end() {
    digitalWrite(LM_CS1, HIGH);
    digitalWrite(LM_CS0, HIGH);
  }
};

extern Humblesoft_LedMat LedMat;

#endif /* _humblesoft_ledmat_h_ */
