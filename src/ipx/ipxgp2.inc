
//#define FAKELINKMATE

unsigned long LinkEstablished = 0;    // global flag for my link
static int Network_active = 0; // Global indicator
static int socket = 0x0000;    // Global value (ofs to IPX_DEFAULT_SOCKET)

//--- to be set from lammcall ------
rmcodedef **ppDosRmCode = NULL;

//------ some cfg switches --------------
unsigned long UseIpxLink = 0;
unsigned long LogIpxInfo = 0;
unsigned long LogIpxOutpackets = 0;
unsigned long LogIpxInpackets = 0;
unsigned long LogLnkCmdsIn = 0;
unsigned long LogLnkCmdsOut = 0;
unsigned long LogLnkCmdsProcess = 0;
unsigned long UseOldNetworkCode = 0;

#include "../gp2rmcod.h"
#include "ipx.inc"
#include "ipxsync.inc"

#define FAKE_GP2RMCODE

#pragma pack (push, 1)

typedef struct lowesrinfo {
  unsigned long lastecb;
  unsigned short lastsize;
  unsigned long packetcount;
  unsigned long neterrors;
  unsigned long missedecbs;
  unsigned long packetsfromme;
  unsigned long missedpackets;  // Bestimmung anhand der seq. vergebenen Paketnummer
} lowesrinfo;

#pragma pack (pop)

//----- variables within 16bit code -------
extern void MyLowESREntry(void);
extern unsigned long LowESRSize;
extern unsigned long EnterIpx;
extern unsigned long Gp2RmCode;
extern lowesrinfo LowEsrInfo;
extern unsigned char MyOwnNode[6];
extern unsigned char LinkMateNode[6];

//---- other imported stuff ------
extern unsigned short ipx_vector_segment;        // realmode seg der "Novell-Routine"
extern unsigned short ipx_vector_offset;         // realmode ofs der "Novell-Routine"

static netplayer_info *mylinkmate = NULL;   // the person, I'm linked with

//----- global stuff -------
char MyGp2LeadMsg[256];       // msg to be displayed at lead connect
char MyGp2FollowMsg[256];      // msg to be displayed at follow connect

rmcodedef *myrmcode = NULL;         // adr meines fake rm code
unsigned char *esrrmaddr = NULL;    // adr meiner esr
lowesrinfo *statsaddr = NULL;       // adr des stats blocks
unsigned long EsrRmcodeOfs = 0;  // ofs of _Gp2RmCode within esr code
unsigned short rmsel = 0, rmcodesel = 0;
unsigned short errcode;

//------------------------------------------------------------------

unsigned long LinAddrToRmAddr( unsigned char *linaddr );
/* parm: linaddr - the linear address to be converted
   value: NULL == error (can't convert)
          otherwise Seg16:Ofs16 pointer
*/
#pragma aux LinAddrToRmAddr = \
             "movzx ebx,ax" \
             "and ebx,0Fh"  /* that's the rm ofs */ \
             "test eax,0FFF00000h"     /* pointing to real mem? */ \
             "mov ecx,0" \
             "jnz latra_err" \
             "shl eax,12" \
             "mov ax,bx" \
             "mov ecx,eax" \
 "latra_err: " \
parm[EAX] value[ECX] modify [EAX EBX ECX EDX];

//------------------------------------------------------------------

void ipx_gp2_done(void)
// šbergabe: -
// Rckgabe: 0 == ok, sonst fehler
{
  if ( !dpmi_real_free(rmsel) )
    fprintf(stderr, "*** ipx: error dpmi_real_free() rmsel\n");
  if ( !dpmi_real_free(rmcodesel) )
    fprintf(stderr, "*** ipx: error dpmi_real_free() rmcodesel\n");
}

//------------------------------------------------------------------

unsigned long ipx_basic_init(ulong verbose)
// Rckgabe: 1 == ok, sonst 0
{
    static int ipx_error;

    if (LogIpxInfo) LogLine("- Now installing ipx\n");

    //--- now init the IPX --------
    if ((ipx_error = ipx_init(IPX_DEFAULT_SOCKET + socket, verbose)) == 0)     {
        //printf( "- Using Network socket 0x%04X\n", IPX_DEFAULT_SOCKET + socket );
        if (LogIpxInfo) LogLine("- Ipx installed successfully\n");
        Network_active = 1;
    }
    else {
        switch( ipx_error ) {
            case 3:  sprintf(strbuf, "ipx: can't find ipx driver\n"); break;
            case -2: sprintf(strbuf, "ipx: error while using socket 0x%04X.\n", IPX_DEFAULT_SOCKET+socket); break;
            case -4: sprintf(strbuf, "ipx: couldn't allocate low mem\n"); break;
            default: sprintf(strbuf, "ipx: unknown error %d\n", ipx_error );
        }
        fprintf(stderr, "*** %s", strbuf);
        if (LogIpxInfo) {
          LogLine("- ");
          LogLine(strbuf);
        }
        //printf( "Networking has been disabled\n");
        Network_active = 0;             // Assume no network
    }

    return Network_active;  // ret val of global flag
} // ipx_basic_init()

//------------------------------------------------------------------

unsigned long gp2_ipx_init(void)
// Rueckgabe: 0 == ok
//            1 == no network active
//            2 == no linkmate found or search aborted
//            3 == can't get dosmem for esr
//            4 == can't get dosmem for rmcode fake
//            5 == can't lock dosmem for esr
//            6 == can't lock dosmem for rmcode fake
{
    if (!Network_active)
      return 1;

    mylinkmate = ipx_sync_linkmate();    puts("");
    if (mylinkmate == NULL) {
#ifdef TEST
      if (GetCfgULong("FakeLinkMate") && *GetCfgULong("FakeLinkMate") ) {
        static netplayer_info fakeplayer;
        strcpy(fakeplayer.callsign, "Henry Jones");
        fakeplayer.server[0] = 0x12;
        fakeplayer.server[1] = 0x34;
        fakeplayer.server[2] = 0x56;
        fakeplayer.server[3] = 0x78;
        fakeplayer.node[0] = 0xDE;
        fakeplayer.node[1] = 0xAD;
        fakeplayer.node[2] = 0xBE;
        fakeplayer.node[3] = 0xAF;
        fakeplayer.node[4] = 0x12;
        fakeplayer.node[5] = 0x34;
        mylinkmate = &fakeplayer;
      }
      else {
        return 2;
      }
#else
      return 2;     // found noone
#endif
    }

    //============================================
    //==== now set some stuff within RM ESR ======
    //============================================

    //--- LinkMate node address vermerken ---------
    memcpy(&LinkMateNode, &mylinkmate->node, 6);  // set in rm esr
    //--- ipx rm entry setzen ------
    EnterIpx = ((unsigned long)ipx_vector_segment << 16) + ipx_vector_offset;
    //--- meine eigene node setzen -----
    memcpy(&MyOwnNode, ipx_get_my_local_address(), 6);
    if (LogIpxInfo) {
      sprintf(strbuf, "- Ipx entry point at %04X:%04X\n", EnterIpx >> 16, (unsigned short)EnterIpx);
      LogLine(strbuf);
    }

    //----- realmode mem allokieren fr rm esr -----------
    if (LogIpxInfo) {
      sprintf(strbuf, "- Now allocating esr dosmem (%u bytes)\n", LowESRSize);
      LogLine(strbuf);
    }
    esrrmaddr = (unsigned char *)dpmi_real_malloc(LowESRSize, &rmsel);
    if (esrrmaddr == NULL) {
      //puts("- Error: esr can't dpmi_real_malloc()");
      return 3;
    }
    if (!dpmi_lock_region(esrrmaddr, LowESRSize)) {
      dpmi_real_free(rmsel);
      return 5;
    }

#ifdef FAKE_GP2RMCODE
    //----- realmode mem allokieren fr rmcode fake -----------
    if (LogIpxInfo) {
      sprintf(strbuf, "- Now allocating rmf dosmem (%u bytes)\n", sizeof(struct rmcodedef));
      LogLine(strbuf);
    }
    myrmcode = (rmcodedef *)dpmi_real_malloc(sizeof(struct rmcodedef), &rmcodesel);
    if (myrmcode == NULL) {
      //puts("- Error: rmf can't dpmi_real_malloc()");
      dpmi_real_free(rmsel);
      return 4;
    }
    if (!dpmi_lock_region(myrmcode, sizeof(struct rmcodedef))) {
      dpmi_real_free(rmsel);
      dpmi_real_free(rmcodesel);
      return 6;
    }


    //--- weiteres zeugs ---
    EsrRmcodeOfs = ( (ulong)(&Gp2RmCode) - (ulong)(&MyLowESREntry) );
    Gp2RmCode = LinAddrToRmAddr((void *)myrmcode);
    myrmcode->wRcvPtr = 0x00A3;
    myrmcode->wRcvProcPtr = 0x00A3;
    memset(&myrmcode->RcvBuf, 0x90, sizeof(myrmcode->RcvBuf));
    strcpy(&myrmcode->RcvBuf, "Well, looks like a typical gp2 receive buffer!");
#endif

    //---- now activate the esr ------
    statsaddr = (lowesrinfo *) (((ulong)(esrrmaddr))+(ulong)((ulong)&LowEsrInfo-(ulong)&MyLowESREntry));
    memcpy(esrrmaddr, &MyLowESREntry, LowESRSize);

    //---- now listen with my esr ------
    ipx_now_stoplisten();  // just to make sure...
    ipx_now_startlisten(LinAddrToRmAddr(esrrmaddr));

    atexit( ipx_gp2_done );  // remove at shutdown

    return 0;
}

//------------------------------------------------------------------

unsigned long start_ipx_link(void)
// Rckgabe: 1 == succes, sonst 0
{
   ulong linkcode;

   linkcode = gp2_ipx_init();
   switch(linkcode) {
      case 0 :
         LinkEstablished = 1;
         set_gp2string(0x0153, "IPX Linkup Menu");
         set_gp2string(0x017F, "IPX Linkup Options");
         set_gp2string(0x018C, "IPX");
         set_gp2string(0x0197, "IPX Link");
         set_gp2string(0x02E3, "IPX Link Abandoned");
         _bprintf(&MyGp2LeadMsg, sizeof(MyGp2LeadMsg),
                  "\x11 Notifying Linkmate now... \x0A\x0A" \
                  "\x11Player: \"%s\"\x0A" \
                  "\x11[Node: %s:%s]\x0A" \
                  "\x11on socket 0x%04X\x0A\x0A" \
                  "\x11(Escape Aborts)", &mylinkmate->callsign,
                  MakeNetStr(&mylinkmate->server), MakeNodeStr(&mylinkmate->node), IPX_DEFAULT_SOCKET+socket);
         set_gp2string(0x02DE, &MyGp2LeadMsg);
         _bprintf(&MyGp2FollowMsg, sizeof(MyGp2FollowMsg),
                  "\x11 Awaiting notification from... \x0A\x0A" \
                  "\x11Player: \"%s\"\x0A" \
                  "\x11[Node: %s:%s]\x0A" \
                  "\x11on socket 0x%04X\x0A\x0A" \
                  "\x11(Escape Aborts)", &mylinkmate->callsign,
                  MakeNetStr(&mylinkmate->server), MakeNodeStr(&mylinkmate->node), IPX_DEFAULT_SOCKET+socket);
         set_gp2string(0x02DF, &MyGp2FollowMsg);
         break;
      case 1 :  sprintf(strbuf, "*** link: no network loaded\n"); break;
      case 2 :  sprintf(strbuf, "*** link: can't find a linkmate\n"); break;
      //case 3 :  sprintf(strbuf, "*** link: ipxinit error 0003\n"); break;
      //case 4 :  sprintf(strbuf, "*** link: ipxinit error 0004\n"); break;
      default : sprintf(strbuf, "*** link: ipxinit error %u\n", linkcode); break;
   }
   if (linkcode)
     fprintf(stderr, strbuf);
   else {
     sprintf(strbuf, "- IpxLink: now playing with\n" \
                     "  callsign : \"%s\"\n  network:node : (%s:%s)\n",
            mylinkmate->callsign, MakeNetStr(&mylinkmate->server), MakeNodeStr(&mylinkmate->node) );
   }

   if (LogIpxInfo) LogLine(strbuf);

   return (linkcode) ? 0 : 1;
} // start_ipx_link()

//------------------------------------------------------------------

#pragma aux AHFTryToReceiveQ parm [EAX] caller;
#pragma aux AHFTryToReceive parm [EAX] caller;
#pragma aux AHFComSendCharNow parm [EDI] caller;

//-------------------------------------------------------------------

void __near _cdecl AHFTryToReceiveQ(unsigned long who)
{
  if (LogIpxInpackets) {
    sprintf(strbuf, "TryToReceive left by %u\n", who);
    LogLine(strbuf);
  }
} // AHFTryToReceiveQ()

void __near _cdecl AHFTryToReceive(unsigned long who)
{
  static ushort size;
  static rmcodedef *rmcode;
  static gp2packet *inpacket = NULL;
  static DWORD i, versuche;
  static BYTE packet[1024]; // Achutng: eigentlich [IPX_MAX_DATA_SIZE]
  static ushort nextpos;

  if (!Network_active)
    return;

  if ((rmcode = *ppDosRmCode) != NULL) {

    if (LogIpxInpackets) {
      sprintf(strbuf, "TryToReceive called by %u\n", who);
      LogLine(strbuf);
    }

    //---- is there a packet present? ----------
    size = ipx_get_packet_data( packet );
    if (size > 0) {
      inpacket = (gp2packet *)&packet;
      if (inpacket->type == PID_PDATA) {

        if (LogIpxInpackets) {
          sprintf(strbuf, "Packet received: %u bytes\n", inpacket->len);
          LogLine(strbuf);
        }

        //---- now take data and store into gp2buf --------
        for (i = 0; i < inpacket->len; i++) {

          // where will the _next_ char be stored?
          nextpos = rmcode->wRcvPtr + 1;
          if (nextpos >= 0x01A3) nextpos = 0x00A3;
          if (nextpos == rmcode->wRcvProcPtr)
            break;

          *(BYTE *)((long)rmcode+(rmcode->wRcvPtr)) = inpacket->data[i];
          rmcode->wRcvPtr = nextpos;

          if (LogIpxInpackets) {
            sprintf(strbuf, "  0x%02X", inpacket->data[i]);
            LogLine(strbuf);
          }
        }
        if (LogIpxInpackets) LogLine("\n");
      }
      else {
        LogLine("  wrong packet received (type != PID_PDATA)\n");
      }
    } else {
      if (LogIpxInpackets)
        LogLine("Invoked but no packet received (waist!!!!)\n");
    }

  }
} // AHFTryToReceive()

//-------------------------------------------------------------------

void __near _cdecl AHFComSendCharNow(rmcodedef *rmcode)
{
  if (rmcode->bBytesToSend != 0) {

    // now send the chars!
    static gp2packet outpacket;
    static DWORD i;

    //sprintf(strbuf, "ptrvergl: ppDosRmCode==%P rmcode==%P\n", *ppDosRmCode, rmcode);
    //LogLine(strbuf);

    //---- setup the packet to be sent --------

    outpacket.type = PID_PDATA;   // former PID_REQUEST;
    outpacket.len = rmcode->bBytesToSend;
    for (i = 0; i < outpacket.len; i++) {
      outpacket.data[i] = *(BYTE *)((long)rmcode+(rmcode->wSendPtr));
      //rmcode->bBytesToSend--;
      if (++rmcode->wSendPtr >= 0x00A3) rmcode->wSendPtr = 0x002B;
    }
    rmcode->bBytesToSend = 0;


    //---- do some logging, if requested ------

    if (LogIpxOutpackets) {
      sprintf(strbuf, "SendCharNow called: %u bytes\n", outpacket.len);
      LogLine(strbuf);
      for (i = 0; i < outpacket.len; i++) {  // tierischer slowdown!!!
        sprintf(strbuf, "  0x%02X", outpacket.data[i]);
        LogLine(strbuf);
      }
      LogLine("\n");
    }

    //rmcode->bool_1AE = 0xFF;

    //ipx_send_broadcast_packet_data( (ubyte *)&outpacket, sizeof(outpacket) );
    if (UseOldNetworkCode) {
      ipx_send_packet_data((ubyte *)&outpacket, sizeof(outpacket),
                            (ubyte *)&mylinkmate->server, (ubyte *)&mylinkmate->node,
                             (ubyte *)&mylinkmate->node); // OPTIMIZE THAT!! (immediate address)
    } else {
      ipx_send_packet_data2((ubyte *)&outpacket, sizeof(outpacket),
                            (ulong *)&mylinkmate->server, (ubyte *)&mylinkmate->node,
                             (ubyte *)&mylinkmate->node); // OPTIMIZE THAT!! (immediate address)
    }
  }
} // AHFComSendCharNow()

//-------------------------------------------------------------------

void (__near _cdecl *fpAHFTryToReceiveQ)(unsigned long) = AHFTryToReceiveQ;
void (__near _cdecl *fpAHFTryToReceive)(unsigned long) = AHFTryToReceive;
void (__near _cdecl *fpAHFComSendCharNow)(rmcodedef*) = AHFComSendCharNow;
