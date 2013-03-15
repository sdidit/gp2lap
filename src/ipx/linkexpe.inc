//-------------------------------------------------------------------

/*
void __near _cdecl mytestpacket(void)
{
    static sequence_packet mypacket;

    return;

    mypacket.type = PID_REQUEST;
    strcpy(mypacket.player.callsign, "Frank");

    ipx_send_broadcast_packet_data( (ubyte *)&mypacket, sizeof(sequence_packet) );
}
*/

//-------------------------------------------------------------------

void (__near _cdecl *fpSendPacket)(void) = mytestpacket;

#pragma aux AHFDirectReceive parm [EAX] caller value [EAX];
#pragma aux AHFMyPICmds parm [EBX] caller;
#pragma aux AHFWithinStoreRcvdCmd parm [ESI] [EBX] caller;
#pragma aux AHFWithinSendCmdInSlotAL parm [EAX] caller;  // new try!

//-------------------------------------------------------------------

void __near _cdecl AHFMyPICmds(unsigned char *cmdptr)
{
  static DWORD i, len;

  if (!LogLnkCmdsProcess)
    return;

  sprintf(strbuf, "Processing_Cmd: [%02X]", *cmdptr);
  LogLine(strbuf);

  switch (*cmdptr) {   // get cmd length
    case 0x00 : len = 10; break;
    case 0x01 : len = 10; break;
    case 0x02 : len = 3;  break;
    case 0x03 : len = 10; break;
    case 0x04 : len = 10; break;
    case 0x05 : len = 10; break;
    case 0x06 : len = 5;  break;
    case 0x07 : len = 2;  break;
    case 0x08 : len = 2;  break;
    case 0x09 : len = 5;  break;
    case 0x0A : len = 2;  break;
    case 0x0B : len = 1;  break;
    case 0x0C : len = 2;  break;
    case 0x0D : len = 9;  break;
    case 0x0E : len = 10; break;
    default :  len = 0;
  }
  if (len) {
    for (i = 1; i < len; i++) {
      cmdptr++;
      sprintf(strbuf, " %02X", *cmdptr);
      LogLine(strbuf);
    }
    LogLine("\n");
  }
  else {
    LogLine(" Error (unknown command)\n");
  }
}

//-------------------------------------------------------------------

void __near _cdecl AHFWithinStoreRcvdCmd(unsigned long rmcode,	 // nciht tasuchen!
					  unsigned long ofs)
{
  static BYTE *myptr;
  static DWORD i, len;

  if (!LogLnkCmdsIn)
    return;

  //myptr = (BYTE *)((unsigned long)(rmcode)+(unsigned long)(ofs));
  if (++ofs >= 0x01A3) ofs = 0x00A3;
  len = *(BYTE *)(rmcode+ofs); // sollte zw 4 und 120 liegen

  if (++ofs >= 0x01A3) ofs = 0x00A3;
  myptr = (BYTE *)(rmcode+ofs);
  sprintf(strbuf, "Received___Cmd: [%02X]", *myptr);  // hdr nicht mit...
  LogLine(strbuf);

  if ((len >= 4) && (len <= 120)) {
    len -= 4;
    for (i = 1; i < len; i++) {
      if (++ofs >= 0x01A3) ofs = 0x00A3;
      sprintf(strbuf, " %02X", *(BYTE *)(rmcode+ofs));
      LogLine(strbuf);
    }
    LogLine("\n");
  }
  else {
    sprintf(strbuf, "Error (length invalid (%u bytes)\n", len);  // hdr nicht mit...
    LogLine(strbuf);
  }

}

//-------------------------------------------------------------------

void __near _cdecl AHFWithinSendCmdInSlotAL(BYTE slotnr)
{
  static BYTE *myptr;
  static DWORD i, len;

  if (!LogLnkCmdsOut)
    return;

  myptr = &t_CmdsToBeSend->slot[slotnr & 0x07].data;
  myptr++;
  len = *myptr++;

  sprintf(strbuf, "SendCmdInSlot_: [%02X]", *myptr);  // hdr nicht mit...
  LogLine(strbuf);

  if ((len >= 4) && (len <= 120)) {
    len -= 4;
    for (i = 1; i < len; i++) {
      myptr++;
      sprintf(strbuf, " %02X", *myptr);
      LogLine(strbuf);
    }
    LogLine("\n");
  }
  else {
    sprintf(strbuf, "Error (length invalid (%u bytes)\n", len);  // hdr nicht mit...
    LogLine(strbuf);
  }
}

//-------------------------------------------------------------------

unsigned long __near _cdecl AHFDirectReceive(unsigned long who)
{
  static ushort size;
  static rmcodedef *rmcode;
  static gp2packet *inpacket = NULL;
  static DWORD i, versuche;
  static BYTE packet[1024]; // Achutng: eigentlich [IPX_MAX_DATA_SIZE]
  static ushort nextpos;

  if (!Network_active)
    return 0;

  if ((rmcode = *ppDosRmCode) != NULL) {

    if (LogIpxInpackets) {
      sprintf(strbuf, "DirectReceive called by %u\n", who);
      LogLine(strbuf);
    }

    //------ wait for a packet -----------
    versuche = 1000;
    do {
      size = ipx_get_packet_data( packet );
    } while ((size == 0) && (versuche-- != 0));
    if (versuche == 0) {
      if (LogIpxInpackets) LogLine("DirectReceive timeout left\n");
      return 1; // CF == timeout
    }

    inpacket = (gp2packet *)&packet;
    if (inpacket->type == PID_PDATA) {

      if (LogIpxInpackets) {
	sprintf(strbuf, "DirectReceive Packet received: %u bytes (%u) rawsize=%u\n", inpacket->len, versuche, size);
	LogLine(strbuf);
      }

      //---- now take data and store into gp2buf --------
      mymemcpy(pRcvLnkCmd, &inpacket->data[0], inpacket->len);
      for (i = 0; i < inpacket->len; i++) {
	if (LogIpxInpackets) {
	  sprintf(strbuf, "  0x%02X", inpacket->data[i]);
	  LogLine(strbuf);
	}
      }
      if (LogIpxInpackets) LogLine("\n");
    }
    else {
      if (LogIpxInpackets) {
	sprintf(strbuf, "DirectReceive wrong packet type %u (%u bytes) rawsize=\n", inpacket->type, inpacket->len, size);
	LogLine(strbuf);
      }
    }
  }

  if (LogIpxInpackets) {
    sprintf(strbuf, "DirectReceive left by %u\n", who);
    LogLine(strbuf);
  }
  return 0;
}

void (__near _cdecl *fpAHFMyPICmds)(void) = AHFMyPICmds;
void (__near _cdecl *fpAHFWithinStoreRcvdCmd)(unsigned long, unsigned long) = AHFWithinStoreRcvdCmd;
void (__near _cdecl *fpAHFWithinSendCmdInSlotAL)(BYTE) = AHFWithinSendCmdInSlotAL;
void (__near _cdecl *fpAHFDirectReceive)(void) = AHFDirectReceive;
