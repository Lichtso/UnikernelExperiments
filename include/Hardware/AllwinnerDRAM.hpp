#include <Hardware/AllwinnerDRAMCTL.hpp>

struct AllwinnerDRAM {
    static const struct Instance {
        AllwinnerDRAM* address;
    } instances[];

    void initialize() {
        auto dramCom = AllwinnerDRAMCOM::instances[0].address; // 0x01C62000
        auto dramCtl = AllwinnerDRAMCTL::instances[0].address; // 0x01C63000

        dramCtl->clockEnable = 0xC00E; // 0x01C6300C

        dramCtl->mode[0] = 0x00001C70; // 0x01C63030
        dramCtl->mode[1] = 0x00000040; // 0x01C63034
        dramCtl->mode[2] = 0x00000018; // 0x01C63038
        dramCtl->mode[3] = 0x00000000; // 0x01C6303C

        dramCtl->timing[0] = 0x0C11180D; // 0x01C63058
        dramCtl->timing[1] = 0x00030412; // 0x01C6305C
        dramCtl->timing[2] = 0x0406050A; // 0x01C63060
        dramCtl->timing[3] = 0x0000400C; // 0x01C63064
        dramCtl->timing[4] = 0x06020406; // 0x01C63068
        dramCtl->timing[5] = 0x05050403; // 0x01C6306C
        dramCtl->timing[8] &= ~0xFFFF; // 0x01C63078
        dramCtl->timing[8] |= 0x6610; // 0x01C63078

        dramCtl->piTiming[0] = 0x02040102; // 0x01C63080
        dramCtl->PT[3] = 0x10D52081; // 0x01C63050
        dramCtl->PT[4] = 0x2A120D01; // 0x01C63054
        dramCtl->refreshTiming = 0x00510076; // 0x01C63090

        dramCom->unknown0[2] = 0x0000018F; // 0x01C6200C
        dramCom->unknown1[8] = 0x00010000; // 0x01C62090
        const Natural32 bandwidthLimit[] = {
            0x00A0000D, 0x06000009, 0x0200000D, 0x01000009, 0x07000009, 0x01000009,
            0x01000009, 0x0100000D, 0x0100000D, 0x04000009, 0x20000209, 0x05000009
        };
        const Natural32 port[] = {
            0x00500064, 0x01000578, 0x00600100, 0x00500064, 0x01000640, 0x00000080,
            0x00400080, 0x00400080, 0x00400080, 0x00400100, 0x08001800, 0x00400090
        };
        for(Natural8 i = 0; i < 12; ++i) {
            dramCom->masterConfig[i].bandwidthLimit = bandwidthLimit[i]; // 0x01C62010 - 0x01C62068
            dramCom->masterConfig[i].port = port[i]; // 0x01C62014 - 0x01C6206C
        }
        dramCom->unknown1[48] = 0x81000004; // 0x01C62130

        dramCtl->PGC[0] &= ~((1<<30)|0x3F); // 0x01C63100
        dramCtl->PGC[1] |= (1<<26); // 0x01C63104
        dramCtl->PGC[1] &= ~(1<<24); // 0x01C63104

        dramCom->unknown0[2] |= 0x18F; // 0x01C6200C
        dramCom->unknown1[484] = 0x94BE6FA3; // 0x01C62800
        dramCtl->MXUpdate[2] |= (0x50<<16); // 0x01C63888
        dramCom->unknown1[484] = 0; // 0x01C62800

        dramCtl->pad2[7] |= (1<<9); // 0x01C630B8
        dramCtl->PGC[2] &= ~(0xF<<8); // 0x01C63108
        dramCtl->PGC[2] |= (0x3<<8); // 0x01C63108
        for(Natural8 i = 0; i < 4; ++i)
            dramCtl->DX[i].GC &= ~0xF03E; // 0x01C63344 + i*0x80
        dramCtl->pad6[0] |= 2; // 0x01C63208
        dramCtl->PGC[2] &= ~0x2000; // 0x01C63108
        dramCtl->PGC[0] &= ~(1<<28); // 0x01C63100

        const Natural32 CAIOC[] = { 0x3352AD05, 0x00013330, 0x04143043, 0x045D1011 };
        for(Natural8 j = 0; j < 4; ++j)
            for(Natural8 i = 0; i < 8; ++i)
                dramCtl->CAIOC[j*8+i] = ((CAIOC[j]>>(i*4))&0xF)<<8; // 0x01C63210 - 0x01C6328C

        dramCtl->DX[0].BDL6 = 0x01000000; // 0x01C6333C
        dramCtl->DX[0].DATXIOC[0] = 0x10; // 0x01C63310
        dramCtl->DX[0].DATXIOC[1] = 0x10; // 0x01C63314
        dramCtl->DX[0].DATXIOC[2] = 0x10; // 0x01C63318
        dramCtl->DX[0].DATXIOC[3] = 0x10; // 0x01C6331C
        dramCtl->DX[0].DATXIOC[4] = 0x11; // 0x01C63320
        dramCtl->DX[0].DATXIOC[5] = 0x10; // 0x01C63324
        dramCtl->DX[0].DATXIOC[6] = 0x10; // 0x01C63328
        dramCtl->DX[0].DATXIOC[7] = 0x11; // 0x01C6332C
        dramCtl->DX[0].DATXIOC[8] = 0x10; // 0x01C63330
        dramCtl->DX[0].DATXIOC[9] = 0xF01; // 0x01C63334

        dramCtl->DX[1].BDL6 = 0x01000000; // 0x01C633BC
        dramCtl->DX[1].DATXIOC[0] = 0x11; // 0x01C63390
        dramCtl->DX[1].DATXIOC[1] = 0x11; // 0x01C63394
        dramCtl->DX[1].DATXIOC[2] = 0x11; // 0x01C63398
        dramCtl->DX[1].DATXIOC[3] = 0x11; // 0x01C6339C
        dramCtl->DX[1].DATXIOC[4] = 0x111; // 0x01C633A0
        dramCtl->DX[1].DATXIOC[5] = 0x111; // 0x01C633A4
        dramCtl->DX[1].DATXIOC[6] = 0x111; // 0x01C633A8
        dramCtl->DX[1].DATXIOC[7] = 0x111; // 0x01C633AC
        dramCtl->DX[1].DATXIOC[8] = 0x11; // 0x01C633B0
        dramCtl->DX[1].DATXIOC[9] = 0xA01; // 0x01C633B4

        // dramCtl->DX[2].BDL6 = 0x01000000; // 0x01C6343C
        dramCtl->DX[2].DATXIOC[0] = 0x110; // 0x01C63410
        dramCtl->DX[2].DATXIOC[1] = 0x11; // 0x01C63414
        dramCtl->DX[2].DATXIOC[2] = 0x111; // 0x01C63418
        dramCtl->DX[2].DATXIOC[3] = 0x110; // 0x01C6341C
        dramCtl->DX[2].DATXIOC[4] = 0x110; // 0x01C63420
        dramCtl->DX[2].DATXIOC[5] = 0x110; // 0x01C63424
        dramCtl->DX[2].DATXIOC[6] = 0x110; // 0x01C63428
        dramCtl->DX[2].DATXIOC[7] = 0x110; // 0x01C6342C
        dramCtl->DX[2].DATXIOC[8] = 0x10; // 0x01C63430
        dramCtl->DX[2].DATXIOC[9] = 0xB00; // 0x01C63434

        dramCtl->DX[3].BDL6 = 0x01000000; // 0x01C634BC
        dramCtl->DX[3].DATXIOC[0] = 0x111; // 0x01C63490
        dramCtl->DX[3].DATXIOC[1] = 0x11; // 0x01C63494
        dramCtl->DX[3].DATXIOC[2] = 0x11; // 0x01C63498
        dramCtl->DX[3].DATXIOC[3] = 0x111; // 0x01C6349C
        dramCtl->DX[3].DATXIOC[4] = 0x111; // 0x01C634A0
        dramCtl->DX[3].DATXIOC[5] = 0x111; // 0x01C634A4
        dramCtl->DX[3].DATXIOC[6] = 0x111; // 0x01C634A8
        dramCtl->DX[3].DATXIOC[7] = 0x111; // 0x01C634AC
        dramCtl->DX[3].DATXIOC[8] = 0x11; // 0x01C634B0
        dramCtl->DX[3].DATXIOC[9] = 0xC01; // 0x01C634B4

        dramCtl->ZQ[0].C &= ~0x00FFFFFF; // 0x01C63140
        dramCtl->ZQ[0].C |= 0x003B3BBB; // 0x01C63140
        dramCtl->PI = 0x5F2; // 0x01C63000
        dramCtl->PI |= 1; // 0x01C63000
        while((dramCtl->PGS[0]&1) == 0); // 0x01C63010
        dramCom->unknown1[24] |= (1<<31); // 0x01C620D0
        while((dramCtl->status&1) == 0); // 0x01C63018
        dramCtl->PGC[3] &= ~(0x6<<24); // 0x01C6310C
        dramCtl->DTC &= ~(3<<24); // 0x01C630C0
        dramCtl->DTC |= (1<<24); // 0x01C630C0
        dramCtl->PI = 0x401; // 0x01C63000
        while((dramCtl->PGS[0]&1) == 0); // 0x01C63010
        dramCtl->ZQ[0].C |= (1<<31); // 0x01C63140

        if(dramCtl->PGS[0]&0xFF0000) // 0x01C63010
            puts("[FAIL] DRAM");
        else
            puts("[ OK ] DRAM");
    }
};
