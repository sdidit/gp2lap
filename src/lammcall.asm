.386

_DATA   SEGMENT BYTE PUBLIC USE32 'DATA'
_DATA   ENDS

DGROUP GROUP _DATA

_TEXT   SEGMENT BYTE PUBLIC USE32 'CODE'
        ASSUME  cs:_TEXT

        PUBLIC  RcvGP2Call_

        EXTRN   _GP2_Found              :dword
        EXTRN   _GP2_FoundAdr           :dword
        EXTRN   _GP2_CSel               :word
        EXTRN   _GP2_DSel               :word

        EXTRN   _GP2_CodeStartAdr       :dword
        EXTRN   _GP2_CodeEndAdr         :dword
        EXTRN   _GP2_DataStartAdr       :dword
        EXTRN   _GP2_DataEndAdr         :dword
        EXTRN   _GP2_wcodeStartAdr      :dword
        EXTRN   _GP2_wcodeEndAdr        :dword
        EXTRN   _GP2_Data2StartAdr      :dword
        EXTRN   _GP2_Data2EndAdr        :dword
        EXTRN   _pCarStructs            :dword
        ;EXTRN   _pRMCodes              :dword  ; disabled 8/98

        EXTRN   _mycar                  :dword
        EXTRN   _pUseSVGA               :dword
        EXTRN   _fpCalcSetuptoESI       :dword
        EXTRN   _fpGetCaridsSetup       :dword
        EXTRN   _fpInitFileHdr          :dword
        EXTRN   _fpMsgBox               :dword
        EXTRN   _fpCrunch               :dword
        EXTRN   _pPitSpeedLimitEnabled  :dword
        EXTRN   _pIsHumanFlag           :dword  ; nur keurzzeitig fur CalcSetup gebracuht
        EXTRN   _pUseAdvSetup           :dword
        EXTRN   _pPlayerSetup           :dword

        EXTRN   _picbufptr              :dword
        EXTRN   _flagfield              :dword

        ;------- Hooks
        EXTRN   _fpEOFCode              :dword
        EXTRN   _fpSOLCode              :dword
        EXTRN   _fpPICode               :dword
        EXTRN   _fpPOCode               :dword
        EXTRN   _fpRetireCode           :dword
        EXTRN   _fpLOSCode              :dword
        EXTRN   _fpSOSCode              :dword
        EXTRN   _fpECPCode              :dword
        EXTRN   _fpLCPCode              :dword
        EXTRN   _fpTFLCode              :dword
        EXTRN   _fpLDFCode              :dword
        EXTRN   _fpSDFCode              :dword
        EXTRN   _fpSPDFCode             :dword
        EXTRN   _fpInitGP2Code          :dword
        EXTRN   _fpPrfCode              :dword

        EXTRN   _pSessionMode           :dword
        EXTRN   _pIsReplay              :dword
        EXTRN   _pIsAccTime             :dword
        EXTRN   _ppPlayerCS             :dword
        EXTRN   _ppCockpitCS            :dword
        EXTRN   _ppSelectedCS           :dword
        EXTRN   _pFrameTime             :dword
        EXTRN   _pRealFrameTime         :dword
        EXTRN   _pTrackBuf              :dword
        EXTRN   _pTrackFileLength       :dword
        EXTRN   _pTrackNr               :dword
        EXTRN   _pTrackIndex            :dword
        EXTRN   _pNumTrackSegs          :dword
        EXTRN   _pTrackSegs             :dword
        EXTRN   _pCurbData              :dword
        EXTRN   _pWheelStructs          :dword
        EXTRN   _dwTrackChecksum        :dword
        EXTRN   _ardwInitVals           :dword
        EXTRN   _pFastLapCars           :dword
        EXTRN   _pDriverNames           :dword
        EXTRN   _pTeamNames             :dword
        EXTRN   _pEngineNames           :dword
        EXTRN   _pCurTime               :dword
        EXTRN   _pSesStartTime          :dword
        EXTRN   _pRaceDistPerc          :dword
        EXTRN   _pNumCars               :dword
        EXTRN   _pCarIDs                :dword
        EXTRN   _pRaceTimes             :dword
        EXTRN   _arCurTyreSet           :dword
        EXTRN   _ppFileInfo             :dword
        EXTRN   _pHotlapSel             :dword
        EXTRN   _pSteeringHelp          :dword
        EXTRN   _pOppLockHelp           :dword
        EXTRN   _pFileHdr               :dword
        EXTRN   _pCarShape              :dword
        EXTRN   _pPitSpeedLimit         :dword
        EXTRN   _pRetireLimit           :dword
        EXTRN   _pDamageTrack           :dword
        EXTRN   _pDamageCars            :dword
        EXTRN   _pDetailLevel           :dword
        EXTRN   _pFrameWaitCode         :dword
        EXTRN   _pCarSetups             :dword
        EXTRN   _pGP2Dir                :dword
        EXTRN   _pPerfFlag              :dword
        EXTRN   _pTeamHPQual            :dword
        EXTRN   _pNumLapsDone           :dword
        EXTRN   _pFileDataLen           :dword
        EXTRN   _pFileDataLen2          :dword
        EXTRN   _pPaused                :dword
        EXTRN   _pNumCars2              :dword
        EXTRN   _pIsKPH                 :dword
        EXTRN   _arTrackTyreWear        :dword
        EXTRN   _arGearRevsTable        :dword
        EXTRN   _arLightRevsTable       :dword

        ; set by me
        EXTRN   _pCurrentCS             :dword

        ; set by hooked code
        EXTRN   _dwOldFileBufLen        :dword


swAlwaysTrue    dd      1
swAlwaysFalse   dd      0

include "frankasm.inc"

;--------------------------------------------------------------------------------------------

;flagfield:
; b0 == rmhook 21 succeeded

RcvGP2Call_:
                pop     EAX         ; "CALL EAX" nachfolge opcode adr vom stack
                sub     eax, 7      ; EAX := Adr von "MOV EAX,... CALL EAX"
                push    eax         ; EAX now pointing to the 8 bytes to be restored

                pushfd
                pushad
                ;---- restore -----
                cli
                ;---- 08/99 changed entry point -----
                mov     dword ptr [eax],0b441ea80h
                mov     dword ptr [eax+4],3c21cd0eh
                ;---- now have to make ESI point to magic adr table -----
                push    eax
                sub     eax,0b8eh
                mov     esi,[eax]
                pop     eax

                ;mov     ax,DGROUP            ;doch lieber mit eigenem selector...
                ;mov     ds,ax
                cmp     ds:_GP2_Found,1
                je      SchonFound
                mov     ds:_GP2_Found,1      ;Flag fuer Lamm gefunden

                ;====== wichtige Variablen setzen ==============
                mov     ds:_GP2_FoundAdr,eax
                mov     ax,cs
                mov     ds:_GP2_CSel,ax
                mov     ax,ds
                mov     ds:_GP2_DSel,ax

                mov     eax,[esi]
                mov     ds:_GP2_CodeStartAdr,eax
                mov     eax,[esi+4]
                mov     ds:_GP2_CodeEndAdr,eax
                mov     eax,[esi+8]
                mov     ds:_GP2_DataStartAdr,eax
                mov     eax,[esi+12]
                mov     ds:_GP2_DataEndAdr,eax
                mov     eax,[esi+16]
                mov     ds:_GP2_wcodeStartAdr,eax
                mov     eax,[esi+20]
                mov     ds:_GP2_wcodeEndAdr,eax
                mov     eax,[esi+24]
                mov     ds:_GP2_Data2StartAdr,eax
                mov     eax,[esi+28]
                mov     ds:_GP2_Data2EndAdr,eax
                ;====== wichtige Variablen setzen == ENDE ======

                or  dword ptr ds:_flagfield,1

                ;==== Ein paar wichtige Pointer setzen =======
;--- Patch data
                mov     eax,[esi]            ; _GP2_CodeStartAdr
				
                ;--- 8/98 --- for all versions -----
                push    eax
                mov     edi,offset wwdata_locs
                sub     eax,10020h
                call    WWPatchDataHooks
                pop     eax
				
                push    eax
                mov     edi,offset code_locs
                sub     eax,10020h
                call    PatchDataHooks
                pop     eax
				
;--- Patch code	
                push    eax
                mov     edi,offset code_hooks
                sub     eax,10020h
                call    CondPatchCodeHooks
                pop     eax

;--- Call Rene's init code
                call    ds:_fpInitGP2Code
				
                ;===== ENDE =====
				
                ;-- EBX := _GP2_CodeStartAdr -----------
                mov     eax,[esi]           ; _GP2_CodeStartAdr
                mov	    ebx,eax
				
                ;=========================
                call    FranksCodeStuff
                ;=========================
				
                ;===================================
                ;===================================
                ;===================================
				
                ;--------- normal weiter ---------
SchonFound:
                popad
                popfd
                retn


;------------------------------------------------------------------------

CondPatchCodeHooks:
; eax = code offset
; edi = table offset
                pushad
                mov     ecx,eax
pchLoop:
                mov     ebx,[edi]               ; address of call to sub
                test    ebx,ebx                 ; zero means end of table
                jz      pchEnd

                ;--- 10/98 --- test hook condition ------
                mov     ebp,[edi+4]      ; load ptr
                cmp     dword ptr [ebp],0
                je      pchNextItem
                ;-------

                add     ebx,ecx
                mov     eax,[ebx+1]             ; relative offset to sub
                cmp     byte ptr [ebx],0e8h     ; call with relative offset
                jz      pchRel
						
                sub     eax,5                   ; assume absolute call
                jmp     pchCont
pchRel:					
                add     eax,ebx                 ; add address of call
                                                ; eax = address of sub-5
pchCont:				
                mov     esi,[edi+12]            ; our hook's patch location, must be relative call to stub
                sub     eax,esi
                mov     [esi+1],eax             ; patch our hook to call original sub first
                mov     esi,[edi+8]             ; our hook
                mov     eax,esi
                sub     eax,ebx
                sub     eax,5
                mov     byte ptr [ebx],0e8h     ; always relative
                mov     [ebx+1],eax             ; patch code to call our hook
pchNextItem:			
                add     edi,16                  ; next table entry
                jmp     pchLoop
pchEnd:			
                popad
                retn



PatchDataHooks:
; eax = data offset
; edi = table offset
                pushad
pdhLoop:		
                mov     ebx,[edi]               ; relative offset of data
                test    ebx,ebx                 ; zero means end of table
                jz      pdhEnd
						
                add     ebx,eax                 ; absolute address of data
                mov     edx,[edi+4]             ; our variable to put data address
                mov     [edx],ebx               ; store address
						
                add     edi,8                   ; next table entry
                jmp     pdhLoop
pdhEnd:			
                popad
                retn


;-----------------------------------------------------------------------
; Patch data locations for all gp2.exe versions
; (based on unique code for all these versions)
; eax = code offset
; edi = ww table offset
;-----------------------------------------------------------------------
WWPatchDataHooks        proc    near
                pushad

WWpdhNext:      mov     ebx,[edi]       ;load IDA-code-address of pointer
                or      ebx,ebx         ;end of table?
                jz      WWpdhOk

                add     ebx,eax         ;make it absulute pointer
                mov     ebx,[ebx]       ;get the real datapointer from the CS
                add     ebx,[edi+4]     ;add ofs if necc.; f.i. needed for steerhelp

                mov     ecx,[edi+8]     ;load gp2lap pointer, where to store EBX
                mov     [ecx],ebx       ;store it now

                add     edi,12          ;next table item
                jmp     WWpdhNext

WWpdhOk:        popad
                retn
WWPatchDataHooks        endp

;include "checkold.inc"


;------------------------------------------------------------------------
;
;       Hook code
;
;------------------------------------------------------------------------


;------ Not used yet -----
;Hook_InitGP2:  call    CodeStub_
;               pushfd
;               pushad
;               mov     edi,dword ptr [ds:_fpInitGP2Code]
;               test    edi,edi
;               jz      higp2_end
;               call    edi
;higp2_end:
;               popad
;               popfd
;               retn


CodeStub_:      retn


;------ Track file loaded -----
Hook_TFL:
                call    CodeStub_
                pushfd
                pushad
				
                movzx   eax,bx
                shl     eax,16
                mov     ax,dx
                mov     ds:_dwTrackChecksum,eax
						 
                mov     eax,dword ptr [pFileErrno]
                cmp     byte ptr [eax],0        ; check for errors
                jnz     htfl_end
                mov     edi,dword ptr [ds:_fpTFLCode]
                test    edi,edi
                jz      htfl_end
                call    edi
htfl_end:		
                popad
                popfd
                retn


;------ Load data file -----
Hook_LDF:
                call    CodeStub_
				jb		hldf_fail

                pushfd
                pushad
                mov     edi,dword ptr [ds:_fpLDFCode]
                test    edi,edi
                jz      hldf_end

                call    edi
hldf_end:		
                popad
                popfd
hldf_fail:
                retn



;------ Save data file -----
Hook_SDF:
                pushfd
                pushad
				
                mov     edi,dword ptr [ds:_fpSDFCode]
                test    edi,edi
                jz      hsdf_end
                call    edi
hsdf_end:		
                popad
                popfd
                mov     eax,dword ptr [ds:_pFileDataLen2]
				test	eax,eax
				jz		hsdf_pl
				mov		eax,[eax]
hsdf_pl:		
                call    CodeStub_
                retn


;------ Save perf data file -----
Hook_SPDF:
                pushfd
                pushad
				
                mov     edi,dword ptr [ds:_fpSPDFCode]
                test    edi,edi
                jz      hspdf_end
                call    edi
hspdf_end:		
                popad
                popfd
hspdf_pl:		 
                call    CodeStub_
                retn



;------ Start of session -----
Hook_SOS:       call    CodeStub_
                pushfd
                pushad
				 
                ;--- Frank 8/98 ----------
                call    dword ptr ds:_fpAHFSOSCode
                popad
                pushad
                ;----
				 
                mov     edi,dword ptr [ds:_fpSOSCode]
                test    edi,edi
                jz      hsos_end
                call    edi
hsos_end:		 
                popad
                popfd
                retn



;------ Load of session -----
Hook_LOS:       call    CodeStub_
                pushfd
                pushad
				 
                mov     edi,dword ptr [ds:_fpLOSCode]
                test    edi,edi
                jz      hlos_end
                call    edi
hlos_end:		 
                popad
                popfd
                retn



;------ Start of lap -----
Hook_SOL:       call    CodeStub_
                pushfd
                pushad
				 
                ;--- Frank 8/98 ----------
                mov     eax,0
                call    dword ptr ds:_fpAHFCrossingSplit
                popad
                pushad
                ;----
				 
                mov     [ds:_pCurrentCS],esi
                mov     edi,dword ptr [ds:_fpSOLCode]
                test    edi,edi
                jz      hsos_end
                call    edi
hsol_end:		 
                popad
                popfd
                retn


;------ Pit in -----
Hook_PI:
                pushfd
                pushad
				 
                mov     [ds:_pCurrentCS],esi
                mov     edi,dword ptr [ds:_fpPICode]
                test    edi,edi
                jz      hpi_end
                call    edi
hpi_end:		 
                popad
                popfd
hpi_pl:			 
                call    CodeStub_       ; reset car on pit in
                retn



;------ Pit out -----
Hook_PO:
                call    CodeStub_
                pushfd
                pushad
				 
                mov     [ds:_pCurrentCS],esi
                mov     edi,dword ptr [ds:_fpPOCode]
                test    edi,edi
                jz      hpo_end
                call    edi
hpo_end:		 
                popad
                popfd
                retn



;------ End of frame -----
Hook_EOF:       call    CodeStub_
                pushfd
                pushad
				 
                mov     [ds:_pCurrentCS],esi
                mov     edi,dword ptr [ds:_fpEOFCode]
                test    edi,edi
                jz      heof_end
                call    edi
heof_end:		 
                popad
                popfd
                retn



;------ Retirement -----
Hook_Retire:    call    CodeStub_
                pushfd
                pushad
				
                mov     [ds:_pCurrentCS],esi
                mov     edi,dword ptr [ds:_fpRetireCode]
                test    edi,edi
                jz      hretire_end
                call    edi
hretire_end:	
                popad
                popfd
                retn




;------ Perf data -----
Hook_Prf:       call    CodeStub_
                pushfd
                pushad
				
                mov     [ds:_pCurrentCS],esi
                mov     edi,dword ptr [ds:_fpPrfCode]
                test    edi,edi
                jz      hprf_end
                call    edi
hprf_end:		
                popad
                popfd
                retn




;-----------------------------------------------------------------------------------
;
;       Hook tables
;
;       When addresses from IDA are used directly in these tables,
;       offsetting them has to be accounted for in the offset parameter
;       in the call to the corresponding patch routine.
;
;-----------------------------------------------------------------------------------

;-----------------------------------------------------------------------------------
; Format: [IDA-code-address], [hookcond ptr], [gp2lap hook function], [gp2lap org hook jump address]
;-----------------------------------------------------------------------------------
code_hooks:
                dd      32c5ch, swAlwaysTrue, Hook_EOF,         Hook_EOF        ; end of frame
                dd      35e26h, swAlwaysTrue, Hook_SOS,         Hook_SOS        ; start of session
                dd      6b6d6h, swAlwaysTrue, Hook_LOS,         Hook_LOS        ; load of session
                dd      16339h, swAlwaysTrue, Hook_SOL,         Hook_SOL        ; start of lap
                dd      93e0fh, swAlwaysTrue, Hook_TFL,         Hook_TFL        ; track file loaded
                dd      9330eh, swAlwaysTrue, Hook_LDF,         Hook_LDF        ; load data file
                dd      93624h, swAlwaysTrue, Hook_SDF,         hsdf_pl         ; save data file
                dd      979cfh, swAlwaysTrue, Hook_SPDF,        hspdf_pl        ; save perf data file
                dd      2ab15h, swAlwaysTrue, Hook_PI,          hpi_pl          ; pit in (on jacks)
                dd      2acc6h, swAlwaysTrue, Hook_PO,          Hook_PO         ; pit out (off jacks)
                dd      19f87h, swAlwaysTrue, Hook_Retire,      Hook_Retire     ; retire; (other check at 31916+...)
                dd      1c8f9h, swAlwaysTrue, Hook_Prf,         Hook_Prf        ; perf data
                dd      0
;!!!!!!!!
;WARNING: there's a second table in frankasm.inc. Check first for interferings...
;!!!!!!!!


;--- 8/98 ---- new for WWPatchDataHooks ---------------------------------------------------------------
; Format: [IDA code address], [to be added to data address], [gp2lap pointer]   ;[old (UK) data offset]
;------------------------------------------------------------------------------------------------------
wwdata_locs:
                dd      11a77h, 0,      ds:_pSessionMode        ;17964ah
                dd      11a0ch, 0,      ds:_pCurTime            ;0d4804h
                dd      15f5eh, 0,      ds:_pSesStartTime       ;0d47e0h
                dd      2f6bdh, 0,      ds:_pIsReplay           ;0c0428h
                dd      11bcfh, 0,      ds:_pIsAccTime          ;1752a5h
                dd      35b36h, 0,      ds:_pUseSVGA            ;1770dch
                dd      167eeh, 0,      ds:_pIsHumanFlag        ;0d5694h ; nur keurzzeitig fur CalcSetup gebraucht
                dd      16b6fh, 0,      ds:_pUseAdvSetup        ;0d5693h
                dd      167f6h, 0,      ds:_pPlayerSetup        ;1771b8h
                dd      14df9h, 0,      ds:_pCarStructs         ;0cdbcch
                dd      15041h, 0,      ds:_ppPlayerCS          ;0d47cch
                dd      11c8ch, 0,      ds:_ppCockpitCS         ;0d47b0h
                dd      11ac2h, 0,      ds:_ppSelectedCS        ;0d47b4h
                dd      17369h, 0,      ds:_pWheelStructs       ;0d3724h
                dd      1799eh, 0,      ds:_pFrameTime          ;1770fah
                dd      1d30eh, 0,      ds:_pRealFrameTime      ;0d6068h
                dd      7ae04h, 0,      ds:_pTrackBuf           ;3b9ad0h
                dd      8b4d1h, 0,      ds:_pTrackFileLength    ;4d291ch
                dd      93dcch, 0,      ds:_pTrackNr            ;4d0a0ch
                dd      11a69h, 0,      ds:_pTrackIndex         ;1770e4h
                dd      149a7h, 0,      ds:_pNumTrackSegs       ;0c7e34h
                dd      14178h, 0,      ds:_pTrackSegs          ;146d14h
                dd      1f3feh, 0,      ds:_pCurbData           ;0c82b8h
                dd      8b515h, 0,      pFileErrno              ;4d48b5h
                dd      22cb7h, 0,      ds:_ardwInitVals        ;0d5774h ; Init values 487 dwords. Physics: d5e50h, 33 dwords
                dd      16213h, 0,      ds:_pFastLapCars        ;17990dh
                dd      14fd3h, 0,      ds:_pDriverNames        ;179026h
                dd      7be4bh, 0,      ds:_pTeamNames          ;1793e6h
                dd      7be5fh, 0,      ds:_pEngineNames        ;1794eah
                dd      1711eh, 0,      ds:_pRaceDistPerc       ;1770eeh
                dd      14f24h, 0,      ds:_pNumCars            ;0ccb54h
                dd      14eb6h, 0,      ds:_pCarIDs             ;0ccb58h
                dd      2c305h, 0,      ds:_pRaceTimes          ;179d37h
                dd      2bf44h, 0,      ds:_arCurTyreSet        ;0ccba8h
                dd      88455h, 0,      ds:_ppFileInfo          ;4d06f3h
                dd      15ff4h, 0,      ds:_pHotlapSel          ;174bdch
                dd      726deh, 3,      ds:_pSteeringHelp       ;175936h
                dd      726deh, 0dh,    ds:_pOppLockHelp        ;175940h
                dd      927fdh, 0,      ds:_pFileHdr            ;4d0707h ; standard file header
                dd      370d5h, 0,      ds:_pCarShape           ;0e928ch
                dd      1997fh, 0,      ds:_pPitSpeedLimit      ;0d55f0h
                dd      32942h, 0,      ds:_pRetireLimit        ;0cbd24h ; 2 words
                dd      32b0ch, 0,      ds:_pDamageTrack        ;0c7a60h ; 0x29 dwords
                dd      32a31h, 0,      ds:_pDamageCars         ;0c75bch ; 0x69 dwords (masks included)
                dd      3471ch, 0,      ds:_pDetailLevel        ;0c0452h
                dd      16810h, 0,      ds:_pCarSetups          ;177218h
                dd      7b09fh, 0,      ds:_pGP2Dir             ;4d2723h
                dd      1e5cdh, 0,      ds:_pPerfFlag           ;0d4890h ; lap to be perfed
                dd      2d697h, 0,      ds:_pTeamHPQual         ;174598h ; power values (2*20 words), skill values (40*2 words + 40*2 words), failure probability (20 words)
                dd      15957h, 0,      ds:_pNumLapsDone        ;0d486ch ; laps completed in race
                dd      935ffh, 0,      ds:_pFileDataLen        ;4d08ach ; temporary length for data when saving file
                dd      93620h, 0,      ds:_pFileDataLen2       ;4d08a4h ; same, but added with 0x20000
                dd      35c80h, 0,      ds:_pPaused             ;0d354eh
                dd      14ef0h, 0,      ds:_pNumCars2           ;0cda44h ; real #cars that started the session
                dd      81956h, 0,      ds:_pIsKPH              ;1770f4h ; 0xffff to use KPH
                dd      76f84h, 0,      ds:_arTrackTyreWear     ;0d57f4h
                dd      16db7h, 0,      ds:_arGearRevsTable     ;178118h
                dd      6d68bh, 0,      ds:_arLightRevsTable    ;0d6700h

                ;--- Frank -- added 8/98 --- sorry for some twice, I'll change that ---
                dd      35b36h, 0,      ds:_GP2_use_svga        ;1770dch
                dd      11a77h, 0,      ds:_GP2_RaceMode        ;17964ah
                dd      14fd3h, 0,      ds:_GP2_DriverNames     ;179026h
                dd      2c305h, 0,      ds:_GP2_RaceTimes       ;179d37h
                dd      15a8ch, 0,      ds:_GP2_NumCarsRunning  ;0D4848h
                dd      14ef0h, 0,      ds:_GP2_NumStarters     ;0cda44h
                dd      1595fh, 0,      ds:_GP2_LapsInThisRace  ;179638h
                dd      2c32ah, 0,      ds:_GP2_RaceOrder       ;179CF5h
                dd      16213h, 0,      ds:_GP2_FastestCars     ;17990dh
                dd      15db4h, 0,      ds:_GP2_LapsRunByCar    ;0CD210h
                dd      14ec3h, 0,      ds:_GP2_CaridTeamTab    ;178f9ah
 						
                dd      15ee4h, 0,      ds:_GP2_BestLaptimes    ;179935h
                dd      15eefh, 0,      ds:_GP2_BestSplit1      ;1799D5h
                dd      15efah, 0,      ds:_GP2_BestSplit2      ;179A75h
                dd      2bfd4h, 0,      ds:_GP2_QualTimesComb   ;179B15h  ; best qualifying time for both session
                dd      2bfbdh, 0,      ds:_GP2_QualTimesSes1   ;179BB5h  ; best time from qual session 1
                dd      2bfc4h, 0,      ds:_GP2_QualTimesSes2   ;179C55h  ; best time from qual session 2
                dd      11ac2h, 0,      ds:_pCarInView          ;0d47b4h
                dd      15041h, 0,      ds:_pPlayerCar          ;0d47cch
						
                dd      14188h, 0,      ds:_GP2_MistSeg         ;0C7D44h        ; IDAOffset des pTrackSeg0000
                dd      14df9h, 0,      ds:_GP2_Cars            ;0CDBCCh
                dd      11a0ch, 0,      ds:d_CurrTime           ;0D4804h
                dd      15f5eh, 0,      ds:d_SesStartTime       ;0D47E0h
                dd      732bbh, 0,      ds:_GP2_TranspTab       ;171080h
                dd      3b4c2h, 0,      ds:_pCameraViewSrc      ;0CA07Ch
                dd      71893h, 0,      ds:b_UpdMirrors         ;17e1d8h
                dd      713eeh, 0,      ds:b_UpdCockpit         ;0ca76ch
                dd      38ee7h, 0,      ds:pAddrCpitPCX         ;174b98h
                dd      718b3h, 0,      ds:ScrCmdBuf            ;17e1dch
                dd      7aca9h, 0,      ds:pSetVPageFunc        ;4d2944h
                dd      35b22h, 0,      ds:d_NewVideoPage       ;0ca768h
                dd      79cf3h, 0,      ds:_ppDosRmCode         ;4d26dch
                dd      6fb9fh, 0,      ds:_pRcvLnkCmd          ;175354h        ; for ipx link
                dd      914d8h, 0,      ds:_t_CmdsToBeSend      ;1753cch        ; for ipx link
                dd      91ba3h, 0,      ds:bSelBaudrate         ;175869h
                dd      7be26h, 0,      ds:_pAllMnuStrPtrs      ;50870Dh        ; for patching menu strings
                dd      370d0h, 0,      ds:_pRCRTable           ;4c4d40h

                dd      0

;-------------------------------------------------------------------
; Format: [IDA function address], [gp2lap function pointer variable]
;-------------------------------------------------------------------
code_locs:
                dd      171bch, ds:_fpCalcSetuptoESI
                dd      167ech, ds:_fpGetCaridsSetup
                dd      9397bh, fpCheckChecksum
                dd      9288dh, ds:_fpInitFileHdr
                dd      1991fh, ds:_pPitSpeedLimitEnabled
                dd      84b0fh, ds:_fpMsgBox
                dd      32c55h, ds:_pFrameWaitCode
                dd      935fdh, ds:_fpCrunch
                dd      0

;-------------------------------------------------------------------

pFileErrno      dd      0       ; byte**
fpCheckChecksum dd      0


_TEXT   ENDS
        END
