#ifndef GP2GLOB_H
#define GP2GLOB_H


// Hooked gp2 variables
extern BYTE			*pSessionMode;
extern BYTE			*pIsReplay;
extern BYTE			*pIsAccTime;
extern DWORD		*pFrameTime;
extern DWORD		*pRealFrameTime;
extern BYTE			*pTrackBuf;
extern DWORD		*pTrackFileLength;
extern DWORD		*pTrackNr;
extern DWORD		*pTrackIndex;
extern DWORD		*ardwInitVals;
extern BYTE			*pWheelStructs;
extern GP2Car		**ppPlayerCS;
extern GP2Car		**ppCockpitCS;
extern GP2Car		**ppSelectedCS;
extern BYTE			*pFastLapCars;
extern GP2LongName	*pDriverNames;
extern GP2ShortName	*pTeamNames;
extern GP2ShortName	*pEngineNames;
extern DWORD		*pCurTime;
extern DWORD		*pSesStartTime;
extern WORD			*pRaceDistPerc;
extern WORD			*pNumCars;
extern BYTE			*pCarIDs;
extern DWORD		*pRaceTimes;
extern BYTE			*arCurTyreSet;
extern GP2FileInfo	**ppFileInfo;
extern DWORD		*pHotlapSel;
extern BYTE			*pSteeringHelp;
extern BYTE			*pOppLockHelp;
extern GP2Header	*pFileHdr;
extern DWORD		*pCarShape;
extern WORD			*pPitSpeedLimit;
extern DWORD		*pRetireLimit;
extern DWORD		*pDamageTrack;
extern DWORD		*pDamageCars;
extern BYTE			*pDetailLevel;
extern WORD			*pFrameWaitCode;
extern GP2CSx		*pCarSetups;
extern BYTE			*pPerfFlag;
extern WORD			*pTeamHPQual;
extern BYTE			*pPlayerHPCode;
extern BYTE			*pNumLapsDone;
extern DWORD		*pFileDataLen;
extern DWORD		*pFileDataLen2;
extern BYTE			*pPaused;
extern DWORD		*pNumCars2;
extern short		*pIsKPH;
extern WORD			*pNumTrackSegs;
extern GP2Seg		*pTrackSegs;
extern DWORD		*pCurbData;
extern WORD         *arTrackTyreWear;
extern WORD			*arGearRevsTable;
extern DWORD		*arLightRevsTable;

////////////////////////////////////////////////////////////////////////////////// //

#endif
