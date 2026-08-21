/*
******************************************************************************
**
**  File        : LinkerScript.ld
**
**  Author      : STM32CubeIDE
**
**  Abstract    : Linker script for STM32H7 series
**                2048Kbytes FLASH and 1056Kbytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
**
*****************************************************************************
** @attention
**
** Copyright (c) 2024 STMicroelectronics.
** All rights reserved.
**
** This software is licensed under terms that can be found in the LICENSE file
** in the root directory of this software component.
** If no LICENSE file comes with this software, it is provided AS-IS.
**
****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(DTCMRAM) + LENGTH(DTCMRAM);    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x800 ;      /* required amount of heap  */
_Min_Stack_Size = 0x8000 ; /* required amount of stack - INCREASED for AI model */

/* Specify the memory areas */
MEMORY
{
  FLASH (rx)     : ORIGIN = 0x08000000, LENGTH = 2048K
  DTCMRAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 128K
  RAM_D1 (xrw)   : ORIGIN = 0x24000000, LENGTH = 512K
  RAM_D2 (xrw)   : ORIGIN = 0x30000000, LENGTH = 288K
  RAM_D3 (xrw)   : ORIGIN = 0x38000000, LENGTH = 64K
  ITCMRAM (xrw)  : ORIGIN = 0x00000000, LENGTH = 64K
  SDRAM (xrw)    : ORIGIN = 0xC0000000, LENGTH = 32M
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH
  .ARM (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array (READONLY) : /* The READONLY keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  .AI_DTCMRAM :
  {
    . = ALIGN(4);
    KEEP(*(.AI_DTCMRAM))
  } >DTCMRAM

  /* Real-time inference frame pool section (DTCMRAM) */
  .dtcmram (NOLOAD) :
  {
    . = ALIGN(16);
    *(.dtcmram)
    *(.dtcmram*)
    . = ALIGN(16);
  } >DTCMRAM

  .AI_ITCMRAM :
  {
    . = ALIGN(4);
    KEEP(*(.AI_ITCMRAM))
  } >ITCMRAM

  .AI_RAM_D2 :
  {
    . = ALIGN(4);
    KEEP(*(.AI_RAM_D2))
  } >RAM_D2

  .AI_RAM_D3 :
  {
    . = ALIGN(4);
    KEEP(*(.AI_RAM_D3))
  } >RAM_D3

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM_D1 AT> FLASH

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM_D1

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >DTCMRAM

  /* RAM_D2 section for task stacks and queues (~50KB / 288KB used) */
  /* Contents:
   *   - Task stacks (3-task AI system): 40KB
   *     * producerTaskStack_3task[2048]: 8KB
   *     * windowManagerTaskStack[4096]: 16KB
   *     * aiInferenceTaskStack[4096]: 16KB
   *   - calibrationTaskStack[2048]: 8KB
   *   - frameQueueStorage_3task[256 × 4]: 1KB
   *   - Other small buffers: ~1KB
   *   TOTAL: ~50KB (plenty of headroom)
   */
  .ram_d2 (NOLOAD) :
  {
    . = ALIGN(4);
    *(.ram_d2)
    *(.ram_d2*)
    . = ALIGN(4);
  } >RAM_D2

  /* ========================================================================= */
  /* SDRAM SECTIONS (32MB Total)                                               */
  /* ========================================================================= */
  
  /* 1. AI Activation Buffer (Starts at 0xC0000000) */
  .ai_activation_buffer (NOLOAD) :
  {
    . = ALIGN(32);
    _ai_activation_start = .;
    KEEP(*(.ai_activation_buffer))
    KEEP(*(.ai_activation_buffer.*))
    . = . + 1M;                    /* Reserve 1MB guaranteed */
    . = ALIGN(32);
    _ai_activation_end = .;
  } >SDRAM

  /* 2. AI Model Weights Area (Reserved 0xC0200000 - 0xC0616000) */
  /* We don't define a section here because weights are loaded via Fire Hose */

  /* 3. User Data & Circular Buffers (Starts at 0xC0700000) */
  .sdram_data 0xC0700000 (NOLOAD) :
  {
    . = ALIGN(32);
    _ssdram_data = .;
    *(.sdram_data)
    *(.sdram_data*)
    . = ALIGN(32);
    _esdram_data = .;
  } >SDRAM

  /* 4. Test Data Area (Starts at 0xC1E00000) */
  .sdram_test_data 0xC1E00000 (NOLOAD) :
  {
    . = ALIGN(16);
    _ssdram_test_data = .;
    *(.sdram_test_data)
    . = ALIGN(16);
    _esdram_test_data = .;
  } >SDRAM

  _lsdram_test_data = LOADADDR(.sdram_test_data);

  /* Remove information from the standard libraries */
  /DISCARD/ :
  
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}


