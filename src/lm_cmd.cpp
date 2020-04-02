#include "Humblesoft_LedMat.h"
#include <SPI.h>

#define LM_CMD_HEADER		0xa5
#define LM_RX_MAX		16
#define LM_RX_HEADER		0x5a

#define LMC_SET_DISP		0x10
#define LMC_SET_WRITE		0x20
#define	LMC_STATUS		0x30
#define	LMC_FIRMWARE		0x31
#define	LMC_FIRMWARE_ENC	0x32
#define	LMC_BOOT		0x33
#define	LMC_BOOTBACK		0x34
#define	LMC_READ_PARAM		0x40
#define	LMC_WRITE_PARAM		0x50

volatile static bool subcon_busy;

void mem_dump(const char *title,uint8_t *buf, uint32_t len)
{
  Serial.println(title);
  for(uint32_t i=0; i < len; i+=16){
    Serial.printf("%02x:",i);
    for(uint32_t j=0; i+j<len && j<16; j++)
      Serial.printf(" %02x", buf[i+j]);
    Serial.println();
  }
}


void Humblesoft_LedMat::cmd_put(uint8_t data)
{
  if(m_ci < LM_CMD_MAX - 1)
    m_cmdBuf[m_ci++] = data;
}

void Humblesoft_LedMat::cmd_put16(uint16_t data)
{
  cmd_put(data);
  cmd_put(data >> 8);
}

void Humblesoft_LedMat::cmd_put24(uint32_t data)
{
  cmd_put(data);
  cmd_put(data >> 8);
  cmd_put(data >> 16);
}

void Humblesoft_LedMat::cmd_put32(uint32_t data)
{
  cmd_put(data);
  cmd_put(data >> 8);
  cmd_put(data >> 16);
  cmd_put(data >> 24);
}

uint8_t Humblesoft_LedMat::calc_checksum(uint8_t *buf, int len)
{
  int cs = 0, i;
  for(i=0; i<len; i++) cs += buf[i];
  return cs;
}

void Humblesoft_LedMat::cmd_open(uint8_t cmd)
{
  m_ci = 0;
  cmd_put(LM_CMD_HEADER);
  cmd_put(0);
  cmd_put(cmd);
}

void Humblesoft_LedMat::cmd_close()
{
  if(m_ci < LM_CMD_MAX){
    m_cmdBuf[1] = m_ci+1;
    m_cmdBuf[m_ci] = ~ calc_checksum(m_cmdBuf, m_ci);
    m_ci++;
  }
}


void Humblesoft_LedMat::write_data(uint8_t *buf, uint32_t length)
{
  // Serial.printf("write_data(0x%x, len:%u)\n",(long)buf, length);
  
  spi1_begin();
  SPI.writeBytes(buf, length);
  spi1_end();
}

bool Humblesoft_LedMat::cmd_send0(uint32_t ms)
{
  if(m_verbose > 1)
    mem_dump("cmd_send:",m_cmdBuf, m_ci);
  subcon_busy = true;
  spi0_begin();
  SPI.writeBytes(m_cmdBuf, m_ci);
  spi0_end();
  return cmd_wait_busy(ms);
}

bool Humblesoft_LedMat::cmd_send(lm_response_t *resp, uint32_t ms)
{
  return cmd_send0(ms) && read_response(resp);
}

bool Humblesoft_LedMat::is_response_ok(uint8_t *buf, int *pStart, int *pLen)
{
  int i;
  
  for(i=0; i<LM_RX_MAX-4; i++)
    if(buf[i] == LM_RX_HEADER) goto found;
  if(m_verbose)
    Serial.println("RX_HEADER not found.");
  return false;
  
 found:
  if(calc_checksum(buf+i, buf[i+1])!=0xff){
    if(m_verbose)
      Serial.println("RX checksum error");
    return false;
  }
  if(pStart) *pStart = i;
  if(pLen)   *pLen   = buf[i+1];
  return true;
}

bool Humblesoft_LedMat::read_response(lm_response_t *resp)
{
  int s,len;
  uint8_t obuf[LM_RX_MAX],ibuf[LM_RX_MAX];
  memset(obuf, 0x00, LM_RX_MAX);
  memset(ibuf, 0xff, LM_RX_MAX);

  digitalWrite(LM_CS0, LOW);
  SPI.transferBytes(obuf, ibuf, LM_RX_MAX);
  digitalWrite(LM_CS0, HIGH);
  
  if(!is_response_ok(ibuf, &s, &len)){
    if(m_verbose){
      mem_dump("Bad response:",ibuf, LM_RX_MAX);
      mem_dump("cmd_send:",m_cmdBuf, m_ci);
    }
    return false;
  }

  if(resp){
    resp->status = ibuf[s+2];
    resp->data_size = ibuf[s+1]-4;
    for(uint32_t i=0; i<resp->data_size && i < sizeof(resp->data); i++)
      resp->data[i] = ibuf[s+3+i];
  }
  return true;
}

bool Humblesoft_LedMat::readSubconStatus(uint8_t *status,
					 uint32_t *pFver, uint32_t *pConfigId)
{
  lm_response_t resp;
  uint32_t v;
  int i;
  
  cmd_open(LMC_STATUS);
  cmd_close();
  if(!cmd_send(&resp)){
    return false;
  }
  if(resp.data_size != 0 && resp.data_size != 8){
    Serial.printf("%s:%d data_size:%u\n",__FUNCTION__,__LINE__, resp.data_size);
    return false;
  }

  *status = resp.status;

  if(resp.data_size == 0){
    if(pFver)     *pFver = 0;
    if(pConfigId) *pConfigId = 0;
  } else {
    if(pFver){
      for(i=0,v=0; i<4; i++)
	v |= (resp.data[i] << (i*8));
      *pFver = v;
    }
    if(pConfigId){
      for(i=0,v=0; i<4; i++)
	v |= resp.data[i+4] << (i*8);
      *pConfigId = v;
    }
  }
  return true;
}

bool Humblesoft_LedMat::readSubconParam(Param_Id id, uint32_t *value)
{
  lm_response_t resp;
  
  cmd_open(LMC_READ_PARAM);
  cmd_put16(id);
  cmd_close();
  if(!cmd_send(&resp))
    return false;
  int i,v;
  for(i=v=0; i<resp.data_size; i++)
    v |= resp.data[i] << (i*8);
  if(value) *value = v;
  return true;
}

bool Humblesoft_LedMat::writeSubconParam(Param_Id id,uint32_t value,int length)
{
  int i; 
  
  cmd_open(LMC_WRITE_PARAM);
  cmd_put16(id);
  for(i=0; i<length; i++)
    cmd_put(value >> 8*i);
  cmd_close();
  return cmd_send();
}


bool Humblesoft_LedMat::set_disp_pos_cmd(uint32_t offset)
{
  cmd_open   (LMC_SET_DISP);
  cmd_put    (m_param.m_scan);
  cmd_put16  (m_param.bufWidth());
  cmd_put   (m_cPlane);
  cmd_put24 (offset);
  cmd_close();
  return cmd_send();
}

bool Humblesoft_LedMat::set_write_pos_cmd(uint32_t offset)
{
  cmd_open (LMC_SET_WRITE);
  cmd_put24(offset);
  cmd_put24(m_param.planeSize() * m_cPlane);
  cmd_close();
  return  cmd_send();
}

bool Humblesoft_LedMat::write_firmware_cmd(uint32_t salt, uint32_t addr_min,
					   uint32_t addr_max, bool bEncrypt)
{
  cmd_open(bEncrypt ? LMC_FIRMWARE_ENC : LMC_FIRMWARE);
  cmd_put32(salt);
  cmd_put32(addr_min);
  cmd_put32(addr_max);
  cmd_close();
  return cmd_send(NULL, LM_FIRMBEGIN_TIMEOUT);
}			   

bool Humblesoft_LedMat::boot_cmd()
{
  cmd_open(LMC_BOOT);
  cmd_close();
  return cmd_send();
}

bool Humblesoft_LedMat::bootback_cmd()
{
  cmd_open(LMC_BOOTBACK);
  cmd_close();
#if 0
  subcon_busy = true;
  spi0_begin();
  SPI.writeBytes(m_cmdBuf, m_ci);
  spi0_end();

  bool b = cmd_wait_busy(LM_BOOTBACK_TIMEOUT);
  if(!b)
    Serial.printf("%s:%d timeout\n",__FUNCTION__,__LINE__);
  return b;
#else
  return cmd_send0(LM_BOOTBACK_TIMEOUT);
#endif
}

void Humblesoft_LedMat::setBusy(bool b)
{
  subcon_busy = b;
}


ICACHE_RAM_ATTR static void busy_done()
{
  subcon_busy = false;
}

void Humblesoft_LedMat::cmd_init()
{
  attachInterrupt(digitalPinToInterrupt(LM_BUSY), busy_done, RISING);
}

// #define BUSY_DEBUG

bool Humblesoft_LedMat::cmd_wait_busy(uint32_t ms)
{
#ifdef BUSY_DEBUG
  Serial.printf("%s:%d busy:%d\n",__FUNCTION__,__LINE__, subcon_busy);
#endif
  unsigned long start = millis();
  while(subcon_busy){
    if(millis() - start > ms){
      if(m_verbose){
	mem_dump("CMD TIMEOUT:",m_cmdBuf, m_ci);
      }
#ifdef BUSY_DEBUG
      Serial.printf("%s:%d timeout (%u ms)\n",__FUNCTION__,__LINE__,ms);
#endif
      return false;
    }
    // delay(0);
  }
#ifdef BUSY_DEBUG
  Serial.printf("%s:%d exit (%lu ms)\n",__FUNCTION__,__LINE__,millis()-start);
#endif
  return true;
}

