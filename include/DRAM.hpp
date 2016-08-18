#include <GICPL400.hpp>

struct DRAM {
    static const struct Instance {
        volatile DRAM* address;
    } instances[];

    static void initialize() {
        auto DRAMCOM = AllwinnerDRAMCOM::instances[0].address; // 0x01C62000
        auto DRAMCTL = AllwinnerDRAMCTL::instances[0].address; // 0x01C63000

        DRAMCTL->clockEnable = 0xC00E; // 0x01C6300C

        DRAMCTL->mode[0] = 0x00001C70; // 0x01C63030
        DRAMCTL->mode[1] = 0x00000040; // 0x01C63034
        DRAMCTL->mode[2] = 0x00000018; // 0x01C63038
        DRAMCTL->mode[3] = 0x00000000; // 0x01C6303C

        DRAMCTL->timing[0] = 0x0C11180D; // 0x01C63058
        DRAMCTL->timing[1] = 0x00030412; // 0x01C6305C
        DRAMCTL->timing[2] = 0x0406050A; // 0x01C63060
        DRAMCTL->timing[3] = 0x0000400C; // 0x01C63064
        DRAMCTL->timing[4] = 0x06020406; // 0x01C63068
        DRAMCTL->timing[5] = 0x05050403; // 0x01C6306C
        DRAMCTL->timing[8] &= ~0xFFFF; // 0x01C63078
        DRAMCTL->timing[8] |= 0x6610; // 0x01C63078

        DRAMCTL->piTiming[0] = 0x02040102; // 0x01C63080
        DRAMCTL->PT[3] = 0x10D52081; // 0x01C63050
        DRAMCTL->PT[4] = 0x2A120D01; // 0x01C63054
        DRAMCTL->refreshTiming = 0x00510076; // 0x01C63090

        DRAMCOM->unknown0[2] = 0x0000018F; // 0x01C6200C
        DRAMCOM->unknown1[8] = 0x00010000; // 0x01C62090
        const Natural32 bandwidthLimit[] = {
            0x00A0000D, 0x06000009, 0x0200000D, 0x01000009, 0x07000009, 0x01000009,
            0x01000009, 0x0100000D, 0x0100000D, 0x04000009, 0x20000209, 0x05000009
        };
        const Natural32 port[] = {
            0x00500064, 0x01000578, 0x00600100, 0x00500064, 0x01000640, 0x00000080,
            0x00400080, 0x00400080, 0x00400080, 0x00400100, 0x08001800, 0x00400090
        };
        for(Natural8 i = 0; i < 12; ++i) {
            DRAMCOM->masterConfig[i].bandwidthLimit = bandwidthLimit[i]; // 0x01C62010 - 0x01C62068
            DRAMCOM->masterConfig[i].port = port[i]; // 0x01C62014 - 0x01C6206C
        }
        DRAMCOM->unknown1[48] = 0x81000004; // 0x01C62130

        DRAMCTL->PGC[0] &= ~((1<<30)|0x3F); // 0x01C63100
        DRAMCTL->PGC[1] |= (1<<26); // 0x01C63104
        DRAMCTL->PGC[1] &= ~(1<<24); // 0x01C63104

        DRAMCOM->unknown0[2] |= 0x18F; // 0x01C6200C
        DRAMCOM->unknown1[484] = 0x94BE6FA3; // 0x01C62800
        DRAMCTL->MXUpdate[2] |= (0x50<<16); // 0x01C63888
        DRAMCOM->unknown1[484] = 0; // 0x01C62800

        DRAMCTL->pad2[7] |= (1<<9); // 0x01C630B8
        DRAMCTL->PGC[2] &= ~(0xF<<8); // 0x01C63108
        DRAMCTL->PGC[2] |= (0x3<<8); // 0x01C63108
        for(Natural8 i = 0; i < 4; ++i)
            DRAMCTL->DX[i].GC &= ~0xF03E; // 0x01C63344 + i*0x80
        DRAMCTL->pad6[0] |= 2; // 0x01C63208
        DRAMCTL->PGC[2] &= ~0x2000; // 0x01C63108
        DRAMCTL->PGC[0] &= ~(1<<28); // 0x01C63100

        const Natural32 CAIOC[] = { 0x3352AD05, 0x00013330, 0x04143043, 0x045D1011 };
        for(Natural8 j = 0; j < 4; ++j)
            for(Natural8 i = 0; i < 8; ++i)
                DRAMCTL->CAIOC[j*8+i] = ((CAIOC[j]>>(i*4))&0xF)<<8; // 0x01C63210 - 0x01C6328C

        DRAMCTL->DX[0].BDL6 = 0x01000000; // 0x01C6333C
        DRAMCTL->DX[0].DATXIOC[0] = 0x10; // 0x01C63310
        DRAMCTL->DX[0].DATXIOC[1] = 0x10; // 0x01C63314
        DRAMCTL->DX[0].DATXIOC[2] = 0x10; // 0x01C63318
        DRAMCTL->DX[0].DATXIOC[3] = 0x10; // 0x01C6331C
        DRAMCTL->DX[0].DATXIOC[4] = 0x11; // 0x01C63320
        DRAMCTL->DX[0].DATXIOC[5] = 0x10; // 0x01C63324
        DRAMCTL->DX[0].DATXIOC[6] = 0x10; // 0x01C63328
        DRAMCTL->DX[0].DATXIOC[7] = 0x11; // 0x01C6332C
        DRAMCTL->DX[0].DATXIOC[8] = 0x10; // 0x01C63330
        DRAMCTL->DX[0].DATXIOC[9] = 0xF01; // 0x01C63334

        DRAMCTL->DX[1].BDL6 = 0x01000000; // 0x01C633BC
        DRAMCTL->DX[1].DATXIOC[0] = 0x11; // 0x01C63390
        DRAMCTL->DX[1].DATXIOC[1] = 0x11; // 0x01C63394
        DRAMCTL->DX[1].DATXIOC[2] = 0x11; // 0x01C63398
        DRAMCTL->DX[1].DATXIOC[3] = 0x11; // 0x01C6339C
        DRAMCTL->DX[1].DATXIOC[4] = 0x111; // 0x01C633A0
        DRAMCTL->DX[1].DATXIOC[5] = 0x111; // 0x01C633A4
        DRAMCTL->DX[1].DATXIOC[6] = 0x111; // 0x01C633A8
        DRAMCTL->DX[1].DATXIOC[7] = 0x111; // 0x01C633AC
        DRAMCTL->DX[1].DATXIOC[8] = 0x11; // 0x01C633B0
        DRAMCTL->DX[1].DATXIOC[9] = 0xA01; // 0x01C633B4

        // DRAMCTL->DX[2].BDL6 = 0x01000000; // 0x01C6343C
        DRAMCTL->DX[2].DATXIOC[0] = 0x110; // 0x01C63410
        DRAMCTL->DX[2].DATXIOC[1] = 0x11; // 0x01C63414
        DRAMCTL->DX[2].DATXIOC[2] = 0x111; // 0x01C63418
        DRAMCTL->DX[2].DATXIOC[3] = 0x110; // 0x01C6341C
        DRAMCTL->DX[2].DATXIOC[4] = 0x110; // 0x01C63420
        DRAMCTL->DX[2].DATXIOC[5] = 0x110; // 0x01C63424
        DRAMCTL->DX[2].DATXIOC[6] = 0x110; // 0x01C63428
        DRAMCTL->DX[2].DATXIOC[7] = 0x110; // 0x01C6342C
        DRAMCTL->DX[2].DATXIOC[8] = 0x10; // 0x01C63430
        DRAMCTL->DX[2].DATXIOC[9] = 0xB00; // 0x01C63434

        DRAMCTL->DX[3].BDL6 = 0x01000000; // 0x01C634BC
        DRAMCTL->DX[3].DATXIOC[0] = 0x111; // 0x01C63490
        DRAMCTL->DX[3].DATXIOC[1] = 0x11; // 0x01C63494
        DRAMCTL->DX[3].DATXIOC[2] = 0x11; // 0x01C63498
        DRAMCTL->DX[3].DATXIOC[3] = 0x111; // 0x01C6349C
        DRAMCTL->DX[3].DATXIOC[4] = 0x111; // 0x01C634A0
        DRAMCTL->DX[3].DATXIOC[5] = 0x111; // 0x01C634A4
        DRAMCTL->DX[3].DATXIOC[6] = 0x111; // 0x01C634A8
        DRAMCTL->DX[3].DATXIOC[7] = 0x111; // 0x01C634AC
        DRAMCTL->DX[3].DATXIOC[8] = 0x11; // 0x01C634B0
        DRAMCTL->DX[3].DATXIOC[9] = 0xC01; // 0x01C634B4

        DRAMCTL->ZQ[0].C &= ~0x00FFFFFF; // 0x01C63140
        DRAMCTL->ZQ[0].C |= 0x003B3BBB; // 0x01C63140
        DRAMCTL->PI = 0x5F2; // 0x01C63000
        DRAMCTL->PI |= 1; // 0x01C63000
        while((DRAMCTL->PGS[0]&1) == 0); // 0x01C63010
        DRAMCOM->unknown1[24] |= (1<<31); // 0x01C620D0
        while((DRAMCTL->status&1) == 0); // 0x01C63018
        DRAMCTL->PGC[3] &= ~(0x6<<24); // 0x01C6310C
        DRAMCTL->DTC &= ~(3<<24); // 0x01C630C0
        DRAMCTL->DTC |= (1<<24); // 0x01C630C0
        DRAMCTL->PI = 0x401; // 0x01C63000
        while((DRAMCTL->PGS[0]&1) == 0); // 0x01C63010
        DRAMCTL->ZQ[0].C |= (1<<31); // 0x01C63140

        if(DRAMCTL->PGS[0]&0xFF0000) // 0x01C63010
            puts("[FAIL] DRAM");
        else
            puts("[ OK ] DRAM");
    }
};
