#ifndef _IPXSTRUC_H
#define _IPXSTRUC_H

#define PID_REQUEST	      25
#define PID_SYNC	      27
#define PID_PDATA	      28
#define PID_ADDPLAYER	      29
#define PID_DUMP	      31
#define PID_ENDLEVEL	      32
#define PID_QUIT_JOINING      34
#define PID_OBJECT_DATA       35
#define PID_GAME_LIST	      36
#define PID_GAME_INFO	      37

#define PID_JOIN	      40
#define PID_LEAVE	      41
#define PID_INVITE	      42
#define PID_AGREE	      43
#define PID_REFUSE	      44


#pragma pack (push, 1)

//------------------------------------------------------------

typedef struct netplayer_info {
    char	    callsign[80];
    ubyte	    server[4];
    ubyte	    node[6];
    ushort	    socket;
    byte	    connected;
} netplayer_info;

typedef struct sequence_packet {
    ubyte	      type;
    netplayer_info    player;
} sequence_packet;

//------------------------------------------------------------

typedef struct gp2packet {
    ubyte      type;
    short      len;
    // ubyte	  data[256];
    ubyte      data[130];
} gp2packet;

//------------------------------------------------------------

typedef struct netgame_info {
    ubyte				    type;
    ubyte				    space[255];
/*
    ubyte				    type;
    char				    game_name[NETGAME_NAME_LEN+1];
    char				    team_name[2][CALLSIGN_LEN+1];
    ubyte				    gamemode;
    ubyte				    difficulty;
    ubyte			    game_status;
    ubyte				    numplayers;
    ubyte				    max_numplayers;
    ubyte				    game_flags;
    netplayer_info	    players[MAX_PLAYERS];
    int 				    locations[MAX_PLAYERS];
    short				    kills[MAX_PLAYERS][MAX_PLAYERS];
    int 				    levelnum;
    ubyte				    protocol_version;
    ubyte				    team_vector;
    ushort			    segments_checksum;
    short				    team_kills[2];
    short				    killed[MAX_PLAYERS];
    short				    player_kills[MAX_PLAYERS];

    fix 				    level_time;
    int 				    control_invul_time;
    int 				    monitor_vector;
    int 				    player_score[MAX_PLAYERS];
    ubyte				    player_flags[MAX_PLAYERS];
    char				    mission_name[9];
    char				    mission_title[MISSION_NAME_LEN+1];
*/
} netgame_info;

#pragma pack (pop)

#endif
