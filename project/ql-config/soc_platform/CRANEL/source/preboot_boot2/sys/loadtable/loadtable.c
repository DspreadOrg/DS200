#include <string.h>
#include "decompress.h"
#include "bl2_arg.h"
#include "syscall-arom.h"
#include "loadtable.h"

#if AROM_VERSION == 20200730 && \
  (defined(MODULE_FASTBOOT_AROM_PATCH) || \
  defined(MODULE_SECUREBOOT_AROM_PATCH))
#define NEED_AROM_PATCH 1
#endif
/*---------------------------------------------------------------------------*/
#ifdef MODULE_LZMA_SYSCALL
extern char _rodata, _rodata_size, _rodata_offset;
extern char _ctext, _ctext_size, _ctext_offset;
extern char _data, _data_size, _data_offset;
#endif
#ifdef NEED_AROM_PATCH
extern char _patch, _patch_size, _patch_offset;
#endif
extern char _end, __lzma_decode_start;
/*---------------------------------------------------------------------------*/
extern int  _start(bl2_arg_t *arg);
/*---------------------------------------------------------------------------*/
__attribute__ ((section(".ro.loadtable")))
const uintptr_t _load_table[] = {
  (uintptr_t)&_end,
  'L' | 'D' << 8 | 'T' << 16 | 'B' << 24,
#ifdef MODULE_LZMA_SYSCALL
  (uintptr_t)&_rodata, (uintptr_t)&_rodata_size, (uintptr_t)&_rodata_offset, (uintptr_t)&_rodata_size,
  (uintptr_t)&_ctext, (uintptr_t)&_ctext_size, (uintptr_t)&_ctext_offset, (uintptr_t)&_ctext_size,
  (uintptr_t)&_data, (uintptr_t)&_data_size, (uintptr_t)&_data_offset, (uintptr_t)&_data_size,
#endif
#ifdef NEED_AROM_PATCH
  (uintptr_t)&_patch, (uintptr_t)&_patch_size, (uintptr_t)&_patch_offset, (uintptr_t)&_patch_size,
#endif
  0
};
/*---------------------------------------------------------------------------*/
static uint8_t *
relocate_image(void)
{
  /* copy image to lzma decompression area */
  uint32_t *end_addr = (uint32_t *)_load_table;
  size_t size = (size_t)(*end_addr) - (size_t)&_start;
  uint32_t *src = (uint32_t *)&_start;
  uint32_t *dst = (uint32_t *)&__lzma_decode_start;
  while(size) {
    *dst++ = *src++;
    size -= sizeof(uint32_t);
  }

  return (uint8_t *)&__lzma_decode_start;
}
/*---------------------------------------------------------------------------*/
int
process_loadtable(void)
{
  uint8_t *image_addr = relocate_image();
#ifdef MODULE_LZMA_SYSCALL
  syscall_init();
#endif
  /* copy the initialized data segment if necessary */
  if(copy_sections((struct load_table *)(_load_table + 2), image_addr)) {
    return -1;
  }
  return 0;
}
