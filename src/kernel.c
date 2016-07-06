typedef char unsigned Natural8;
typedef char Integer8;
typedef short unsigned Natural16;
typedef short Integer16;
typedef unsigned Natural32;
typedef int Integer32;
typedef float Float32;
typedef long long unsigned Natural64;
typedef long long int Integer64;
typedef double Float64;

typedef volatile struct {
    Natural32 DR;
    Natural32 RSR_ECR;
    Natural8 reserved1[0x10];
    const Natural32 FR;
    Natural8 reserved2[0x4];
    Natural32 LPR;
    Natural32 IBRD;
    Natural32 FBRD;
    Natural32 LCR_H;
    Natural32 CR;
    Natural32 IFLS;
    Natural32 IMSC;
    const Natural32 RIS;
    const Natural32 MIS;
    Natural32 ICR;
    Natural32 DMACR;
    Natural8 reserved3[0xF94];
    const Natural32 PeriphID[4];
    const Natural32 PCellID[4];
} pl011;
pl011* const UART[] = { (pl011*)0x101f1000, (pl011*)0x101f2000, (pl011*)0x101f3000 };

enum UART_RSR_ECR {
    Overrun_Error = 1<<3,
    Break_Error = 1<<2,
    Parity_Error = 1<<1,
    Framing_Error = 1<<0
};

enum UART_FR {
    Ring_Indicator = 1<<8,
    Transmit_Empty = 1<<7,
    Receive_Full = 1<<6,
    Transmit_Full = 1<<5,
    Receive_Empty = 1<<4,
    UART_Busy = 1<<3,
    Data_Carrier_Detect = 1<<2,
    Data_Set_Ready = 1<<1,
    Clear_To_Send = 1<<0
};

enum UART_LCR_H {
    Stick_Parity_Select = 1<<7,
    Word_Length_8 = 3<<5,
    Word_Length_7 = 2<<5,
    Word_Length_6 = 1<<5,
    Word_Length_5 = 0<<5,
    FIFO_Enable = 1<<4,
    Two_Stop_Bits_Select = 1<<3,
    Even_Parity_Select = 1<<2,
    Parity_Enable = 1<<1,
    Send_Break = 1<<0
};

enum UART_CR {
    CTS_Hardware_Flow_Control_Enable = 1<<15,
    RTS_Hardware_Flow_Control_Enable = 1<<14,
    Out_2 = 1<<13,
    Out_1 = 1<<12,
    Request_To_Send = 1<<11,
    Data_Transmit_Ready = 1<<10,
    Receive_Enable = 1<<9,
    Transmit_Enable = 1<<8,
    Loop_Back_Enable = 1<<7,
    IrDA_SIR_Low_Power_Mode = 1<<2,
    SIR_Enable = 1<<1,
    UART_Enable = 1<<0
};

Natural8 upperchar(Natural8 c) {
    return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}

void uart_echo(pl011* uart) {
    while(!(uart->FR&Receive_Empty)) {
        while(uart->FR&Transmit_Full);
        uart->DR = upperchar(uart->DR);
    }
}

void uart_init(pl011* uart) {
    uart->CR = 0;
    uart->CR = Receive_Enable | Transmit_Enable | UART_Enable;
}

void uart_print(pl011* uart, const Integer8* str) {
    while(*str)
        uart->DR = *(str++);
}

void _start() {
    asm(
        "ldr x1, =0x07FFFFFF\n"
        "mov sp, x1"
    );

    // uart_print(UART[0], "Hello, World!\n");
    while(1)
        uart_echo(UART[0]);
}
