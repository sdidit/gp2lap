//------------------------------------------------------------------

#define SYNCDELAY 750
#define MAXPLAYERS 100

#define WAITPHASE 1
#define INVITEPHASE 2

static ulong linkmode = 0;
static char nullnode[6] = {0, 0, 0, 0, 0, 0};

static netplayer_info myself;
static netplayer_info *LinkMate;

static long freeslot;
static long numplayers;
static long slots[MAXPLAYERS];  // -1 == NULL
static long foundlist[MAXPLAYERS];  // seq. Liste mit players[] indizes
static long focus = 0;         // idx in foundlist[]
static long wanted = 0;        // idx in players[]
static netplayer_info players[MAXPLAYERS];

void init_playerslots(void)
{
  long i;
  freeslot = 0;
  numplayers = 0;
  for (i = 0; i < (MAXPLAYERS-1); i++) slots[i] = i + 1;
  slots[MAXPLAYERS-1] = -1;
  for (i = 0; i < MAXPLAYERS; i++) foundlist[i] = -1;
}

long get_playerslot(void)
// Rckgabe: -1 == alles voll, sonst slot to be used
{
  long retslot = freeslot;
  if (freeslot != /* > */ -1) {     // slot available?
    freeslot = slots[freeslot];
    slots[retslot] = -2;       // mark used
    foundlist[numplayers++] = retslot;
  }
  return retslot;
}

void free_playerslot(long slot)
// šbergabe: slot == freizugebender slot
{
  long i;
  for (i = 0; i < numplayers; i++) {  // update foundlist[]
    if (foundlist[i] == slot) {
      for ( ; i < (numplayers-1); i++) {
        foundlist[i] = foundlist[i+1];
      }
      numplayers--;
      break;
    }
  }
  slots[slot] = freeslot;
  freeslot = slot;
}

ulong is_in_playerstab(local_address *pnode)
// šbergabe: pnode == zu suchender player (ident. anhand node)
// Rckgabe: 0 == not in table
//           sonst tableindex+1 !!!
{
  long i;
  for (i = 0; i < numplayers; i++) {  //MAXPLAYERS?????
    if (!memcmp(&players[foundlist[i]].node, pnode, sizeof(players[0].node)))
      return i+1;
  }
  return 0;
} // is_in_playerstab()


//-----------------------------

static char *MakeNodeStr(void *node)
{
  static char nodestr[20];
  unsigned char *nf = (unsigned char *)node;
  _bprintf(&nodestr, sizeof(nodestr), "%02X%02X%02X%02X%02X%02X", nf[0], nf[1], nf[2], nf[3], nf[4], nf[5]);
  return &nodestr;
}

static char *MakeNetStr(void *netw)
{
  static char netwstr[20];
  unsigned char *nf = (unsigned char *)netw;
  _bprintf(&netwstr, sizeof(netwstr), "%02X%02X%02X%02X", nf[0], nf[1], nf[2], nf[3]);
  return &netwstr;
}

ulong ask_user(char *msg)
// šbergabe: auszugebende msg (auf selber zeile)
// Rckgabe: 1 == ja, 0 == nein
{
  int ch;
  do {
    cprintf("\r%78s\r%s [Y/N] ", "", msg);
    ch = toupper(getche());
  } while ( !( (ch == 'Y') || (ch == 'N') ) );

  return (ch == 'Y');
} // ask_user()

void add_player(netplayer_info *newplayer)
// šbergabe: newplayer == player to add (bei tab voll -> ignoriert)
// Rckgabe; -
{
  ulong i;
  if (!is_in_playerstab((local_address *)&newplayer->node)) { // only new players
    if ((i = get_playerslot()) != -1) { // find a free slot
      // found a free slot (i)
      memcpy(&players[i], newplayer, sizeof(players[0]));
      if (LogIpxInfo) {
        sprintf(strbuf, "- IpxLink: added player \"%s\" (%s:%s)\n",
          &newplayer->callsign, MakeNetStr(&newplayer->server), MakeNodeStr(&newplayer->node));
        LogLine(strbuf);
      }
    }
  }
} // add_player()

void remove_player(netplayer_info *delplayer)
// šbergabe: delplayer == player to remove from tab
// Rckgabe: -
{
  ulong idx;
  if ((idx = is_in_playerstab((local_address *)&delplayer->node))) { // only recog. players
    free_playerslot(idx-1);
    if (LogIpxInfo) {
      sprintf(strbuf, "- IpxLink: player \"%s\" (%s:%s) left\n",
        &delplayer->callsign, MakeNetStr(&delplayer->server), MakeNodeStr(&delplayer->node));
      LogLine(strbuf);
    }
  }
} // remove_player()

void invite_player(ulong target)
// šbergabe: target == idx of player to invite within players[] tab
// Rckgabe: -
{
  netplayer_info *dest = &players[target];
  sequence_packet invitepacket;

  //if (linkmode != WAITPHASE) return;

  if (LogIpxInfo) {
    sprintf(strbuf, "- IpxLink: now trying to invite\n"); LogLine(strbuf);
    sprintf(strbuf, "  callsign: \"%s\"\n" \
                    "  network:node: (%s:%s)\n",
      dest->callsign, MakeNetStr(&dest->server), MakeNodeStr(&dest->node) );
    LogLine(strbuf);
  }

  memset(&invitepacket, 0, sizeof(invitepacket));
  invitepacket.type = PID_INVITE;
  memcpy(&invitepacket.player, &myself, sizeof(invitepacket.player));  // meine Daten
  ipx_send_packet_data((ubyte *)&invitepacket, sizeof(invitepacket),
                        (ubyte *)&dest->server, (ubyte *)&dest->node,
                         (ubyte *)&dest->node); // OPTIMIZE THAT!! (immediate address)
} // invite_player()

ulong invited_by_player(netplayer_info *src)
// šbergabe: src == player, der mich eingeladen hat...
// Rckgabe: 0 == abgelehnt
//           1 == zugestimmt
{
  ulong ergo;
  sequence_packet replypacket;

  if (LogIpxInfo) {
    sprintf(strbuf, "- IpxLink: received an invitation from\n"); LogLine(strbuf);
    sprintf(strbuf, "  callsign : \"%s\"\n" \
                    "  network:node : (%s:%s)\n",
      src->callsign, MakeNetStr(&src->server), MakeNodeStr(&src->node) );
    LogLine(strbuf);
  }

  sprintf(strbuf, "- Invited by \"%s\" (%s). Agree?", &src->callsign, MakeNodeStr(&src->node));
  ergo = ask_user(&strbuf);

  //--- set up a confirmation packet -----
  memset(&replypacket, 0, sizeof(replypacket));
  if (ergo) replypacket.type = PID_AGREE;
      else  replypacket.type = PID_REFUSE;
  memcpy(&replypacket.player, &myself, sizeof(replypacket.player));
  ipx_send_packet_data((ubyte *)&replypacket, sizeof(replypacket),
                        (ubyte *)&src->server, (ubyte *)&src->node,
                         (ubyte *)&src->node); // OPTIMIZE THAT!! (immediate address)

  return ergo;    // we'll leave completely if ergo == 1...

} // invited_by_player()


void send_joinpacket(void)
// šbergabe: -
// Rckgabe: -
{
  sequence_packet joinpacket;
  memset(&joinpacket, 0, sizeof(joinpacket));
  joinpacket.type = PID_JOIN;
  memcpy(&joinpacket.player, &myself, sizeof(joinpacket.player));
  ipx_send_broadcast_packet_data( (ubyte *)&joinpacket, sizeof(joinpacket));
} // send_joinpacket()

void send_leavepacket(void)
// šbergabe: -
// Rckgabe: -
{
  sequence_packet leavepacket;
  memset(&leavepacket, 0, sizeof(leavepacket));
  leavepacket.type = PID_LEAVE;
  memcpy(&leavepacket.player, &myself, sizeof(leavepacket.player));
  ipx_send_broadcast_packet_data( (ubyte *)&leavepacket, sizeof(leavepacket));
} // send_leavepacket()

netplayer_info *listen_players(void)
// šbergabe: -
// Rckgabe: NULL == no link established
//           sonst ptr auf linkmate
{
  int size;
  sequence_packet *inpacket = NULL;
  ubyte pignore[6] = {0,0,0,0,0,0};
  static ubyte packet[IPX_MAX_DATA_SIZE];  // muss static sein!!!!

  if (!Network_active)
    return NULL;

  inpacket = (sequence_packet *)&packet;
  size = ipx_get_packet_data( packet );
  while ( size > 0 ) {

    //network_process_packet( packet, size );
    switch (inpacket->type) {
        case PID_JOIN :
            //=== got a JOIN PACKET =====
            add_player(&inpacket->player);
            break;
        case PID_LEAVE :
            //=== got a JOIN PACKET =====
            remove_player(&inpacket->player);
            break;
        case PID_INVITE :
            //=== got an INVITATION PACKET =====
            if (linkmode == WAITPHASE) {
                if (memcmp(&pignore, &inpacket->player.node, sizeof(pignore)))
                  if (invited_by_player(&inpacket->player))
                    return &inpacket->player;  // ok, lets sync
                  else {
                    init_playerslots();  // init freelist
                    // naja...„ndern
                    memcpy(&pignore, &inpacket->player.node, sizeof(pignore)); // for ignoring outstanding packets
                  }
            }
            break;
        case PID_AGREE :
            //=== got an AGREE PACKET =====
            if (linkmode == INVITEPHASE) {
              if ( !memcmp(&inpacket->player.node, &players[wanted].node, sizeof(players[0].node)) ) {  // was it my target
                //--- ok, lets play ----
                linkmode = WAITPHASE;
                return &inpacket->player;  // ok, lets sync
              }
            }
            break;
        case PID_REFUSE :
            //=== got a REFUSE PACKET =====
            if (linkmode == INVITEPHASE) {
              if ( !memcmp(&inpacket->player.node, &players[wanted].node, sizeof(players[0].node)) ) {  // was it my target
                cprintf("\r%78s\r- Player \"%s\" refused to play with you", "", &players[wanted].callsign);
                delay(2500);
                linkmode = WAITPHASE;
                return NULL;
              }
            }
            break;
        default:
            break;
    }

    size = ipx_get_packet_data( packet );
  }
  return NULL;
}


ulong sync_players(netplayer_info *mate)
// šbergabe: mate == ....
// Rckgabe: 1 == sync ok, sonst 0
{
  ulong i = 10, sync = 0;
  int size;
  sequence_packet syncpacket;
  ubyte packet[IPX_MAX_DATA_SIZE];
  sequence_packet *inpacket = (sequence_packet *)&packet;

  cprintf("\r%78s\r- Now synchronizing with \"%s\"", "", &mate->callsign);

  //--- send sync packet ------
  memset(&syncpacket, 0, sizeof(syncpacket));
  syncpacket.type = PID_SYNC;
  memcpy(&syncpacket.player, &myself, sizeof(syncpacket.player));  // reply with my vals
  ipx_send_packet_data((ubyte *)&syncpacket, sizeof(syncpacket),
                        (ubyte *)&mate->server, (ubyte *)&mate->node,
                         (ubyte *)&mate->node); // OPTIMIZE THAT!! (immediate address)
  while (i--) {
    //--- process packets -----
    size = ipx_get_packet_data( packet );
    while ( size > 0 ) {
      if (inpacket->type == PID_SYNC) {
        if ( !memcmp(&mate->node, &inpacket->player.node, 6) ) {
          sync = 1;
          i = 0;  // leave outer loop
          break;
        }
      }
      size = ipx_get_packet_data( packet );
    }
    delay(500);
  }

  if (sync == 1) {
    printf("\r%78s\r- Successfully synchronized with \"%s\"", "", &mate->callsign);
  }
  else {
    cprintf("\r%78s\r- Error: Timeout while synchronizing...", "");
    delay(2500);
  }

  return sync;
}


netplayer_info *ipx_sync_linkmate(void)
// šbergabe: -
// Rckgabe: ptr auf linkmates netplayer_info
{
    static char spinchars[] = {"-\\|/"};
    ulong sometimer, sync = 0;
    ulong i=0, namelen;
    int size, ch;

    //---- set up my own netplayer_info ---------
    if ((namelen = strlen(GetCfgString("MyLinkName"))) == 0) {
      puts("- Error: you must specify a name under MyLinkName within gp2lap.cfg");
      return NULL;
    }
    memset(&myself, 0, sizeof(myself));
    _bprintf(&myself.callsign, sizeof(myself.callsign),
             "%.15s%s", GetCfgString("MyLinkName"), namelen > 20 ? ".." : "");
    memcpy(&myself.server, ipx_get_my_server_address(), sizeof(myself.server));
    memcpy(&myself.node, ipx_get_my_local_address(), sizeof(myself.node));

    if (LogIpxInfo)
      LogLine("- IpxLink: searching players now...\n");

    sync = 0; sometimer = 0;  // still to be done
    linkmode = WAITPHASE;
    memset(&players, 0, sizeof(players));
    init_playerslots();  // init freelist

    //---- now listen with my esr ------
    ipx_now_stoplisten();   // for re-loops
    ipx_now_startlisten(0L);

    //=============================================
    //=== now wait for incoming request ===========
    //=============================================
    while (!sometimer) {

      //--- cyclic broadcasts or invitation -------
      switch (linkmode) {
          case WAITPHASE :
              send_joinpacket();
              break;
          case INVITEPHASE :
              invite_player(wanted);
              break;
      }

      //--- give some screen info --------
      switch (linkmode) {
          case WAITPHASE :
              if (numplayers > 0) {
                cprintf("\r%78s\r- Found %d player%s. Choose player %s[\"%s\" (%s)]%s", "", numplayers, numplayers > 1 ? "s" : "",
                focus>0 ? "" : "-", &players[foundlist[focus]].callsign, MakeNodeStr(&players[wanted].node), focus<(numplayers-1) ? "" : "-" );
              } else {
                cprintf("\r%78s\r- Searching players [%c]", "", spinchars[i++ & 0x03]);
              }
              break;
          case INVITEPHASE :
              cprintf("\r%78s\r- Awaiting reply from \"%s\" [%c]", "", &players[wanted].callsign, spinchars[i++ & 0x03]);
              break;
      }

      if ( (LinkMate = listen_players()) ) {
        if ((sync = sync_players(LinkMate))) break; // ok, lets play
        init_playerslots();  // init freelist
      }


      if (kbhit()) {
        switch (getch()) {
            case 0x0D :  // enter
                if ( (linkmode == WAITPHASE) && (numplayers > 0) ) {
                  linkmode = INVITEPHASE;
                  wanted = foundlist[focus];
                  invite_player(wanted);  // auch cyclic oben
                }
                break;
            case 0x1B :  // escape
                switch (linkmode) {
                    case WAITPHASE :  // ok, leave directly
                        send_leavepacket();
                        return NULL;
                        break;
                    case INVITEPHASE :  // ...ask
                        sprintf(strbuf, "- I'm currently waiting for \"%s\"'s reply. Abort?", &players[wanted].callsign);
                        if (!ask_user(strbuf)) break;
                        //send_leavepacket();  // ok, leave directly
                        //return NULL;
                        init_playerslots();  // init freelist
                        linkmode = WAITPHASE;
                        break;
                }
                break;
            case 0 : // extended keycode
                if (linkmode == WAITPHASE) {
                    switch(getch()) {
                        case 0x48 : // up
                            if (focus > 0) focus--;
                            break;
                        case 0x50 : // down
                            if (focus < (numplayers-1)) focus++;
                            break;
                    }
                }
                break;
        }
      }
      delay(SYNCDELAY);
    }

    ipx_now_stoplisten();
    send_leavepacket();

    if (LogIpxInfo) {
      sprintf(strbuf, "- IpxLink %s\n", sync ? "established with" : "failed with");
      LogLine(strbuf);
      if (LinkMate) {
        sprintf(strbuf, "  callsign: \"%s\"\n" \
                        "  network:node: (%s:%s)\n",
          &LinkMate->callsign, MakeNetStr(&LinkMate->server), MakeNodeStr(&LinkMate->node) );
        LogLine(strbuf);
      }
    }
    //cprintf("\n\rlinkestablished!\n");
    //puts("");

    return (sync) ? LinkMate : NULL;
}
