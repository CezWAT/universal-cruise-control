MEMORY
{
    flash (rx)  : ORIGIN = 0x08000000, LENGTH = 64k
    ram   (rwx) : ORIGIN = 0x20000000, LENGTH = 20k
}

ENTRY(Reset_Handler)

SECTIONS
{
    .vector_table  :
    {
        _start = .;
        . = ALIGN(4);
        KEEP(*(.vector_table ));
        . = ALIGN(4);
    } >flash
    
    .copy.table :
    {
        . = ALIGN(4);
        __copy_table_start__ = .;

        LONG (__etext)
        LONG (__data_start__)
        LONG ((__data_end__ - __data_start__) / 4)

        __copy_table_end__ = .;
    } > flash

    .zero.table :
    {
        . = ALIGN(4);
        __zero_table_start__ = .;

        __zero_table_end__ = .;
    } > flash

    __etext = ALIGN (4);


    .text :
    {
		*(.text);
		_etext = .;
    } >flash

    .data :
    {
        _data = .;
    	*(.data);
    	_edata = .;
    } >ram AT>flash

	.bss :
	{
        __data_start__ = .;
	    _bss = .;
	    __bss_start__ = .;
    	*(.bss)
    	*(COMMON)
    	__bss_end__ = .;
    	_ebss = .;

        __data_end__ = .;
	} >ram

    _vStackTop = ORIGIN(ram) + LENGTH(ram);
}
