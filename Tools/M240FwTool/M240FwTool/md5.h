#ifndef __M240FwTool__md5__
#define __M240FwTool__md5__

#include <stdint.h>

void get_md5(uint8_t* in_buf, uint32_t in_size, uint8_t* md5);

bool check_md5(uint8_t* in_buf, uint32_t in_size, uint8_t* md5);

#endif /* defined(__M240FwTool__md5__) */
