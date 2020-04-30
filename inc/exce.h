#ifndef	_EXCE_H
#define	_EXCE_H

#include "peri_base.h"


typedef struct{
    volatile unsigned int IRQ_BASIC_PENDING;  // 0x200
    volatile unsigned int IRQ_PENDING_1;      // 0x204
    volatile unsigned int IRQ_PENDING_2;      // 0x208
    volatile unsigned int FIQ_CONTROL;        // 0x20C
    volatile unsigned int ENABLE_IRQS_1;      // 0x210
    volatile unsigned int ENABLE_IRQS_2;      // 0x214
    volatile unsigned int ENABLE_BASIC_IRQS;  // 0x218
    volatile unsigned int DISABLE_IRQS_1;     // 0x21C
    volatile unsigned int DISABLE_IRQS_2;     // 0x220
    volatile unsigned int DISABLE_BASIC_IRQS; // 0x224
} INTE_Typedef;
#define INTE_REG ((INTE_Typedef *)INTE_REG_BASE)

#define INTE_PENDING_1_UART_INT    (1<<25)

#define CORE0_IRQ_SRC        ((volatile unsigned int*)0x40000060)

// #define IRQ_SYSTEM_TIMER_1	(1 << 1)
// #define IRQ_SYSTEM_TIMER_3	(1 << 3)

void sync_svc_handler(unsigned long esr, unsigned long elr);

// void enable_interrupt_controller();
// void handle_irq();
void enable_irq( void );
void disable_irq( void );
#endif  /*_EXC_H */