__MAX_ROM_SIZE   = 1024*1024;

__RAM_START_ADDR = 0x7e420000;
__MAX_RAM_SIZE   = 0x100000;


SECTIONS
{
	. = __RAM_START_ADDR;
	.initdata : ALIGN(4)
	{ 
		KEEP(*(.initdata))
	}
	.userStartUp :ALIGN(4)
	{
		KEEP(*(.userInit))
	}
	.text : ALIGN(4)
	{ 
	    *(.text)
	    *(.text*)
	}
	.rodata : ALIGN(4)
	{
	    *(.rodata)
	    *(.rodata*)
	}
	.data : 
	{
		PROVIDE_HIDDEN (__data_load = LOADADDR(.data));
		PROVIDE_HIDDEN (__data_start = .);
		*(.data)
		*(.data*)
		PROVIDE_HIDDEN (_edata = .);
	}
	.userBootTable :ALIGN(4)
	{
		PROVIDE_HIDDEN (__userBoot_start__ = .);
		KEEP(*(.userBootTable))
		PROVIDE_HIDDEN (__userBoot_end__ = .);
	}
	.userKernelTable :ALIGN(4)
	{
		PROVIDE_HIDDEN (__userKernel_start__ = .);
		KEEP(*(.userKernelTable))
		PROVIDE_HIDDEN (__userKernel_end__ = .);
	}
	.appRegTable :ALIGN(4)
	{
		PROVIDE_HIDDEN (__appRegTable_start__ = .);
		KEEP(*(.appRegTable))
		PROVIDE_HIDDEN (__appRegTable_end__ = .);
	}
	.ARM.exidx : ALIGN(4)
	{ 
		PROVIDE_HIDDEN (__exidx_start = .);
		*(.ARM.exidx);
		PROVIDE_HIDDEN (__exidx_end = .);
	}
	
	ASSERT(__exidx_end - __RAM_START_ADDR < __MAX_ROM_SIZE,"xxxx ERR:APP ROM OVERFLOW !!")

	.bss : ALIGN(4)
	{
		PROVIDE_HIDDEN (__bss_start__ = .);
		*(.bss)
		*(.bss*)
		*(COMMON)
		PROVIDE_HIDDEN (__bss_end__ = .);
	}
	.ymdata :ALIGN(4)
	{
		PROVIDE_HIDDEN (__ymdata_start__ = .);
		KEEP(*(.ymdata))
		PROVIDE_HIDDEN (__ymdata_end__ = .);
	}
	.appHeader :
	{
		PROVIDE_HIDDEN (__appHeader_start__ = .);
		KEEP(*(.appHeader))
		PROVIDE_HIDDEN (__appHeader_end__ = .);
	}
	end = .;

	ASSERT(end -  __RAM_START_ADDR < __MAX_RAM_SIZE,"xxxx ERR:APP RAM OVERFLOW !!")

}
