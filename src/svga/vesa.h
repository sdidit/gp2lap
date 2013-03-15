#ifndef _VESA_H
#define _VESA_H


//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

#define CPYLINESNUMDEFAULT  194
#define CPYLINESNUMMAX      479


// subfunctions for vesa_handle_videostate
#define GET_STATE_BUFFER_SIZE   0x00
#define SAVE_VIDEO_STATES       0x01
#define RESTORE_VIDEO_STATES    0x02

// several states (bitfield in CX) for vesa_handle_videostate
#define VIDEO_HW_STATE         0x0001
#define VIDEO_BIOSDATA_STATE   0x0002
#define VIDEO_DAC_STATE        0x0004
#define VIDEO_SUPERVGA_STATE   0x0008
#define VIDEO_ALL4_STATE       0x000F


//--------------------------------------------------------
//------ data types --------------------------------------
//--------------------------------------------------------

#pragma pack (push, 1)

struct VesaInfo {
        unsigned long       Signature;              // 00h
        unsigned short      Version;                // 04h
        unsigned short      OEMNameOffset;          // 06h    Pointer to OEM name (C000:0xxx)
        unsigned short      OEMNameSegment;         //
        unsigned char       Capabilities[4];        // 0Ah
        unsigned short      SupportedModesOffset;   // 0Eh    Pointer to list of supported VESA and OEM modes (terminated with 0xffff).
        unsigned short      SupportedModesSegment;  //        (C000:0xxx)
        unsigned short      VideoMemory;            // 12h    amount of video memory in 64K blocks
    //--- VBE v1.x (reserved) ---
    //--- VBE v2.0 ---
        unsigned short      OEM_SW_Version;         // 14h
        unsigned long       VendorPtr;              // 16h
        unsigned long       ProductNamePtr;         // 1Ah
        unsigned long       ProductRevPtr;          // 1Eh
        unsigned char       reserved1[222];         // 22h
        unsigned char       OEM_Scratchpad[256];    // 100h
        unsigned char       my_safety[16];          // 200h
};

struct VesaModeInfo {

        unsigned short      ModeAttributes;             // 00h
        unsigned char       WindowAAttributes;          // 02h
        unsigned char       WindowBAttributes;          // 03h
        unsigned short      WindowGranularity;          // 04h
        unsigned short      WindowSize;                 // 06h
        unsigned short      StartSegmentWindowA;        // 08h
        unsigned short      StartSegmentWindowB;        // 0Ah
        void                (*WindowPositioningFunction)(signed long page);   // 0Ch
        unsigned short      BytesPerScanLine;           // 10h

        //Remainder of this structure is optional for VESA modes in v1.0/1.1, needed for OEM modes.

        unsigned short      PixelWidth;                 // 12h
        unsigned short      PixelHeight;                // 14h
        unsigned char       CharacterCellPixelWidth;    // 16h
        unsigned char       CharacterCellPixelHeight;   // 17h
        unsigned char       NumberOfMemoryPlanes;       // 18h
        unsigned char       BitsPerPixel;               // 19h
        unsigned char       NumberOfBanks;              // 1Ah
        unsigned char       MemoryModelType;            // 1Bh
        unsigned char       SizeOfBank;                 // 1Ch
        unsigned char       NumberOfImagePages;         // 1Dh
        unsigned char       Reserved1;                  // 1Eh

        //VBE v1.2+

        unsigned char       RedMaskSize;                // 1Fh
        unsigned char       RedFieldPosition;           // 20h
        unsigned char       GreenMaskSize;              // 21h
        unsigned char       GreenFieldPosition;         // 22h
        unsigned char       BlueMaskSize;               // 23h
        unsigned char       BlueFieldPosition;          // 24h
        unsigned char       ReservedMaskSize;           // 25h
        unsigned char       ReservedMaskPosition;       // 26h
        unsigned char       DirectColourModeInfo;       // 27h

        //VBE v2.0+

        unsigned char      *pLFBPhysBase;               // 28h  physical address of linear video buffer
        unsigned char      *pOffScreenMem;              // 2Ch  pointer to start of offscreen memory
        unsigned short      OffScreenMemKB;             // 30h  KB of offscreen memory

        unsigned char       Reserved2[206];             // 32h
};

#pragma pack (pop)

//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------

//-----------------
//--- published ---
//-----------------

extern unsigned long ForcePMBanking;
extern unsigned long ForceLFBuffering;
extern unsigned char *pVesaPMFunc5;
extern unsigned char *pLFBuf;


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

void SetCopySvgaLinesNum(unsigned long);
unsigned long GetCopySvgaLinesNum();
unsigned short GetSvgaGranularity();

void initvesa();
void donevesa();


//--------------------------------------------------------
//------ some macros -------------------------------------
//--------------------------------------------------------

#include "vpragmas.inc"



#endif

