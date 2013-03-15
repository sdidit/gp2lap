#ifndef _IPX_H
#define _IPX_H


#include <i86.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <conio.h>
#include <assert.h>

// The default socket to use.
#define IPX_DEFAULT_SOCKET 0x5100               // 0x869d
#define IPX_MAX_DATA_SIZE (542)         //(546-4)

//---- new frank 9/98 -----
//#pragma off (check_stack);
//---------

#include "typedefs.h"
#include "ipxstruc.h"


#pragma pack (push, 1)

// ist pracktisch eine nodeaddress
typedef struct local_address {
        ubyte address[6];
} local_address;

// wie wirs kennen: NETWORK:NODE:SOCKET
typedef struct net_address {
        BYTE                            network_id[4];
        local_address   node_id;
        WORD                            socket_id;
} net_address;

//--------------------------------
//--- IPX HEADER definition ------
//--------------------------------
typedef struct ipx_header {
        WORD                    checksum;
        WORD                    length;
        BYTE                    transport_control;
        BYTE                    packet_type;
        net_address     destination;
        net_address     source;
} ipx_header;

//--------------------------------
//--- ECB HEADER definition ------
//--------------------------------
typedef struct ecb_header {
        WORD                    link[2];
        WORD                    esr_address[2];
        BYTE                    in_use;
        BYTE                    completion_code;
        WORD                    socket_id;
        BYTE                    ipx_reserved[14];
        WORD                    connection_id;
        local_address     immediate_address;  // die nodeadr (wohin?)
        WORD                    fragment_count;
        WORD                    fragment_pointer[2];
        WORD                    fragment_size;
} ecb_header;

//----------------------------------
//--- die Paketdaten definition ----
//----------------------------------
typedef struct packet_data {
        int                     packetnum;  // ist der Wert des ZÑhlers "ipx_packetnum", der (angefangen mit 0)
                                            // bei jedem Senden um eins erhîht wird
                                            // -1 seems to be "free"
        byte                    data[IPX_MAX_DATA_SIZE];  // [542]
} packet_data;


//------------------------------------
//--- so und jetzt erst das Paket ----
//------------------------------------
typedef struct ipx_packet {
        ecb_header      ecb;    // event controlblock
        ipx_header      ipx;    // der ipx header
        packet_data     pd;     // die eigentlichen daten
} ipx_packet;

// nochmal die Definition einer vollst. netaddress
// NETWORK:NODE:(ADDRESS)???
typedef struct user_address {
        ubyte network[4];
        ubyte node[6];
        ubyte address[6];
} user_address;

#pragma pack (pop)

//------------------------------------
//--- die einzelnen User -------------
//------------------------------------
#define MAX_USERS 64    // maximal soviel
int Ipx_num_users = 0;  // soviele wirklich
user_address Ipx_users[MAX_USERS];   // von allen usern die user_address aufheben

//------------------------------------
//--- die einzelnen Netzwerke --------
//------------------------------------
#define MAX_NETWORKS 64
int Ipx_num_networks = 0;
uint Ipx_networks[MAX_NETWORKS];   // dÅrften 4 bytes sein! Ja...


//---------------------------------------
//--- die einzelnen Pakete (im pmmem ----
//---------------------------------------
int ipx_packetnum = 0;  // die fortlaufend erhîhte Paketnummer fÅr zu sendende Pakete
                        // kommt in ipx_header.pd.packetnum
#define MAX_PACKETS 64

//--- ein Speicher fÅr empfangene (aber noch nicht ausgewertete) Pakete
static packet_data packet_buffers[MAX_PACKETS];  // packetnum < 0, bedeuted dabei: frei


//--- die Freispeicher Verwaltung ----
static short packet_free_list[MAX_PACKETS];  // eine Freiliste, die einzelnen Elemente sind
                                             // Indizes auf packet_buffers[]
static int num_packets = 0;  // Zeiger fÅr Freiliste, wohl nicht:
                             // eher: Anzahl der (unausgelesenen) DatenPakete in packet_buffers[]

static int largest_packet_index = 0;     // ptr auf das groesste Paket
static short packet_size[MAX_PACKETS];   // die einzelnen Paketgroessen


//---------------------------------------
//--- anderer Kram ----------------------
//---------------------------------------
ubyte ipx_installed=0;          // Ja/nein flag

WORD ipx_vector_segment;        // realmode seg der "Novell-Routine"
WORD ipx_vector_offset;         // realmode ofs der "Novell-Routine"
ubyte ipx_socket_life = 0;      // 0=closed at prog termination, 0xff=closed when requested.

//-- User's network address ------
DWORD ipx_network = 0;          // meine network (4Byte) address
local_address ipx_my_node;      // meine node (6Byte) address
WORD ipx_socket=0;              // zu verwendender socket

//-- fÅr die IPX Pakete im realmem ------
WORD ipx_num_packets=32;        // 32 Ipx packets
ipx_packet * packets;           // Zeiger zu den 32 IPX Paketen im realmem
                                 // packets[0] scheint das Sendepaket zu sein
ushort ipx_packets_selector = 0; // der Selektor fÅr die realmem packets
int neterrors = 0;

//-- ECB pointer (nur einer?) ----
ecb_header * last_ecb=NULL;
int lastlen=0;

//----------------------------------------
//--- setzt das Paket id in Freiliste ----
//----------------------------------------
//--- used internally by ipx_get_packet_data() ---
void free_packet( int id )
{
  packet_buffers[id].packetnum = -1;  // free packet_buffer id

  packet_free_list[ --num_packets ] = id;  // id in freiliste eintragen

  if (largest_packet_index == id)
    while ( (--largest_packet_index > 0) &&
            (packet_buffers[largest_packet_index].packetnum == -1 ) );
}

void got_new_packet( ecb_header * ecb );
void ipx_listen_for_packet(ecb_header * ecb );

//--------------------------------------------------
//--- holt Paketdaten eines beliebig wartenden? ----
//--------------------------------------------------
// If any packets waiting to be read in, this fills data in with the packet
// data and returns the number of bytes read.
// Else returns 0 if no packets waiting.
int ipx_get_packet_data( ubyte * data )
{
        int i, n, best, best_id, size;

        //---- alle lauschenden ecb's auf InUseFlag != 0xFF prÅfen
        for (i=1; i < ipx_num_packets; i++ )      {
                if ( !packets[i].ecb.in_use )   {
                        //--- jetzt kann InUseFlag alles au·er 0xFF sein
                        got_new_packet( &packets[i].ecb );  // get the data
                        //--- den ecb wieder lauschen lassen
                        packets[i].ecb.in_use = 0;
                        ipx_listen_for_packet(&packets[i].ecb);
                }
        }

        //--- glaube: jetzt von den (evtl. mehreren) empf Paketen, das
        //--- mit der kleinsten ("besten") Paketnummer raussuchen
        best = -1;        // die "beste" Paketnummer
        best_id = -1;     // der slot in packet_buffers[] dieses Pakets
        n = 0;

        //--- fÅr alle empfangenen Datenpakete ----------
        for (i = 0; i <= largest_packet_index; i++) {
          //--- nur wenn gÅltige paketnummer?? -------
          if (packet_buffers[i].packetnum > -1 ) {
            n++;
            if ((best == -1) || (packet_buffers[i].packetnum < best) ) {
              //--- ich hab' ein noch "jÅngeres" (bestes) gefunden
              best = packet_buffers[i].packetnum;  // unser neues Bestes
              best_id = i;
            }
          }
        }

        //mprintf( (0, "Best id = %d, pn = %d, last_ecb = %x, len=%x, ne = %d\n", best_id, best, last_ecb, lastlen, neterrors ));
        //mprintf( (1, "<%d> ", neterrors ));

        if (best_id < 0) return 0;   // es gibt kein ("jÅngstes") Paket

        //--- die zuÅckzuliefernden Daten des besten Pakets erstellen ----
        size = packet_size[best_id];
        memcpy(data, packet_buffers[best_id].data, size);

        //--- den packet_buffer best_id wieder in die Freiliste setzen
        free_packet(best_id);

        return size;
}  // ipx_get_packet_data()

unsigned int swap_short( unsigned int short );
#pragma aux swap_short parm [eax] = "xchg al,ah";

//--- used internally by ipx_get_packet_data(), I think ------------
void got_new_packet( ecb_header * ecb )  // der ecb, der scheinbar was empf. hat
{
        ipx_packet * p;
        int id;
        unsigned short datasize;

        datasize = 0;
        last_ecb = ecb;   // den Åbergebenen ecb merken
        p = (ipx_packet *)ecb;  // PS: typedef struct ipx_packet {
                                //             ecb_header      ecb;    // event controlblock
                                //             ipx_header      ipx;    // der ipx header
                                //             packet_data     pd;     // die eigentlichen daten
                                //     } ipx_packet;

        if ( p->ecb.in_use ) {
          neterrors++;  // InUseFlag war damit != 0x00,..
          return;       // ...aber auch(!) != 0xFF -> Fehler++
        }

        if ( p->ecb.completion_code ) {
          neterrors++;   // alles != 0x00 ist ein Fehler
          //  Error( "Recieve error %d for completion code", p->ecb.completion_code );
          return;
        }

        // make sure, that the packet is not from ourselves
        if ( memcmp( &p->ipx.source.node_id, &ipx_my_node, 6 ) )        {

                datasize=swap_short(p->ipx.length);  // the whole packet size
                lastlen=datasize;
                datasize -= sizeof(ipx_header);      // ipx_header abziehen
                                                     // datasize := pure data length
                // Find slot to put packet in...
                if ( (datasize > 0) && (datasize <= sizeof(packet_data)) )  {

                        // die Datengrî·e ist richtig
                        if ( num_packets >= MAX_PACKETS ) { // zeiger auf freiliste checken??
                                // wenn ich die Daten des eben empf Pakets Åbernehmen
                                // wÅrde -> öberlauf des Daten Ringbuffers
                                //printf( 1, "IPX: Packet buffer overrun!!!\n" );
                                neterrors++;
                                return;
                        }

                        //PS: static short packet_free_list[MAX_PACKETS];      // eine Freiliste
                        id = packet_free_list[ num_packets++ ];

                        if (id > largest_packet_index)
                          largest_packet_index = id;

                        //PS: typedef struct packet_data {
                          //          int                     packetnum;  // unique for every packet?
                          //                                              // -1 seems to be "free"
                          //          byte                    data[IPX_MAX_DATA_SIZE];  // [542]
                          //  } packet_data;

                        packet_size[id] = datasize-sizeof(int); // without packetnum
                        packet_buffers[id].packetnum =  p->pd.packetnum;  // packetnum setzen

                        if ( packet_buffers[id].packetnum < 0 ) { // hmm??
                          neterrors++;
                          return;
                        }

                        // ok, now take the data out of packet to store into packet_buffers[]
                        memcpy( packet_buffers[id].data, p->pd.data, packet_size[id] );

                } else {
                        neterrors++;    // packet data size is out of range
                        return;
                }
        }

        // Repost the ecb
        p->ecb.in_use = 0;  // set free here (rubbish, because already checked above)
        //ipx_listen_for_packet(&p->ecb);
} // got_new_packet()


//-------------------------------------
//--- liefert User's Netzwerkdaten ----
//-------------------------------------
ubyte * ipx_get_my_local_address()
{
  return ipx_my_node.address;   // meine node Adresse
}

ubyte * ipx_get_my_server_address()
{
  return (ubyte *)&ipx_network; // meine Netzwerk Adresse
}

//--- used internally, i think by: -----------
// ipx_get_packet_data() - um den ecb (nach Empfang) wieder lauschen zu lassen
// ipx_init() - um die (32-1) ecb auf "Lauschen" zu initialisieren
// ipx_change_default_socket() - um repost all listen requests on the new socket...
void ipx_listen_for_packet(ecb_header * ecb )
{
  dpmi_real_regs rregs;

  ecb->in_use = 0x1d;   // hmmm, eigentlich unnîtig?
  memset(&rregs,0,sizeof(dpmi_real_regs));
  rregs.ebx = 4;  // Listen For Packet function
  rregs.esi = DPMI_real_offset(ecb);
  rregs.es = DPMI_real_segment(ecb);
  dpmi_real_int386x( 0x7A, &rregs );
}

//--- used internally, i think by: -----------
// ipx_change_default_socket() - um die lauschenden Pakete vom "alten" Socket zu trennen
void ipx_cancel_listen_for_packet(ecb_header * ecb )
{
  dpmi_real_regs rregs;
  memset(&rregs,0,sizeof(dpmi_real_regs));
  rregs.ebx = 6;  // IPX Cancel event
  rregs.esi = DPMI_real_offset(ecb);
  rregs.es = DPMI_real_segment(ecb);
  dpmi_real_int386x( 0x7A, &rregs );
}


//--------------------------------------------------
//--- gibt ein (bereits geschnÅrtes) Paket auf -----
//--------------------------------------------------
//--- used internally, i think by: ----------
// ipx_send_packet_data() - um einen geschnÅrtes Paket aufzugeben
void ipx_send_packet(ecb_header * ecb )
{
  dpmi_real_regs rregs;
  memset(&rregs,0,sizeof(dpmi_real_regs));
  rregs.ebx = 3;  // Send Packet function
  rregs.esi = DPMI_real_offset(ecb);
  rregs.es = DPMI_real_segment(ecb);
  dpmi_real_int386x( 0x7A, &rregs );
}

//--- new Frank 9/98 ----------
unsigned long ipx_get_vector(void)
{
  dpmi_real_regs rregs;
  // Get the IPX vector
  memset(&rregs, 0, sizeof(dpmi_real_regs));
  rregs.eax=0x00007a00;
  dpmi_real_int386x( 0x2f, &rregs );
  if ((rregs.eax & 0xFF) != 0xFF ) {
    return 0;       // er meint wohl -3 == no ipx installed
  }
  return (((ulong)(rregs.es) << 16) + (rregs.edi & 0xFFFF));
}

//-------------------------------------
//--- liefert User's local target -----
//-------------------------------------
#pragma pack (push, 1)
typedef struct {
        ubyte   network[4];
        ubyte           node[6];
        ubyte           local_target[6];    // ???
} net_xlat_info;
#pragma pack (pop)

//---------------------------------------------------------------------
// return local target at *local_target
// liefert die physische Stationsadresse des nÑchsten Routers fÅr eine
// vorgegebene Zieladresse
//--- used internally, i think by: ----------
// ipx_send_broadcast_packet_data() - um??
void ipx_get_local_target( ubyte * server, ubyte * node, ubyte *local_target )
{
  net_xlat_info * info;
  dpmi_real_regs rregs;

  // Get dos memory for call...
  info = (net_xlat_info *)dpmi_get_temp_low_buffer( sizeof(net_xlat_info) );
  assert( info != NULL );

  //--- die vorgegebene Zieladresse -------
  memcpy( info->network, server, 4 );
  memcpy( info->node, node, 6 );

  memset(&rregs,0,sizeof(dpmi_real_regs));

  rregs.ebx = 2;          // Get Local Target
  rregs.es = DPMI_real_segment(info);
  rregs.esi = DPMI_real_offset(info->network);
  rregs.edi = DPMI_real_offset(info->local_target);

  dpmi_real_int386x( 0x7A, &rregs );

  // Save the local target...
  memcpy( local_target, info->local_target, 6 );
}

void ipx_close()
{
        dpmi_real_regs rregs;
        if ( ipx_installed )    {
                // When using VLM's instead of NETX, the sockets don't
                // seem to automatically get closed, so we must explicitly
                // close them at program termination.
                ipx_installed = 0;
                memset(&rregs,0,sizeof(dpmi_real_regs));
                rregs.edx = ipx_socket;
                rregs.ebx = 1;  // Close socket
                dpmi_real_int386x( 0x7A, &rregs );
        }
        if ( ipx_packets_selector ) {
          dpmi_real_free(ipx_packets_selector);
        }
}


//--- new Frank 9/98 ------
//--- nicht mehr innerhalb ipx_init() zum Lauschen anstossen sondern hier
//----------------------------------------------------
void ipx_now_startlisten(unsigned long esr)
{
    int i;

    // fÅr alle Pakete ausser dem ersten
    for (i = 1; i < ipx_num_packets; i++) {
       //--- new frank 9/98 -------
       *((ulong*)(&packets[i].ecb.esr_address[0])) = esr;
       //---------
       ipx_listen_for_packet(&packets[i].ecb);   // alle lauschen lassen
    }
}
//----------------------------------------------------
// nÅtzlich um alle ecb's vom "Lauschen" abzuziehen
//----------------------------------------------------
void ipx_now_stoplisten(void)
{
  int i;
  for (i = 1; i < ipx_num_packets; i++)
    ipx_cancel_listen_for_packet(&packets[i].ecb);
}


//---------------------------------------------------------------
// Initializes all IPX internals.
// If socket_number==0, then opens next available socket.
// Returns:     0  if successful.
//                              -1 if socket already open.
//                              -2      if socket table full.
//                              -3 if IPX not installed.
//                              -4 if couldn't allocate low dos memory
//                              -5 if error with getting internetwork address

//int ipx_init( int socket_number, int show_address )

//--- new frank 9/98 -------
int ipx_init( int socket_number, int show_address)
//---------
{
        dpmi_real_regs rregs;
        ubyte *ipx_real_buffer;
        int i;

        atexit(ipx_close);

        ipx_packetnum = 0;  // den "ausgehenden PaketzÑhler" auf 0 setzen

        // init packet buffers.
        for (i=0; i<MAX_PACKETS; i++ )  {
                packet_buffers[i].packetnum = -1;
                packet_free_list[i] = i;
        }
        num_packets = 0;
        largest_packet_index = 0;

        // Get the IPX vector

        memset(&rregs,0,sizeof(dpmi_real_regs));
        rregs.eax=0x00007a00;
        dpmi_real_int386x( 0x2f, &rregs );

        if ( (rregs.eax & 0xFF) != 0xFF )       {
                return 3;       // er meint wohl -3 == no ipx installed
        }
        ipx_vector_offset = rregs.edi & 0xFFFF;
        ipx_vector_segment = rregs.es;
        if (show_address)
          printf( "IPX entry point at %.4x:%.4x\n", ipx_vector_segment, ipx_vector_offset );

        // Open a socket for IPX

        memset(&rregs,0,sizeof(dpmi_real_regs));
        swab( (char *)&socket_number,(char *)&ipx_socket, 2 );  // zuverwenden socketnumer setzen
        rregs.edx = ipx_socket;
        rregs.eax = ipx_socket_life;  // 0=closed at prog termination, 0xff=closed when requested.
        rregs.ebx = 0;  // Open socket
        dpmi_real_int386x( 0x7A, &rregs );

        ipx_socket = rregs.edx & 0xFFFF;

        if ( rregs.eax & 0xFF ) {
                //mprintf( (1, "IPX error opening channel %d\n", socket_number-IPX_DEFAULT_SOCKET ));
                printf( "IPX error opening channel %d\n", socket_number-IPX_DEFAULT_SOCKET );
                return -2;
        }

        ipx_installed = 1;

        // Find our internetwork address

        // ipx_real_buffer ist ubyte *
        ipx_real_buffer = dpmi_get_temp_low_buffer( 1024 );     // 1k block
        if ( ipx_real_buffer == NULL )  {
                printf( "Error allocation realmode memory\n" );
                return -4;
        }

        memset(&rregs,0,sizeof(dpmi_real_regs));
        rregs.ebx = 9;          // Get internetwork address
        rregs.esi = DPMI_real_offset(ipx_real_buffer);
        rregs.es = DPMI_real_segment(ipx_real_buffer);
        dpmi_real_int386x( 0x7A, &rregs );

        if ( rregs.eax & 0xFF ) {
                printf( "Error getting internetwork address!\n" );
                return -2;
        }

        memcpy( &ipx_network, ipx_real_buffer, 4 );
        memcpy( &ipx_my_node, &ipx_real_buffer[4], 6 );

        // Die Anzahl erkannter Netze setzen und Nr merken
        Ipx_num_networks = 0;
        memcpy( &Ipx_networks[Ipx_num_networks++], &ipx_network, 4 );

        if ( show_address )     { // int, Åbergeben an main
                printf( "My IPX addresss is " );
                // network
                printf( "%02X%02X%02X%02X/", ipx_real_buffer[0],ipx_real_buffer[1],ipx_real_buffer[2],ipx_real_buffer[3] );
                // node
                printf( "%02X%02X%02X%02X%02X%02X\n", ipx_real_buffer[4],ipx_real_buffer[5],ipx_real_buffer[6],ipx_real_buffer[7],ipx_real_buffer[8],ipx_real_buffer[9] );
                printf( "\n" );
        }

        // allocate RealMem for ipx_num_packets (auf 32 gesetzt)

        packets = dpmi_real_malloc( sizeof(ipx_packet)*ipx_num_packets, &ipx_packets_selector );
        if ( packets == NULL )  {
                printf( "Couldn't allocate real memory for %d packets\n", ipx_num_packets );
                return -4;
        }
        if (!dpmi_lock_region( packets, sizeof(ipx_packet)*ipx_num_packets ))   {
                printf( "Couldn't lock real memory for %d packets\n", ipx_num_packets );
                return -4;
        }
        memset( packets, 0, sizeof(ipx_packet)*ipx_num_packets );

        // fÅr alle Pakete ausser dem ersten
        for (i=1; i<ipx_num_packets; i++ )      {
                packets[i].ecb.in_use = 0x1d;   // warum ??
                //packets[i].ecb.in_use = 0;
                packets[i].ecb.socket_id = ipx_socket;
                packets[i].ecb.fragment_count = 1;
                packets[i].ecb.fragment_pointer[0] = DPMI_real_offset(&packets[i].ipx);
                packets[i].ecb.fragment_pointer[1] = DPMI_real_segment(&packets[i].ipx);
                // aha, hier ist der Trick: Fragment1 ist ipxheader und(!) data
                packets[i].ecb.fragment_size = sizeof(ipx_packet)-sizeof(ecb_header);                   //-sizeof(ecb_header);
                                          // == Gesamtpaketsize (ecb,ipxhdr,data) - sizeof(ecb)

                //--- new frank 9/98 -------
                //--- ich lass sie erst explizit mit ipx_now_startlisten() lauschen
                // ipx_listen_for_packet(&packets[i].ecb);   // alle lauschen lassen
                //------
        }

        // fÅr alle Pakete ausser dem ersten (das ist denk ich Sendepaket)
        packets[0].ecb.socket_id = ipx_socket;
        packets[0].ecb.fragment_count = 1;
        packets[0].ecb.fragment_pointer[0] = DPMI_real_offset(&packets[0].ipx);
        packets[0].ecb.fragment_pointer[1] = DPMI_real_segment(&packets[0].ipx);
        packets[0].ipx.packet_type = 4;         // IPX packet
        packets[0].ipx.destination.socket_id = ipx_socket;
//      memcpy( packets[0].ipx.destination.network_id, &ipx_network, 4 );
        memset( packets[0].ipx.destination.network_id, 0, 4 );

        //--- new frank 9/98 -------
        // I don't need an ESR for outgoing packets....
        //---------

        return 0;
}


// Sends a packet to a certain address
//-------------------------------------
//--- IPX SEND PACKET DATA ------------
//-------------------------------------
void ipx_send_packet_data( ubyte * data,
                            int datasize,
                             ubyte *network,
                              ubyte *address,
                               ubyte *immediate_address )
{
        assert(ipx_installed);

        //--- Paketgroesse checken ---
        if ( datasize >= IPX_MAX_DATA_SIZE )    {
          printf( "Data too big\n" );
          exit(1);
        }

        // Make sure no one is already sending something
        while( packets[0].ecb.in_use ) // PS: packets[0] is our sending packet
        {
        }

        //--- Gab es beim letzten gesendeteb Paket einen Fehler? ----
        if (packets[0].ecb.completion_code)     {
          printf( "Send error %d for completion code\n", packets[0].ecb.completion_code );
          exit(1);
        }

        //--- Fill in destination address  (genauer: Network Address)
        if ( memcmp( network, &ipx_network, 4 ) )
          memcpy( packets[0].ipx.destination.network_id, network, 4 );
        else {
          // im selben Network wie ich, also 0x00000000 eintragen
          memset( packets[0].ipx.destination.network_id, 0, 4 );
        }

        //--- nodeadresse und immediateaddress eintragen
        memcpy( packets[0].ipx.destination.node_id.address, address, 6 );
        memcpy( packets[0].ecb.immediate_address.address, immediate_address, 6 );

        //--- die Paketnummer setzen
        packets[0].pd.packetnum = ipx_packetnum++;

        // Fill in data to send
        packets[0].ecb.fragment_size = sizeof(ipx_header) + sizeof(int) + datasize;
        // logisch: das (einzige) Fragment (bestehend aus ipx_header und den eigentlichen Daten)
        //          hat die Grî·e: sizeof(ipx_header) + sizeof(packetnum) + "pure data size";

        assert( datasize > 1 );   // also keine 0 Pakete zulassen ;)
        assert( packets[0].ecb.fragment_size <= 576 );  // und nicht zu gross

        //--- jetzt die Daten eintragen ---
        memcpy( packets[0].pd.data, data, datasize );

        // Send it
        ipx_send_packet( &packets[0].ecb );  // das Paket "aufgeben" (dem ipx driver Åbergeben)

}

//--- 08/99 um gp2 network zu optimieren -----
// Sends a packet to a certain address
//-------------------------------------
//--- IPX SEND PACKET DATA2 -----------
//-------------------------------------

void send_packet_dirty( void *rmintbuf );   // EDI
// öbergabe: rmintbuf == dpmi call (simulate rmint)
// RÅckgabe: -
#pragma aux send_packet_dirty = \
             /* "pushad" */ \
     /*----- setup rmint-buffer ----------*/ \
             /* already done from outside this function */ \
     /*----- issue rmint via DPMI call ----------*/ \
             "mov eax, 300h"        /* simulate rmint */ \
             "mov ebx, 7ah"         /* interrupt 10h */ \
             "xor ecx, ecx"         /* copy 0 words stack from pm to rm */ \
             "int 31h" \
     /*----- rmint-fehlerauswertung im rmi-buf ----------*/ \
             /* no checking */ \
     /*----- RÅckgabewerte aufbereiten ------------*/ \
             /* "popad" */ \
parm[EDI] modify exact [];

// --------------------------------------

void ipx_send_packet_data2( ubyte *data,
                             int datasize,
                              ulong *network,
                               ubyte *address,
                                ubyte *immediate_address )
{
        static dpmi_real_regs rmintregs;


        //--- Fill in destination address  (genauer: Network Address)
        if ( *network != ipx_network )
          *(ulong *)&packets[0].ipx.destination.network_id = *network;
        else {
          // im selben Network wie ich, also 0x00000000 eintragen
          *(ulong *)&packets[0].ipx.destination.network_id = 0;
        }

        //--- nodeadresse und immediateaddress eintragen
        mymemcpy( &packets[0].ipx.destination.node_id.address, address, 6 );
        mymemcpy( &packets[0].ecb.immediate_address.address, immediate_address, 6 );

        //--- die Paketnummer setzen
        packets[0].pd.packetnum = ipx_packetnum++;

        // Fill in data to send
        packets[0].ecb.fragment_size = sizeof(ipx_header) + sizeof(int) + datasize;
        // logisch: das (einzige) Fragment (bestehend aus ipx_header und den eigentlichen Daten)
        //          hat die Grî·e: sizeof(ipx_header) + sizeof(packetnum) + "pure data size";

        //--- jetzt die Daten eintragen ---
        mymemcpy( (void *)&packets[0].pd.data, data, datasize );

        // Send it
        rmintregs.ebx = 3;
        rmintregs.esi = DPMI_real_offset(&packets[0].ecb);
        rmintregs.es = DPMI_real_segment(&packets[0].ecb);
        send_packet_dirty( &rmintregs );   // das Paket "aufgeben" (dem ipx driver Åbergeben)

} // ipx_send_packet_data2()



//-----------------------------------------
//--- IPX SEND BROADCAST PACKET DATA ------
//-----------------------------------------
void ipx_send_broadcast_packet_data( ubyte * data,
                                      int datasize )
{
        int i, j;
        ubyte broadcast[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
        ubyte local_address[6];

        // Set to all networks besides mine
        for (i = 0; i < Ipx_num_networks; i++ ) {
          if (memcmp( &Ipx_networks[i], &ipx_network, 4)) {
            //--- it's not my network, muss also "weiter" senden ---
            ipx_get_local_target( (ubyte *)&Ipx_networks[i], broadcast, local_address );
            // local_address erhielt damit: nÑhester Router?
            // and den jetzt die broadcast schicken?? (ihn als immediate address einsetzen)
            ipx_send_packet_data( data, datasize, (ubyte *)&Ipx_networks[i], broadcast, local_address );
          }
          else {
            //--- it's my network ----
            ipx_send_packet_data( data, datasize, (ubyte *)&Ipx_networks[i], broadcast, broadcast );
          }
        }

        //OLDipx_send_packet_data( data, datasize, (ubyte *)&ipx_network, broadcast, broadcast );

        // Send directly to all users not on my network or in the network list.
        for (i = 0; i < Ipx_num_users; i++) {

          if ( memcmp( Ipx_users[i].network, &ipx_network, 4 ) )  {
            //--- hierhin nur, wenn nicht meinem network? ----
            for (j = 0; j < Ipx_num_networks; j++ ) {
              if (!memcmp( Ipx_users[i].network, &Ipx_networks[j], 4 ))
                goto SkipUser;
            }
            ipx_send_packet_data( data, datasize, Ipx_users[i].network, Ipx_users[i].node, Ipx_users[i].address );
SkipUser:
            j = 0;
          }
        }
}

// Sends a non-localized packet... needs 4 byte server, 6 byte address
void ipx_send_internetwork_packet_data( ubyte * data,
                                         int datasize,
                                          ubyte *server,
                                           ubyte *address )
{
    ubyte local_address[6];

    if ( (*(uint *)server) != 0 ) {
      ipx_get_local_target( server, address, local_address );
      // now use nearest router??
      ipx_send_packet_data( data, datasize, server, address, local_address );
    } else {
      // Old method, no server info.
      ipx_send_packet_data( data, datasize, server, address, address );
    }
}

int ipx_change_default_socket( ushort socket_number )
{
        int i;
        WORD new_ipx_socket;
        dpmi_real_regs rregs;

        if ( !ipx_installed ) return -3;

        // Open a new socket
        memset(&rregs,0,sizeof(dpmi_real_regs));
        swab( (char *)&socket_number,(char *)&new_ipx_socket, 2 );
        rregs.edx = new_ipx_socket;
        rregs.eax = ipx_socket_life;
        rregs.ebx = 0;  // Open socket
        dpmi_real_int386x( 0x7A, &rregs );

        new_ipx_socket = rregs.edx & 0xFFFF;

        if ( rregs.eax & 0xFF ) {
                //printf( (1, "IPX error opening channel %d\n", socket_number-IPX_DEFAULT_SOCKET ));
                return -2;
        }

        for (i=1; i<ipx_num_packets; i++ )      {
                ipx_cancel_listen_for_packet(&packets[i].ecb);
        }

        // Close existing socket...
        memset(&rregs,0,sizeof(dpmi_real_regs));
        rregs.edx = ipx_socket;
        rregs.ebx = 1;  // Close socket
        dpmi_real_int386x( 0x7A, &rregs );

        ipx_socket = new_ipx_socket;

        // Repost all listen requests on the new socket...
        for (i=1; i<ipx_num_packets; i++ )      {
                packets[i].ecb.in_use = 0;
                packets[i].ecb.socket_id = ipx_socket;
                ipx_listen_for_packet(&packets[i].ecb);
        }

        packets[0].ecb.socket_id = ipx_socket;
        packets[0].ipx.destination.socket_id = ipx_socket;

        ipx_packetnum = 0;  // den "ausgehenden PaketzÑhler" auf 0 setzen
        // init packet buffers.
        for (i=0; i<MAX_PACKETS; i++ )  {
                packet_buffers[i].packetnum = -1;
                packet_free_list[i] = i;
        }
        num_packets = 0;
        largest_packet_index = 0;

        return 0;
}

void ipx_read_user_file(char * filename)
{
        FILE * fp;
        user_address tmp;
        char temp_line[132], *p1;
        int n, ln=0;

        if (!filename) return;

        Ipx_num_users = 0;

        fp = fopen( filename, "rt" );
        if ( !fp ) return;

        printf( "Broadcast Users:\n" );

        while (fgets(temp_line, 132, fp)) {
                ln++;
                p1 = strchr(temp_line,'\n'); if (p1) *p1 = '\0';
                p1 = strchr(temp_line,';'); if (p1) *p1 = '\0';
                n = sscanf( temp_line, "%2x%2x%2x%2x/%2x%2x%2x%2x%2x%2x", &tmp.network[0], &tmp.network[1], &tmp.network[2], &tmp.network[3], &tmp.node[0], &tmp.node[1], &tmp.node[2],&tmp.node[3], &tmp.node[4], &tmp.node[5] );
                if ( n != 10 ) continue;
                if ( Ipx_num_users < MAX_USERS )        {
                        ubyte * ipx_real_buffer = (ubyte *)&tmp;
                        ipx_get_local_target( tmp.network, tmp.node, tmp.address );
                        Ipx_users[Ipx_num_users++] = tmp;
                        printf( "%02X%02X%02X%02X/", ipx_real_buffer[0],ipx_real_buffer[1],ipx_real_buffer[2],ipx_real_buffer[3] );
                        printf( "%02X%02X%02X%02X%02X%02X\n", ipx_real_buffer[4],ipx_real_buffer[5],ipx_real_buffer[6],ipx_real_buffer[7],ipx_real_buffer[8],ipx_real_buffer[9] );
                } else {
                        printf( "Too many addresses in %s! (Limit of %d)\n", filename, MAX_USERS );
                        fclose(fp);
                        return;
                }
        }
        fclose(fp);
}


void ipx_read_network_file(char * filename)
{
        FILE * fp;
        user_address tmp;
        char temp_line[132], *p1;
        int i, n, ln=0;

        if (!filename) return;

        fp = fopen( filename, "rt" );
        if ( !fp ) return;

        printf( "Using Networks:\n" );
        for (i=0; i<Ipx_num_networks; i++ )             {
                ubyte * n1 = (ubyte *)&Ipx_networks[i];
                printf("* %02x%02x%02x%02x\n", n1[0], n1[1], n1[2], n1[3] );
        }

        while (fgets(temp_line, 132, fp)) {
                ln++;
                p1 = strchr(temp_line,'\n'); if (p1) *p1 = '\0';
                p1 = strchr(temp_line,';'); if (p1) *p1 = '\0';
                n = sscanf( temp_line, "%2x%2x%2x%2x", &tmp.network[0], &tmp.network[1], &tmp.network[2], &tmp.network[3] );
                if ( n != 4 ) continue;
                if ( Ipx_num_networks < MAX_NETWORKS  ) {
                        int j;
                        for (j=0; j<Ipx_num_networks; j++ )
                                if ( !memcmp( &Ipx_networks[j], tmp.network, 4 ) )
                                        break;
                        if ( j >= Ipx_num_networks )    {
                                memcpy( &Ipx_networks[Ipx_num_networks++], tmp.network, 4 );
                                printf("  %02x%02x%02x%02x\n", tmp.network[0], tmp.network[1], tmp.network[2], tmp.network[3] );
                        }
                } else {
                        printf( "Too many networks in %s! (Limit of %d)\n", filename, MAX_NETWORKS );
                        fclose(fp);
                        return;
                }
        }
        fclose(fp);

}

//=========================================================================
//=== neu dazu ============================================================
//=========================================================================
//#include "ipxdiv.h"
//---- new frank 9/98 -----
//#pragma on (check_stack);
//---------

#endif
