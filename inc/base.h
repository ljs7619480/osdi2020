#ifndef _BASE_H
#define _BASE_H


#define MMIO_BASE               0x3F000000
#define UART_REG_BASE           (MMIO_BASE+0x00201000) //UART
// #define SYS_TIM_REG_BASE        (MMIO_BASE+0x00003000) //System timer
#define INTE_REG_BASE           (MMIO_BASE+0x0000B000) //Interrupt
#define MBOX_REG_BASE           (MMIO_BASE+0x0000B880) //Mailbox

#endif//_BASE_H