//---- defined items ----
itemdef items[] = {         // item names are case insensitive
    //[EXTENDED DATA LOGGING]
    NULL, ULONGTYPE,  "logPerf",
    NULL, ULONGTYPE,  "logGLX",
    NULL, ULONGTYPE,  "logCC",
    NULL, ULONGTYPE,  "logDebug",

    //[DEBUG LOGGING]
    NULL, STRTYPE,    "logDebugName",
    NULL, ULONGTYPE,  "logDebugFlush",
    NULL, ULONGTYPE,  "logDebugScreenLines",

    //[AT THE LINE]
    NULL, ULONGTYPE,  "atlNoFastestLap",
    NULL, ULONGTYPE,  "atlSplit1Color",
    NULL, ULONGTYPE,  "atlSplit2Color",
    NULL, ULONGTYPE,  "atlLineColor",
    NULL, ULONGTYPE,  "atlAcrossTop",
    NULL, ULONGTYPE,  "atlMarkTopX",

    //[TRACK MAP]
    NULL, ULONGTYPE,  "mapTrackColor",
    NULL, ULONGTYPE,  "mapHAlign",
    NULL, ULONGTYPE,  "mapVAlign",
    NULL, ULONGTYPE,  "mapRotate",
    NULL, ULONGTYPE,  "mapMaxY",
    NULL, ULONGTYPE,  "mapZoom",
    NULL, ULONGTYPE,  "mapDotSize",
    NULL, ULONGTYPE,  "mapDotSizeVGA",

    //[OTHERS]
    NULL, ULONGTYPE,  "ForcePMBanking",
    NULL, ULONGTYPE,  "PureDOS",
    NULL, ULONGTYPE,  "UseOldNetworkCode",
#ifdef TEST
    NULL, ULONGTYPE,  "ForceLFBuffering",
#endif

    //[Network]
    NULL, ULONGTYPE,  "UseIpxLink",
    NULL, STRTYPE,    "MyLinkName",
    NULL, ULONGTYPE,  "LogIpxInfo",
#ifdef TEST
    NULL, ULONGTYPE,  "FakeLinkMate",
#endif

    NULL, ULONGTYPE,  "LogDpmiInfo",
    NULL, ULONGTYPE,  "LogVesaInfo",
#ifdef TEST
    NULL, ULONGTYPE,  "LogAllStrings",
#endif
    NULL, ULONGTYPE,  "LogIpxOutpackets",
    NULL, ULONGTYPE,  "LogIpxInpackets",

    NULL, ULONGTYPE,  "LogLnkCmdsIn",
    NULL, ULONGTYPE,  "LogLnkCmdsOut",
    NULL, ULONGTYPE,  "LogLnkCmdsProcess",

    //[HOF2.5]
    NULL, ULONGTYPE,  "hof25Enable",

    // trackfile stuff
    NULL, ULONGTYPE,  "CatchTrackInfos",
    NULL, ULONGTYPE,  "CatchSplitSegs",
    NULL, ULONGTYPE,  "MenuTrackMap",
    NULL, STRTYPE,    "f1ct01",
    NULL, STRTYPE,    "f1ct02",
    NULL, STRTYPE,    "f1ct03",
    NULL, STRTYPE,    "f1ct04",
    NULL, STRTYPE,    "f1ct05",
    NULL, STRTYPE,    "f1ct06",
    NULL, STRTYPE,    "f1ct07",
    NULL, STRTYPE,    "f1ct08",
    NULL, STRTYPE,    "f1ct09",
    NULL, STRTYPE,    "f1ct10",
    NULL, STRTYPE,    "f1ct11",
    NULL, STRTYPE,    "f1ct12",
    NULL, STRTYPE,    "f1ct13",
    NULL, STRTYPE,    "f1ct14",
    NULL, STRTYPE,    "f1ct15",
    NULL, STRTYPE,    "f1ct16",

    // misc stuff
    NULL, LONGTYPE,   "CockpitZoom",
    NULL, ULONGTYPE,  "SplitsInRace",
    NULL, ULONGTYPE,  "CarRemoveTime",
    NULL, ULONGTYPE,  "UseLocalizedStrings",
    NULL, ULONGTYPE,  "Spa98",
    NULL, ULONGTYPE,  "KeepDistUnit",
    NULL, ULONGTYPE,  "DrawLines",
    NULL, ULONGTYPE,  "VGARcr1",
    NULL, ULONGTYPE,  "SVGARcr1",
    NULL, ULONGTYPE,  "VGARcr2",
    NULL, ULONGTYPE,  "SVGARcr2",
    NULL, ULONGTYPE,  "VGARcr3",
    NULL, ULONGTYPE,  "SVGARcr3",

#ifdef SOCKCLNT
    NULL, STRTYPE,    "podIP",
    NULL, ULONGTYPE,  "podPort",
#endif

    NULL, TERMITEM, NULL    // itemlist must be terminated this way!!!!
};


//---- paths to check while searching CFGNAME ----
//---- (current dir is checked first) -----
char *paths_to_check[] = {
    "PATH", // ,INCLUDE
    NULL    // pathlist must be NULL terminated!!!!
};
