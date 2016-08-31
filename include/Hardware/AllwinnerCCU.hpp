#include "AllwinnerPRCM.hpp"

struct AllwinnerCCU {
    static const struct Instance {
        volatile AllwinnerCCU* address;
    } instances[];

    Natural32
        PLLCPUXControl,
        pad0,
        PLLAUDIOControl,
        pad1,
        PLLVIDEO0Control,
        pad2,
        PLLVEControl,
        pad3,
        PLLDDR0Control,
        pad4,
        PLLPERIPH0Control,
        PLLPERIPH1Control,
        PLLVIDEO1Control,
        pad5,
        PLLGPUControl,
        pad6,
        PLLMIPIControl,
        PLLHSICControl,
        PLLDEControl,
        PLLDDR1Control,
        CPUXAXIConfiguration,
        AHB1APB1Configuration,
        APB2Configuration,
        AHB2Configuration,
        BusClockGating[5],
        THSClock,
        pad7[2],
        NANDClock,
        pad8,
        SMHCClock[3],
        pad9,
        TSClock,
        CEClock,
        SPI0Clock,
        SPI1Clock,
        pad10[2],
        I2SPCM0Clock,
        I2SPCM1Clock,
        I2SPCM2Clock,
        pad11,
        SPDIFClock,
        pad12[2],
        USBPHYConfiguration,
        pad13[9],
        DRAMConfiguration,
        PLLDDRConfiguration,
        MBUSReset,
        DRAMClockGating,
        DEClock,
        pad14[5],
        TCON0Clock,
        TCON1Clock,
        pad15,
        DEINTERLACEClock,
        pad16[2],
        CSIMISCClock,
        CSIClock,
        VEClock,
        ACDigitalClock,
        AVSClock,
        pad17[2],
        HDMIClock,
        HDMISlowClock,
        pad18,
        MBUSClock,
        pad19[2],
        MIPIDSIClock,
        pad20[13],
        GPUClock,
        pad21[23],
        PLLStableTime0,
        PLLStableTime1,
        pad22[6],
        PLLCPUXBias,
        PLLAUDIOBias,
        PLLVIDEO0Bias,
        PLLVEBias,
        PLLDDR0Bias,
        PLLPERIPH0Bias,
        PLLVIDEO1Bias,
        PLLGPUBias,
        PLLMIPIBias,
        PLLHSICBias,
        PLLDEBias,
        PLLDDR1Bias,
        PLLCPUXTuning,
        pad23[3],
        PLLDDR0Tuning,
        pad24[3],
        PLLMIPITuning,
        pad25[2],
        PLLPERIPH1PatternControl,
        PLLCPUXPatternControl,
        PLLAUDIOPatternControl,
        PLLVIDEO0PatternControl,
        PLLVEPatternControl,
        PLLDDR0PatternControl,
        pad26,
        PLLVIDEO1PatternControl,
        PLLGPUPatternControl,
        PLLMIPIPatternControl,
        PLLHSICPatternControl,
        PLLDEPatternControl,
        PLLDDR1PatternControl0,
        PLLDDR1PatternControl1,
        pad27[3],
        BusSoftwareReset0,
        BusSoftwareReset1,
        BusSoftwareReset2,
        pad28,
        BusSoftwareReset3,
        pad29,
        BusSoftwareReset4,
        pad30[5],
        CCMSecuritySwitch,
        pad31[3],
        PSControl,
        PSCounter,
        pad32[6],
        PLLLockControl;

    void configureUART0() volatile {
        BusClockGating[3] |= (1<<16);
        BusSoftwareReset4 |= (1<<16);

        auto PIO = AllwinnerPIO::instances[0].address;
        PIO->banks[1].configure[1].slot0 = 4; // PB8 : UART0_TX
        PIO->banks[1].configure[1].slot1 = 4; // PB9 : UART0_RX
        PIO->banks[1].pull[0].slot9 = 1; // PB9 : PullUP
    }

    void configureRSB() volatile {
        auto PRCM = AllwinnerPRCM::instances[0].address;
        PRCM->APBSSoftwareReset |= 8; // 0x01F014B0 : R_RSB_RESET
        PRCM->APBSClockGating |= 9; // 0x01F01428 : R_PIO_GATING, R_RSB_GATING

        auto PIO = AllwinnerPIO::instances[1].address;
        PIO->banks[0].configure[0].slot0 = 2; // PL0 : S_RSB_SCK
        PIO->banks[0].configure[0].slot1 = 2; // PL1 : S_RSB_SDA
        PIO->banks[0].multiDriving[0].slot0 = 2; // PL0 : MultiDrivingLevel 2
        PIO->banks[0].multiDriving[0].slot1 = 2; // PL1 : MultiDrivingLevel 2
    }

    void configureEMAC() volatile {
        auto PIO = AllwinnerPIO::instances[0].address;
        PIO->banks[3].configure[1].slot7 = 4; // PD15 : RGMII_TXD3  | MII_TXD3  | RMII_NULL
        PIO->banks[3].configure[1].slot6 = 7; // PD14 : RGMII_NULL  | MII_RXERR | RMII_RXER // Unnecessary
        PIO->banks[3].configure[1].slot5 = 4; // PD13 : RGMII_RXCTL | MII_RXDV  | RMII_CRS_DV
        PIO->banks[3].configure[1].slot4 = 4; // PD12 : RGMII_RXCK  | MII_RXCK  | RMII_NULL
        PIO->banks[3].configure[1].slot3 = 4; // PD11 : RGMII_RXD0  | MII_RXD0  | RMII_RXD0
        PIO->banks[3].configure[1].slot2 = 4; // PD10 : RGMII_RXD1  | MII_RXD1  | RMII_RXD1
        PIO->banks[3].configure[1].slot1 = 4; //  PD9 : RGMII_RXD2  | MII_RXD2  | RMII_NULL
        PIO->banks[3].configure[1].slot0 = 4; //  PD8 : RGMII_RXD3  | MII_RXD3  | RMII_NULL
        PIO->banks[3].configure[2].slot7 = 4; // PD23 : MDIO
        PIO->banks[3].configure[2].slot6 = 4; // PD22 : MDC
        PIO->banks[3].configure[2].slot5 = 4; // PD21 : RGMII_CLKIN | MII_COL   | RMII_NULL
        PIO->banks[3].configure[2].slot4 = 4; // PD20 : RGMII_TXCTL | MII_TXEN  | RMII_TXEN
        PIO->banks[3].configure[2].slot3 = 4; // PD19 : RGMII_TXCK  | MII_TXCK  | RMII_TXCK
        PIO->banks[3].configure[2].slot2 = 4; // PD18 : RGMII_TXD0  | MII_TXD0  | RMII_TXD0
        PIO->banks[3].configure[2].slot1 = 4; // PD17 : RGMII_TXD1  | MII_TXD1  | RMII_TXD1
        PIO->banks[3].configure[2].slot0 = 4; // PD16 : RGMII_TXD2  | MII_TXD2  | RMII_NULL

        auto SYSCTL = AllwinnerSYSCTL::instances[0].address;
        BusClockGating[0] |= (1<<17);
        BusSoftwareReset0 |= (1<<17);

        SYSCTL->EMACClock.clockSource = 2;
        SYSCTL->EMACClock.phyInterface = 1;
        SYSCTL->EMACClock.invertTransmitClock = 0;
        SYSCTL->EMACClock.invertReceiveClock = 0;
        SYSCTL->EMACClock.receiveClockDelayChain = 3;
        SYSCTL->EMACClock.transmitClockDelayChain = 0;
        SYSCTL->EMACClock.RMIIEnable = 0;
    }

    void configurePLL() volatile {
        PLLLockControl |= (1<<28); // 0x01C20320 : MODE_SEL New Mode

        // CPUXAXI init
        CPUXAXIConfiguration |= 2; // 0x01C20050
        PLLLockControl |= (1<<0); // 0x01C20320
        PLLCPUXControl |= (1<<31); // 0x01C20000
        while((PLLCPUXControl&(1<<28)) == 0); // 0x01C20000
        PLLLockControl &= ~(1<<0); // 0x01C20320
        CPUXAXIConfiguration &= ~(3<<16); // 0x01C20050
        CPUXAXIConfiguration |= (2<<16); // 0x01C20050

        // HSIC init
        PLLLockControl |= (1<<9); // 0x01C20320
        PLLHSICControl |= (1<<31); // 0x01C20044
        while((PLLHSICControl&(1<<28)) == 0); // 0x01C20044
        PLLLockControl &= ~(1<<9); // 0x01C20320

        // AHB1APB1 init
        AHB1APB1Configuration = (AHB1APB1Configuration&(3<<12))|(1<<12); // 0x01C20054
        PLLLockControl |= (1<<5); // 0x01C20320
        PLLPERIPH0Control |= (1<<31); // 0x01C20028
        while((PLLPERIPH0Control&(1<<28)) == 0); // 0x01C20028
        PLLLockControl &= ~(1<<5); // 0x01C20320
        AHB1APB1Configuration = 0x180; // 0x01C20054
        AHB1APB1Configuration |= (3<<12); // 0x01C20054

        // PLLDDR0 init
        // PLLDDR0Control &= ~(0x7F<<8); // 0x01C20020
        // PLLDDR0Control = (1<<31)|(27<<8); // 0x01C20020 : PLL_ENABLE, PLL_FACTOR_N: 28 (672 MHz)
        // PLLDDR0Control |= (1<<20); // 0x01C20020 : PLL_DDR0_CFG_UPDATE
        // PLLLockControl |= (1<<4); // 0x01C20320 : Lock Enable PLL_DDR0
        // puts("Wait for PLL0");
        // while(((PLLDDR0Control>>20)&0x11) != 0x10); // 0x01C20020 : LOCK, PLL_DDR0_CFG_UPDATE
        // PLLLockControl &= ~(1<<4); // 0x01C20320

        // PLLDDR1 init
        PLLDDR1Control &= ~(0x7F<<8); // 0x01C2004C
        PLLDDR1Control |= (1<<31)|(55<<8); // 0x01C2004C : PLL_ENABLE, PLL_FACTOR_N: 55 (1320 MHz)
        PLLDDR1Control |= (1<<30); // 0x01C2004C : SDRPLL_UPD
        PLLLockControl |= (1<<11); // 0x01C20320 : PLL_DDR1
        while(((PLLDDR1Control>>28)&0x5) != 0x1); // 0x01C2004C : SDRPLL_UPD, LOCK
        PLLLockControl &= ~(1<<11); // 0x01C20320

        PLLLockControl &= ~(1<<28); // 0x01C20320 : MODE_SEL Old Mode
    }

    void configureDRAM() volatile {
        DRAMConfiguration |= (1<<20); // 0x01C200F4 : PLL_DDR1
        DRAMConfiguration |= (1<<16); // 0x01C200F4 : SDRCLK_UPD
        while(DRAMConfiguration&(1<<16));
        BusSoftwareReset0 |= (1<<14); // 0x01C202C0 : SDRAM_RST
        BusClockGating[0] |= (1<<14); // 0x01C20060 : DRAM_GATING
        MBUSClock = 0x81000002; // 0x01C2015C
        DRAMConfiguration |= (1<<31); // 0x01C200F4 : DRAM_CTR_RST
    }

    void configureSMHC() volatile {
        BusClockGating[0] |= (7<<8); // 8, 9, 10
        BusSoftwareReset0 |= (7<<8); // 8, 9, 10

        // TODO: The recommended clock frequency is 200MHz.
        // SMHCClock[0] = ;
        // SMHCClock[1] = ;
        // SMHCClock[2] = ;
    }
};
