#include "../stdinc.h"
#include "../basiclog.h"
#include "../dpmi.h"
#include "../cfgmain.h"
#include "vesa.h"

//------ the cfg switches --------------
unsigned long ForcePMBanking = 0;    // by default: no
unsigned long ForceLFBuffering = 0;  // by default: no


//----- can't be set anymore, sorry; only for compatibility -----
unsigned char *pVesaPMFunc5 = NULL;
unsigned char *pLFBuf = NULL;


// to be used within copy6.inc to define how many lines to insert from picbuf[]
unsigned long CopySvgaLinesNum = CPYLINESNUMDEFAULT;

unsigned short Mode0x101WindowGranularity = 64;


//------------------------------------------------------------------------

void SetCopySvgaLinesNum(unsigned long lines)
{
  CopySvgaLinesNum = (lines > CPYLINESNUMMAX) ? CopySvgaLinesNum : lines;

  if ( GetCfgULong("LogVesaInfo") && *GetCfgULong("LogVesaInfo") ) {
    sprintf(strbuf, "- CopySvgaLinesNum set to %u\n", CopySvgaLinesNum);
    LogLine(strbuf);
  }

} // SetCopySvgaLinesNum()

//------------------------------------------------------------------------

unsigned long GetCopySvgaLinesNum(void)
{
  return CopySvgaLinesNum;

} // GetCopySvgaLinesNum()


//------------------------------------------------------------------------


unsigned short GetSvgaGranularity(void)
{
  return Mode0x101WindowGranularity;

} // GetSvgaGranularity()


//------------------------------------------------------------------------


void initvesa(void)
{
  dpmi_real_regs vesacallregs;
  struct VesaModeInfo *pvminfo;

  //---- 11/99 get the vesa mode 0x101 granularity (for screenshots)

  pvminfo = dpmi_get_temp_low_buffer(sizeof(struct VesaModeInfo));

  if (pvminfo && !((unsigned long)pvminfo & 0xF) ) {  // nur wenn DOS mem bereit und (!) unteres Nibble==0 (sonst Probleme in vesa_modeinfo())

    memset((void *)&vesacallregs, 0, sizeof(vesacallregs));
    if (vesa_modeinfo(0x0101, pvminfo, (void *)&vesacallregs)) {
      //----- transfer to "normal" memory --------------
      Mode0x101WindowGranularity = pvminfo->WindowGranularity;
      if ( GetCfgULong("LogVesaInfo") && *GetCfgULong("LogVesaInfo") ) {
        sprintf(strbuf, "- InitVesa(): 0x101 WindowGranularity==%u\n", pvminfo->WindowGranularity);
        LogLine(strbuf);
        sprintf(strbuf, "- InitVesa(): 0x101 WindowSize==%u\n", pvminfo->WindowSize); // sollte 64 sein
        LogLine(strbuf);
      }
    }
  }
  else {
    sprintf(strbuf, "- InitVesa() Error: pvminfo==%P\n", pvminfo);
    LogLine(strbuf);
  }


/*----- NOT USED -----

  //FILE *vesaout = NULL;
  unsigned short vesainfosel = 0;    // fr _dos_memalloc ..... vinf
  unsigned short errcode = 0;
  struct VesaInfo *rmaddr = NULL;
  unsigned long mysize = 0;

  if (!ForcePMBanking)  // try to switch it on?
    return;

  ForcePMBanking = 0;   // erstmal aus

  //----- realmode mem allokieren -----------
  //if (_dos_allocmem( (sizeof(vinf) + 15) >> 4, &vesainfosel) == 0) {
  //if ((rmaddr = D32DosMemAlloc(sizeof(vinf))) != NULL) {
  rmaddr = (struct VesaInfo *)dpmi_real_malloc(sizeof(vinf), &vesainfosel);
  if (rmaddr != NULL) {

    //rmaddr = (struct VesaInfo *)(hsel_to_linsegaddr(vesainfosel));
    #ifndef SETVESALOGQUIET
        sprintf(strbuf, "  VESA: rm mem (%u paragraphs) allocated at 0x%08P\n", (sizeof(vinf) + 15) >> 4, rmaddr);
        LogLine(strbuf);
    #endif

    //------ init structures -------------
    memset(rmaddr, 0, sizeof(struct VesaInfo));
    memset((void *)&rmi, 0, sizeof(rmi));

    //----- Informationen ber den vesatreiber einholen ------
    rmaddr->Signature = 0x32454256; // "VBE2"
    if (vesa_getinfo((void *)rmaddr, (void *)&rmi)) {

      //if ((vesaout = fopen("vesa.out", "wb+")) != NULL) {
      //  fwrite(rmaddr, sizeof(struct VesaInfo), 1, vesaout);
      //  fclose(vesaout);
      //}

      //----- transfer to "normal" memory --------------
      memcpy((void *)&vinf, (void *)rmaddr, sizeof(vinf));
      pVI = &vinf;

      //----- valid vesa signature? --------------
      if (vinf.Signature == 0x41534556) {  // "VESA" ?

        #ifndef SETVESALOGQUIET
            sprintf(strbuf, "  VESA: VBE information found\n" \
                            "    Version: %d.%d  " \
                            "    OemString: %s\n", HIBYTE(vinf.Version), LOBYTE(vinf.Version), (char *) ( ((unsigned long)(vinf.OEMNameSegment) << 4) + vinf.OEMNameOffset) );
            LogLine(strbuf);
        #endif


        //----- any usefull pm stuff? --------------
        if (vinf.Version >= 0x0200) {
          memset((void *)&rmi, 0, sizeof(rmi));

          //----- determine pm entry for Func5 ---------
          pVesaPMFunc5 = (unsigned char *) vesa_getpmfunc5((void *)&rmi);
          #ifndef SETVESALOGQUIET
              sprintf(strbuf, "  VESA: %spm bank switching at 0x%08X%s\n", pVesaPMFunc5 ? "using " : "",
                              pVesaPMFunc5, pVesaPMFunc5 ? "" : " (error)");
              LogLine(strbuf);
          #endif
          ForcePMBanking = 1;   // ok, then try to use it!
        }
        else {
          #ifndef SETVESALOGQUIET
              sprintf(strbuf, "  VESA: need VBE 2.0 or higher to use that feature...\n" \
                              "        (tip: you should try Scitech's latest UNIVBE driver.)\n");
              LogLine(strbuf);
          #endif
        }
      }
      else {
        #ifndef SETVESALOGQUIET
            sprintf(strbuf, "  VESA: illegal signature found (0x%08X)\n", vinf.Signature);
            LogLine(strbuf);
        #endif
      }
    }   // if vesa get info succeeded
    else {
      #ifndef SETVESALOGQUIET
          sprintf(strbuf, "  VESA: can't get vesa information\n");
          LogLine(strbuf);
      #endif
    }


    if (ForceLFBuffering != 0) {   // try to use lfb too?
      ForceLFBuffering = 0;        // erstmal aus
      if (ForcePMBanking == 1) {    // otherwise it's senseless


        //DetectLFBSupport();
        //===========================================
        //----- now get the vesamode info -----------
        //===========================================
        if (sizeof(vminf) > sizeof(vinf))
          puts("  VESA: Fatal error vminf\n");

        memset((void *)&rmi, 0, sizeof(rmi));
        if (vesa_modeinfo(0x0101, rmaddr, (void *)&rmi)) {
          //----- transfer to "normal" memory --------------
          memcpy((void *)&vminf, (void *)rmaddr, sizeof(vminf));
          pVMI = &vminf;
          //screensize = vesamodeinfo->PixelWidth * vesamodeinfo->PixelHeight * (vesamodeinfo->BitsPerPixel >> 3);
          //if (v640_480_buffer != NULL)
          //  vcpy_vesascr(VCPYSAVE, v640_480_buffer, vesamodeinfo->WindowGranularity, screensize);
          #ifndef SETVESALOGQUIET
              sprintf(strbuf, "  VESA: found mode 0x0101 info (ModeAttr==0x%04X)\n" \
                              "    pLFBPhysBase==0x%P, pOffScreenMem==0x%P, OffScreenMemKB==%u\n",
                              pVMI->ModeAttributes, pVMI->pLFBPhysBase, pVMI->pOffScreenMem, pVMI->OffScreenMemKB );
              LogLine(strbuf);
          #endif

          if (pVMI->ModeAttributes & 0x0081 == 0x81) {  // mode and lfb supported
            //pLFBPhysBase

            mysize = pVI->VideoMemory << 6;   // now in KB's
            mysize <<= 10;  // now in Bytes

            //-------- do the mapping ---------------
            if ((pLFBuf = physaddrmapping(pVMI->pLFBPhysBase, mysize, &errcode)) != NULL) {

              #ifndef SETVESALOGQUIET
                  sprintf(strbuf, "  VESA: mapped physical address to 0x%P\n", pLFBuf);
                  LogLine(strbuf);
              #endif

              //if (video_set_mode(0x4101) == 0) {
              //  #ifndef SETVESALOGQUIET
              //    sprintf(strbuf, "  successfull switched to mode 0x4101\n");
              //    LogLine(strbuf);
              //  #endif
              //
              //  for (i = 0; i < 0x4B000; i++)
              //    pLFBuf[i] = 0x0F;
              //  getch();
              //
              //  ForceLFBuffering = 1; // ok, everything seems to work fine...
              //}
              //video_set_mode(0x03);

              ForceLFBuffering = 1; // ok, everything seems to work fine...
              puts("- Using linear framebuffer");

            }   // if mapping succeeded
            else {
              #ifndef SETVESALOGQUIET
                  sprintf(strbuf, "  VESA: error while physical address mapping (0x%04X)\n", errcode);
                  LogLine(strbuf);
              #endif
            }

          }  // if lfb supported
        }  // if vesa mode info available
      }  // if ForcePMBanking berhaupt an

      #ifdef TEST
        if (!ForceLFBuffering) {
          sprintf(strbuf, "- ForceLFBBuffering has been disabled\n");
          LogLine(strbuf);
        }
      #endif

    }  // if ForceLFBuffering != 0


    if ( !dpmi_real_free(vesainfosel) ) {
      puts("  VESA: error dpmi_real_free()");
    }

  }
  else {
    #ifndef SETVESALOGQUIET
        sprintf(strbuf, "  VESA: can't allocate rm mem (%u paragraphs)\n", (sizeof(vinf) + 15) >> 4);
        LogLine(strbuf);
    #endif
  }

  #ifdef TEST
    if (!ForcePMBanking) {
      sprintf(strbuf, "- ForcePMBanking has been disabled\n");
      LogLine(strbuf);
    }
  #endif

*/

} // initvesa()

//------------------------------------------------------------------------

void donevesa(void)
{

/*----- NOT USED -----

  unsigned short errcode = 0;

  if (ForceLFBuffering == 1) {
    //-------- free mapping ---------------
    if (!freephysaddrmapping(pLFBuf, &errcode)) {
      // some WATCOM FAQ stated: hasn't/could'nt be freed
      #ifndef SETVESALOGQUIET
          sprintf(strbuf, "  VESA: error while free mapping (0x%04X)\n", errcode);
          //LogLine(strbuf);
     #endif
    }
  }

*/

} // donevesa()
