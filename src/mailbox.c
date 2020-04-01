#include "mailbox.h"
#include "peri_base.h"
#include "string.h"
#include "uart.h"


static struct _MBOX_REG
{
    volatile unsigned int READ;      // +0x00
    volatile unsigned int RESERVED0; // +0x04
    volatile unsigned int RESERVED1; // +0x08
    volatile unsigned int RESERVED2; // +0x0C
    volatile unsigned int POLL;      // +0x10
    volatile unsigned int SENDER;    // +0x14
    volatile unsigned int STATUS;    // +0x18
    volatile unsigned int CONFIG;    // +0x1C
    volatile unsigned int WRITE;     // +0x20
} *MBOX_REG = (void *)MBOX_REG_BASE;

/* mailbox message buffer */
volatile unsigned int  __attribute__((aligned(16))) mbox[36];


/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mbox_call(unsigned char ch)
{
    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~MBOX_CH_MASK) | (ch&MBOX_CH_MASK));
    /* wait until we can write to the mailbox */
    do{
        asm volatile("nop");
    }while(MBOX_REG-> STATUS & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    MBOX_REG-> WRITE = r;
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{
            asm volatile("nop");
        }while(MBOX_REG-> STATUS & MBOX_EMPTY);
        /* is it a response to our message? */
        if(r == MBOX_REG-> READ)
            return mbox[1]==MBOX_RESPONSE_SUCCESS;
    }
    return 0;
}

void get_board_info(){
    // header
    mbox[ 0] = 17*4;                  // buffer size in bytes
    mbox[ 1] = MBOX_REQUEST_CODE;     // buffer request/response code
    // tags
    mbox[ 2] = MBOX_TAG_GET_BREVI;    // tag identifier
    mbox[ 3] = 1*4;                   // value buffer size in bytes
    mbox[ 4] = MBOX_TAG_REQUEST_CODE; // tag request/response code
    mbox[ 5] = 0;                     // value buffer

    mbox[ 6] = MBOX_TAG_GET_ARMADDR;
    mbox[ 7] = 2*4;
    mbox[ 8] = MBOX_TAG_REQUEST_CODE;
    mbox[ 9] = 0;
    mbox[10] = 0;

    mbox[11] = MBOX_TAG_GET_VCADDR;
    mbox[12] = 2*4;
    mbox[13] = MBOX_TAG_REQUEST_CODE;
    mbox[14] = 0;
    mbox[15] = 0;
    // tailer
    mbox[16] = MBOX_END_TAG;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROPT_ARM_VC)) {
        char buff[11];
        uart_puts("Board revision:");
        bin2hex(mbox[5], buff);
        uart_puts(buff);
        uart_puts("\nARM address base: ");
        bin2hex(mbox[9], buff);
        uart_puts(buff);
        uart_puts("\tsize: ");
        bin2hex(mbox[10], buff);
        uart_puts(buff);
        uart_puts("\nVC  address base: ");
        bin2hex(mbox[14], buff);
        uart_puts(buff);
        uart_puts("\tsize: ");
        bin2hex(mbox[15], buff);
        uart_puts(buff);
        uart_puts("\n");
    } else {
        uart_puts("Mailbox fail to query board info!\n");
    }
}