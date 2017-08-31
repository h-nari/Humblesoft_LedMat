#ifndef _bnh_header_h
#define _bnh_header_h

#define BNH_SIGNATURE "Humblesoft/BNH"
#define BNH_FLAG_ENCRYPTED 1
#define BNH_ADDR_MIN	0x9d008000
#define BNH_ADDR_MAX	0x9d03ffff

typedef struct _bnh_header_s {
  char 	signature[16];		// Humblesoft BNH
  u8   	version_major;
  u8   	version_minor;
  u8   	flags;
  u8	option;
  u32	salt;
  u32	addr_min;
  u32	addr_max;
} bnh_header_t;



#endif /* _bnh_header_h */
