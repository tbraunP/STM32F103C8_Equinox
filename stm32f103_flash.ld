/*
 *  Linker script for STM32F407VG Device with  1024KByte FLASH, 128KByte RAM
 */
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)

/* entry Point */
ENTRY(Reset_Handler)

/* Specify the memory areas */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 0x00010000
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 0x00005000
}

/* Highest address for stack start */
_estack = 0x20000000 + 0x00005000;

/* The size of the stack used by the application. NOTE: you need to adjust*/
STACK_SIZE = 1024;
/* The size of the heap used by the application. NOTE: you need to adjust */
HEAP_SIZE = 6*1024;

SECTIONS {
	.isr_vector : { 						/* the vector table goes FIRST into FLASH */
		KEEP(*(.isr_vector)) 				/* vector table */
		. = ALIGN(4);
	} >FLASH

	/* code and constants */
	.text : {
		. = ALIGN(4);
		*(.text)							/* .text sections (code) */
		*(.text*)							/* .text* sections (code) */
		*(.rodata)							/* .rodata sections (constants, strings, etc.) */
		*(.rodata*)							/* .rodata* sections (constants, strings, etc.) */

		KEEP(*(.init))
		_fini = .;
		KEEP(*(.fini))
				
		. = ALIGN(4);
		_etext = .;							/* global symbols at end of code */
	} >FLASH


 	/* C++ exception unwinding stuff--needed by some toolchains */
	.ARM.extab   : { 
	    *(.ARM.extab* .gnu.linkonce.armextab.*) 
	} >FLASH
	
	.ARM : {
	    __exidx_start = .;
	    *(.ARM.exidx*)
	    __exidx_end = .;
	} >FLASH

	.preinit_array : {
		PROVIDE(__preinit_array_start = .);
		KEEP (*(.preinit_array*))
		PROVIDE(__preinit_array_end = .);
	} >FLASH

	/* The .preinint_array and .inint_array sections hold arrays of function pointers that are
	called by the startup code to initialize the program. In C++ programs these hold pointers to the static
	constructors that are called by __libc_init_array() before main(). */
	.init_array : {
      . = ALIGN(4);
		PROVIDE(__init_array_start = .);

		/* support for c++, see http://tech.munts.com/MCU/Frameworks/ARM/stm32f4/stm32f407vg.lld*/    	
	   /*__ctors_start__ = .;*/
	   	/*
		    KEEP (*crtbegin.o(.ctors))
	    	KEEP (*crtbegin?.o(.ctors))
	    	KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors)) 
	    */
    	
      	KEEP (*(SORT(.init_array.*)))
    	KEEP(*(.init_array*));  		/* C++ constructors */
    	
    	/*KEEP(*(.ctors)); */			/* C++ constructors */

    		/*__ctors_end__ = .;*/
		PROVIDE(__init_array_end = .);
	} >FLASH
	

	 /* The .fini_array section holds an array of function pointers that are called before terminating the
	program. In C++ programs this array holds pointers to the static destructors.*/
	.fini_array : {
		PROVIDE(__fini_array_start = .);
		KEEP (*(.fini_array*))
		KEEP (*(SORT(.fini_array.*)))
		PROVIDE(__fini_array_end = .);
	} >FLASH

	/* The .data section contains initialized data.
	The .data section is located in RAM, but is loaded to ROM and copied to RAM during startup. */
	.data : {
		__data_load = LOADADDR (.data);
		_sidata = LOADADDR(.data);
		_sdata = .;
		__data_start = .;
		*(.data)							/* .data sections */
		*(.data*)							/* .data* sections */
		. = ALIGN(4);
		__data_end__ = .;
		_edata = __data_end__;
	} >RAM AT>FLASH

 	/* Uninitialized data section */
	.bss : {
		. = ALIGN(4);
		__bss_start__ = . ;
		_sbss = .;
		*(.bss)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;							/* define a global symbol at bss end */
		__bss_end__ = .;
	} >RAM

	PROVIDE( end = _ebss );
	PROVIDE( _end = _ebss );
	PROVIDE( __end__ = _ebss );

	/* Heap */
	.heap : {
		__heap_start__ = . ;
		. = . + HEAP_SIZE;
		. = ALIGN(8);
		__heap_end__ = . ;
	} >RAM

	/* Stack */
	.stack : {
		__stack_start__ = . ;
		. = . + STACK_SIZE;
		. = ALIGN(4);
		__c_stack_top__ = . ;
		__stack_end__ = . ;
	} >RAM

	/* Remove information from the standard libraries */
	/DISCARD/ : {
		libc.a ( * )
		libm.a ( * )
		libgcc.a ( * )
	}
	
	.ARM.attributes 0 : { *(.ARM.attributes) }
}
