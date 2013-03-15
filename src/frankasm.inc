;flagfield:
; b1 == MyDrawFunc_ installed
; b2 == Hook_AfterGp2Init reached

        ;---- Frank 8/98 -------
        EXTRN   _fpAHFCrossingSplit     :dword
        EXTRN   _fpAHFSOSCode           :dword
        EXTRN   _fpAHFCOPCode           :dword
        EXTRN   _fpAHFGetStrNrESI       :dword
        EXTRN   _fpAHFAfterGp2Init      :dword
        ;---- Frank 8/99 -------
        EXTRN   _fpAHFGetOpSystem       :dword
        EXTRN   _fpAHFInitDivers        :dword
        EXTRN   _fpAHFInt2f1600         :dword
        EXTRN   _fpAHFInt2f1500         :dword
        EXTRN   _fpAHFInt2f150b         :dword
        EXTRN   _fpAHFInt2f4a33         :dword
        EXTRN   _fpAHFCarInViewUpd      :dword
        ;---- Frank 9/99 -------
        EXTRN   _fpAHFBefDestrBest      :dword
        ;---- Frank 10/99 -------
        EXTRN   _fpAHFPitLaneArea       :dword
        ;---- Frank 12/99 -------
        EXTRN   _fpKeyMainHandler       :dword
        EXTRN   _fpTrackNrToFilename :dword
        EXTRN   _fpOnDecodePicData   :dword
        EXTRN   _fpOnImgTranspTabLoad   :dword
        EXTRN   _fpInsertLogoNow        :dword

        ;---- link stuff -------
include "linkasm.inc"

        EXTRN   _activepage                     :dword  ; for use within MyDrawFunc_
        EXTRN   _GP2_use_svga           :dword
        EXTRN   _CopySvgaLinesNum       :dword  ; for use within MyDrawFunc_, how much lines to insert from picbuf[]
        EXTRN   _GP2_RaceMode           :dword
        EXTRN   _GP2_DriverNames        :dword
        EXTRN   _GP2_RaceTimes       :dword
        EXTRN   _GP2_BestLaptimes    :dword
        EXTRN   _GP2_BestSplit1      :dword
        EXTRN   _GP2_BestSplit2      :dword
        EXTRN   _GP2_QualTimesComb   :dword
        EXTRN   _GP2_QualTimesSes1   :dword
        EXTRN   _GP2_QualTimesSes2   :dword
        EXTRN   _pCarInView          :dword
        EXTRN   _pPlayerCar          :dword
        EXTRN   _GP2_NumCarsRunning     :dword
        EXTRN   _GP2_NumStarters        :dword
        EXTRN   _GP2_LapsInThisRace     :dword
        EXTRN   _GP2_RaceOrder          :dword
        EXTRN   _GP2_FastestCars        :dword
        EXTRN   _GP2_LapsRunByCar       :dword
        EXTRN   _GP2_CaridTeamTab       :dword
        EXTRN   _GP2_MistSeg            :dword
        EXTRN   _GP2_Cars                       :dword
        EXTRN   _GP2_TranspTab          :dword
        EXTRN   _pCameraViewSrc         :dword
        EXTRN   _pAllMnuStrPtrs         :dword
        EXTRN   _pRCRTable              :dword

        ;------- not directly gp2 related ---------
        EXTRN   _pVesaPMFunc5           :dword
        EXTRN   _ForcePMBanking         :dword
        EXTRN   _ForceLFBuffering       :dword
        EXTRN   _pLFBuf                 :dword
        EXTRN   _ppDosRmCode  :dword

        ;---- general imported config switches ------
        EXTRN   _SupressFastestLap      :dword

        ;---- imported indirect switches -----


        ;---- some defined variables ------
bSelBaudrate    dd      00000000h

;========================================================================
; EBX must point to _GP2_CodeStartAdr
;========================================================================
FranksCodeStuff     proc    near
                    pushad
                    ;int     3

;--- Patch code
                    mov     eax,ebx
                    mov     edi,offset ahfcode_hooks
                    sub     eax,10020h
                    call    CondPatchCodeHooks
;------------

                    ;------------------------------------
                    ;---- first: some full takeovers ----
                    ;------------------------------------

                    ;---- 08/99 int2fh/1600h 1st of 4 takeovers -----
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,0a411ch ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h   ;call
                    mov     ecx,offset MyInt2f1600
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx
                    mov     byte ptr [eax+5],90h  ; one NOP needed
                    ;---- 08/99 int2fh/1600h 2nd of 4 takeovers -----
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,0a4146h ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h   ;call
                    mov     ecx,offset MyInt2f1600
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx
                    mov     byte ptr [eax+5],90h  ; one NOP needed
                    ;---- 08/99 int2fh/1600h 3rd of 4 takeovers -----
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,0a417bh ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h   ;call
                    mov     ecx,offset MyInt2f1600
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx
                    mov     byte ptr [eax+5],90h  ; one NOP needed
                    ;---- 08/99 int2fh/1600h 4th of 4 takeovers -----
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,0a42ceh ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h   ;call
                    mov     ecx,offset MyInt2f1600
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx
                    mov     byte ptr [eax+5],90h  ; one NOP needed
                    ;---- 08/99 int2fh/1600h takeover -----
                    ;---- wegen mov *E*ax,1600h eins laenger ----
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,07b616h ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h   ;call
                    mov     ecx,offset MyInt2f1600
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx
                    mov     word ptr [eax+5],9090h  ; two NOP needed
                    ;---- 08/99 int2fh/1500h -----
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,07b5c4h ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h   ;call
                    mov     ecx,offset MyInt2f1500
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx
                    mov     word ptr [eax+5],9090h  ; two NOP needed
                    ;---- 08/99 int2fh/150bh -----
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,07b5e6h ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h   ;call
                    mov     ecx,offset MyInt2f150b
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx
                    mov     word ptr [eax+5],9090h  ; two NOP needed
                    ;---- 08/99 int2fh/4a33h takeover -----
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,07b645h ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h   ;call
                    mov     ecx,offset MyInt2f4a33
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx
                    mov     word ptr [eax+5],9090h  ; two NOP needed


                    ;--------- new GetStrNrESI ------
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,7bee0h  ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E9h    ;jmp (flat) near
                    mov     ecx,offset MyGetStrNrESI
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx

                    ;-- Frank -- 9/98 --- new set vmode101 func ----
                    cmp     dword ptr ds:_ForceLFBuffering,0
                    je      L_MyCockpitDraw
                    ;je      L_PMBanking

                    cmp     dword ptr ds:_pLFBuf,0
                    je      L_MyCockpitDraw
                    ;je      L_PMBanking
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,7ac67h  ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     [eax],Offset MySetVMode101Func_
                    ;-------------- new DrawCameraView -------------
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,717bfh  ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h
                    mov     ecx,offset DrawCameraView_
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx

L_MyCockpitDraw:
                    jmp     L_PMBanking ;) otherwise...
                    ;-------------- new DrawCockPitView SVGA ---------
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,7181eh  ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    ;----- die Adresse des Originalcalls absolut machen -----
                    mov     ecx,[eax+1]
                    add     ecx,eax
                    add     ecx,5           ; jetzt in ecx abs address
                    mov     ds:MyDCPVOrgPtr,ecx
                    ;-------
                    mov     byte ptr [eax],0E8h
                    mov     ecx,offset DrawCockpitView
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx
                    ;-------------- new DrawCockPitView VGA ---------
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,71889h  ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h
                    mov     ecx,offset DrawCockpitView
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx
                    ;-------------- new DrawCockPitView (cpitupd) ---------
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,713f3h  ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h
                    mov     ecx,offset DrawCockpitView
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx

                    jmp     L_PMBanking

                    ;-------------- divers ---------
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,71043h  ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h
                    mov     ecx,offset My71043
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx
                    ;-------------- divers ---------
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,71d95h  ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h
                    mov     ecx,offset My71D95
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx
                    ;-------------- divers ---------
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,71f8fh  ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h
                    mov     ecx,offset My71F8F
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx
                    ;-------------- divers ---------
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,722a1h  ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     byte ptr [eax],0E8h
                    mov     ecx,offset My722A1
                    sub     ecx,5
                    sub     ecx,eax
                    mov     dword ptr [eax+1],ecx


L_PMBanking:
                    ;-- Frank -- 9/98 --- Should force pm banking? ----
                    cmp     dword ptr ds:_ForcePMBanking,0
                    je      L_No4F05Patch
                    cmp     dword ptr ds:_pVesaPMFunc5,0
                    je      L_No4F05Patch
                    ;-------- VESA 05 Patch -------------
                    mov     edi,6AD89h
                    cmp     dword ptr [ebx+edi],4F05B866h
                    jne     L_NoSelWinA
                    mov     word ptr [ebx+edi],0E890h
                    ;--------- call offset bestimmung ---------
                    mov     ecx,ebx
                    add     ecx,edi
                    add     ecx,6
                    mov     eax,ds:_pVesaPMFunc5
                    sub     eax,ecx
                    mov     [ebx+edi+2],eax
L_NoSelWinA:
                    mov     edi,6ADB2h
                    cmp     dword ptr [ebx+edi],4F05B866h
                    jne     L_NoSelWinB
                    mov     word ptr [ebx+edi],0E890h
                    ;--------- call offset bestimmung ---------
                    mov     ecx,ebx
                    add     ecx,edi
                    add     ecx,6
                    mov     eax,ds:_pVesaPMFunc5
                    sub     eax,ecx
                    mov     [ebx+edi+2],eax
L_NoSelWinB:
L_No4F05Patch:
                    ;----- 8/98 ------ blueline unterdrcken ------------
                    mov     eax,ebx         ; EAX := _GP2_CodeStartAdr
                    add     eax,7ac9dh      ; IDA Adresse
                    sub     eax,10020h      ; standardm„áig
                    mov     ecx,[eax+6]
                    mov     ds:MyCLIPOrgPtr,ecx
                    mov     [eax+6],Offset MyClipFunc_

                    ;-- 8/98 -- Funktion MenuCpyScr anzapfen ------------
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,7ac6bh  ; IDA Adresse
                    sub     eax,10020h  ; standardm„áig
                    mov     ecx,[eax+6]
                    mov     ds:MyMNUOrgPtr,ecx
                    mov     [eax+6],Offset MyMenuFunc_
                    ;--

                    ;-- 8/98 ---- patch gp2 framebuffer ----------------
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,718adh  ; IDA Adresse von 'mov edi,0A0000'
                    sub     eax,10020h  ; standardm„áig
                    cmp     dword ptr [eax],0A0000BFh
                    jne     NotMyDrawView
                    or      dword ptr ds:_flagfield,2
                    ;----- die Adresse des Originalcalls absolut machen -----
                    mov     byte ptr [eax], 0E8h
                    mov     [eax+1],Offset MyDrawFunc_
                    sub     [eax+1],eax
                    sub     dword ptr [eax+1],5
                    NotMyDrawView:

                    ;-- 9/99 ---- patch gp2 framebuffer for camview in vga ---
                    mov     eax,ebx     ; EAX := _GP2_CodeStartAdr
                    add     eax,71842h  ; IDA Adresse von 'mov edi,0A1400'
                    sub     eax,10020h  ; standardm„áig
                    cmp     dword ptr [eax],0A1400BFh
                    jne     NotMyDrawView2
                    ;----- die Adresse des Originalcalls absolut machen -----
                    mov     byte ptr [eax], 0E8h
                    mov     [eax+1],Offset DrawVgaCamView
                    sub     [eax+1],eax
                    sub     dword ptr [eax+1],5
                    NotMyDrawView2:


                    popad
                    retn
FranksCodeStuff     endp


;========================================================================
;========================================================================
;========================================================================

;---- 8/98 ---- inserting the logo ------
MyMNUOrgPtr     dd      0
MyMenuFunc_     proc    near
                pushad
                call    dword ptr ds:_fpInsertLogoNow
                popad
                jmp     dword ptr ds:MyMNUOrgPtr
MyMenuFunc_     endp

;------------------------------------------------------------------------

MyCLIPOrgPtr    dd      0
MyClipFunc_     proc    near
                pushad
                call    dword ptr ds:_fpInsertLogoNow
                popad
                ;------------------------
                jmp     dword ptr ds:MyCLIPOrgPtr
MyClipFunc_     endp

;------------------------------------------------------------------------

                ;------------------------
                  ;include "copyorg.inc"       ; too slow...
                  ;include "copy1.inc"
                ;include "copy4.inc"
                include "copy6.inc"
                ;------------------------

MyDrawFunc_     proc    near
                ;---- Test 8/98 ---------------
                ;---- šbergabe: ESI schon auf scr gesetzt -----
                ;---- Flags noch von 'test _GP2_UpdateCockpit,0FFh' gesetzt
                jnz     IgnoreNow      ; no patch for cockpitupdate
                cmp     dword ptr ds:_activepage,0
                je      IgnoreNow      ; no patching for page '0'
                pushad
                call    PatchBildBuffer
                popad
IgnoreNow:
                ;---- 12/99 -----
                pushad
                call    dword ptr ds:_fpKeyMainHandler
                popad

                mov     edi,0A0000h
                retn
MyDrawFunc_     endp

;------------------------------------------------------------------------

MyGetStrNrESI   proc    near    ; wird ANGESPRUNGEN!!!!
                jmp     dword ptr ds:_fpAHFGetStrNrESI
                ;retn
MyGetStrNrESI   endp

;------------------------------------------------------------------------

;---- 08/99 to solve the Int2f problem ---
MyInt2f1600    proc    near    ; wird angecalled
               jmp     dword ptr ds:_fpAHFInt2f1600
MyInt2f1600    endp
MyInt2f1500    proc    near    ; wird angecalled
               jmp     dword ptr ds:_fpAHFInt2f1500
MyInt2f1500    endp
MyInt2f150b    proc    near    ; wird angecalled
               jmp     dword ptr ds:_fpAHFInt2f150b
MyInt2f150b    endp
MyInt2f4a33    proc    near    ; wird angecalled
               jmp     dword ptr ds:_fpAHFInt2f4a33
MyInt2f4a33    endp


;------------------------------------------------------------------------
                ;MYVMODE  equ 4101h
                MYVMODE  equ 101h
MySetVMode101Func_:
                mov     bx, MYVMODE
                mov     ax, 4F02h
                int     10h
                mov     ax, 4F03h
                int     10h
                cmp     bx, MYVMODE        ; 640x480x256 Modus ?
                jnz     SVMErr
                clc
                retn
SVMErr:         stc
                retn

;------------------------------------------------------------------------
BarsSchon       db  00h
; ESI == source
DrawCameraView_:
                cmp     dword ptr ds:_pLFBuf,0
                je      L_NoLFB
                cmp     byte ptr ds:BarsSchon,0
                jne     JustImg

                ;--------- draw the bars --------------
                mov     byte ptr ds:BarsSchon,0FFh    ; ACHTUNG!
                mov     edi,ds:_pLFBuf
                mov     ecx,3200
                mov     eax,0
                rep stosd

                mov     edi,ds:_pLFBuf
                add     edi,40B00h
                mov     ecx,10560
                mov     eax,0
                rep stosd
JustImg:
                ;--------- draw the image --------------
                mov     edi,ds:_pLFBuf
                add     edi,3200h
                mov     ecx,63040
                rep movsd

L_NoLFB:
                pop     eax     ; call from stack
                popad
                retn

;------------------------------------------------------------------------

MyDCPVOrgPtr    dd      0
d_NewVideoPage  dd      0
pSetVPageFunc   dd      0
b_UpdMirrors    dd      0
b_UpdCockpit    dd      0
pAddrCpitPCX    dd      0
ScrCmdBuf       dd      0

include "drawcpit.inc"

My71043         proc    near
                retn
My71043         endp

My71D95         proc    near
                retn
My71D95         endp

My71F8F         proc    near
                retn
My71F8F         endp

My722A1         proc    near
                retn
My722A1         endp


;========================================================================
;========================================================================
;========================================================================

Hook_AfterGp2Init proc  near
; init some stuf, depending on gp2 variables
MyOrgAGIJmp:    db      0E8h    ; call relative
                dd      00000000h
                pushad
                or      dword ptr ds:_flagfield,4
                ;---- set selected baudrate -----
                ;mov     eax,ds:bSelBaudrate   ;doesn't work
                ;mov     byte ptr [eax],6      ;select maximum baudrate (fake)

                call    dword ptr ds:_fpAHFAfterGp2Init
                popad

                retn
Hook_AfterGp2Init endp

;----- Frank 10/98 ---- Visual Output to CamView SVGA too ------
DrawSvgaCamView proc    near
                ;---- šbergabe: ESI schon auf scr gesetzt -----
                cmp     dword ptr ds:_activepage,0
                je      NoDSCVPatch    ; no patching for page '0'
                pushad
                call    PatchBildBuffer
                popad
NoDSCVPatch:
                ;---- 12/99 -----
                pushad
                call    dword ptr ds:_fpKeyMainHandler
                popad

MyOrgSCVJmp:
                db      0E9h    ; jmp relative
                dd      00000000h
DrawSvgaCamView endp


;----- Frank 09/99 ---- Visual Output to CamView VGA too -------
DrawVgaCamView  proc    near
                ;---- šbergabe: ESI schon auf scr gesetzt -----
                cmp     dword ptr ds:_activepage,0
                je      NoDVCVPatch    ; no patching for page '0'
                pushad
                call    PatchBildBuffer
                popad
NoDVCVPatch:
                ;---- 12/99 -----
                pushad
                call    dword ptr ds:_fpKeyMainHandler
                popad

                mov     edi,0A1400h
                retn
DrawVgaCamView  endp


;----- 10/98 ---- time split1 passed (calc's already done) --------
Hook_Split1     proc    near
MyOrgSpl1Jmp:   db      0E8h    ; call relative
                dd      00000000h
                pushad
                mov     eax,1        ; split1
                call    dword ptr ds:_fpAHFCrossingSplit
                popad
                retn
Hook_Split1     endp

;----- 10/98 ---- time split2 passed (calc's already done) --------
Hook_Split2     proc    near
MyOrgSpl2Jmp:   db      0E8h    ; call relative
                dd      00000000h
                pushad
                mov     eax,2        ; split2
                call    dword ptr ds:_fpAHFCrossingSplit
                popad
                retn
Hook_Split2     endp


;----- Frank 8/98 ---- Change of Position --------------------
Hook_COP        proc    near
                pushad
                call    dword ptr ds:_fpAHFCOPCode
                popad
MyOrgCOPJmp:    db      0E9h    ; jmp relative
                dd      00000000h
Hook_COP        endp
;--- 10/99
Hook_COP2       proc    near
                pushad
                call    dword ptr ds:_fpAHFCOPCode
                popad
MyOrgCOP2Jmp:   db      0E9h    ; jmp relative
                dd      00000000h
Hook_COP2       endp



;----- Frank 8/98 ---- for every segment -----
d_CurrTime      dd      0
d_SesStartTime  dd      0
Hook_FES        proc    near
                ;--- aktuelle Segnr laden ----
                test    word ptr [edi+1Ah],2000h ; sind wir in pits?
                jz      NoPitlane

                pushad
                ;--- ja, wir sind in den pits, WatchSeg laden ----
                mov     eax,ds:_GP2_MistSeg
                mov     eax,[eax]           ; first TrackSeg laden
                mov     eax,[eax+44h]           ; „qu. in den Pits laden
                movzx   eax,word ptr [eax+1Ah]
                cmp     ax,word ptr [edi+1Ah]
                jne     PitsButOther     ; jmp, falls in anderem Segment
                ;----- yes we're in our special segemnt -----
                test    byte ptr [esi+0ACh],80h
                jnz     StillOnThatSeg       ; jmp, falls schon registriert

                ;----- ok, we're in -------
                or      byte ptr [esi+0ACh],80h    ;mark this car for being detected
                ;esi still set

                ;----- make sure, that it's a race -----------
                mov     eax,ds:_GP2_RaceMode
                test    byte ptr [eax],80h
                jz      StillOnThatSeg        ; jmp, if not

                ;----- ecx := laptime of that finished lap ------
                mov     ecx,ds:d_CurrTime
                mov     ecx,[ecx]
                sub     ecx,[esi+54h]         ; timeLapStart abziehen

                mov     [esi+2D4h],ecx        ; jetzt auf timeLast

                ;----- jetzt esi's Racetime aktualisieren -----
                movzx   eax,byte ptr [esi+0A6h]
                dec     eax
                and     eax,03Fh
                mov     ebx,ds:_GP2_RaceTimes
                add     dword ptr [ebx+eax*4],ecx ; die letzte Rundenzeit zur RaceTime dazu

                ;----- jetzt noch timeLapStart aktualisieren -----
                mov     ebx,ds:d_CurrTime
                mov     eax,[ebx]
                ;mov     ebx,ds:d_SesStartTime
                ;sub     eax,[ebx]
                mov     [esi+54h],eax         ; timeLapStart := currTime
                mov     eax,0        ; split0 (line)
                call    dword ptr ds:_fpAHFCrossingSplit
                jmp     StillOnThatSeg

PitsButOther:   popad

NoPitlane:      and     byte ptr [esi+0ACh],7Fh

MyOrgFESJmp:    db      0E9h    ; jmp relative
                dd      00000000h
                ;jmp    dword ptr ds:MyFESOrgPtr

StillOnThatSeg: popad
                jmp     MyOrgFESJmp      ;jmp    dword ptr ds:MyFESOrgPtr
Hook_FES        endp


;----- Frank 8/98 ---- for optional time reseting -----
Hook_DelTimes   proc    near
                pushfd
                pushad
                mov     eax,ds:_GP2_RaceMode
                test    byte ptr [eax],80h
                jnz     ItsARace
                mov     dword ptr [esi+54h], 0C0000000h ; reset time lap start
                mov     dword ptr [esi+2D4h], 10000000h ; reset last lap time
ItsARace:       popad
                popfd
MyOrgDTJmp:     db      0E9h    ; jmp relative
                dd      00000000h
Hook_DelTimes   endp


;----- Frank 8/98 ---- don't show fastest lap if necc. -----
Hook_PrnFastestLap proc  near
                pushfd
                cmp    dword ptr ds:_activepage,9       ; at the line?
                jne    PrnNow
                popfd
                retn
PrnNow:         popfd
MyOrgPFLJmp:    db     0E9h    ; jmp relative
                dd     00000000h
Hook_PrnFastestLap endp


;----- Frank 9/98 ---- just to set a flag, if we're really in cockpit -----
Hook_RealECP    proc    near
                pushad
                ;--- Rene 28-10-99 call Rene's ECP
                mov     edi,dword ptr [ds:_fpECPCode]
                test    edi,edi
                jz      hecp_end
                call    edi
hecp_end:
                mov     eax,00h  ; no replay | enter
                call    dword ptr ds:_fpAHFGotoCockpit
                popad
                ;-----------------------
MyOrgRealECP:   db      0E8h    ; call relative
                dd      00000000h
                ;-----------------------
                pushad
                mov     eax,01h ; no replay | leave
                call    dword ptr ds:_fpAHFGotoCockpit
                ;--- Rene 28-10-99 call Rene's LCP
                mov     edi,dword ptr [ds:_fpLCPCode]
                test    edi,edi
                jz      hlcp_end
                call    edi
hlcp_end:
                popad
                retn
Hook_RealECP    endp


;----- Frank 10/98 ---- like Hook_RealECP, but for replay -----
Hook_RealECP_R  proc    near
                pushad
                ;--- Rene 30-10-99 call Rene's ECP
                mov     edi,dword ptr [ds:_fpECPCode]
                test    edi,edi
                jz      hecp_r_end
                call    edi
hecp_r_end:
                mov     eax,80h ; replay | enter
                call    dword ptr ds:_fpAHFGotoCockpit
                popad
                ;-----------------------
MyOrgRealECP_R: db      0E8h    ; call relative
                dd      00000000h
                ;-----------------------
                pushad
                mov     eax,81h ; replay | leave
                call    dword ptr ds:_fpAHFGotoCockpit
                ;--- Rene 30-10-99 call Rene's LCP
                mov     edi,dword ptr [ds:_fpLCPCode]
                test    edi,edi
                jz      hlcp_r_end
                call    edi
hlcp_r_end:
                popad
                retn
Hook_RealECP_R  endp


;----- Frank 08/99 ---- car's cockpit gets redrawn (changed CarInView) -----
Hook_CpitUpd    proc    near
                pushad
                call    dword ptr ds:_fpAHFCarInViewUpd
                popad
                ;-----------------------
MyOrgCpitUpd    db      0E9h    ; jmp relative
                dd      00000000h
                ;-----------------------
Hook_CpitUpd    endp

;----- Frank 08/99 ---- cameraview gets redrawn (changed CarInView) -----
Hook_CamUpd     proc    near
                pushad
                call    dword ptr ds:_fpAHFCarInViewUpd
                popad
                ;-----------------------
MyOrgCamUpd     db      0E9h    ; jmp relative
                dd      00000000h
                ;-----------------------
Hook_CamUpd     endp


;----- Frank 08/99 ---- hook before gp2's determining OS -----
Hook_GetOS      proc    near
                pushad
                mov     eax,0   ; before
                call    dword ptr ds:_fpAHFGetOpSystem
                or      eax,eax
                popad
                jz      getos_end
                ;-----------------------
MyOrgGetOS:     db      0E8h    ; call relative
                dd      00000000h
                ;-----------------------
                pushad
                mov     eax,1   ; after
                call    dword ptr ds:_fpAHFGetOpSystem  ; retval uninteressant
                popad
getos_end:
                retn
Hook_GetOS      endp


;----- Frank 08/99 ---- hook before some gp2's init -----
Hook_InitSCard  proc    near
                pushad
                mov     eax,00010000h   ; before
                call    dword ptr ds:_fpAHFInitDivers
                popad
                ;-----------------------
MyOrgInitSCard: db      0E8h    ; call relative
                dd      00000000h
                ;-----------------------
                pushad
                mov     eax,00010001h   ; after
                call    dword ptr ds:_fpAHFInitDivers
                popad
                retn
Hook_InitSCard  endp
;---------------------------
Hook_LoadSyst   proc    near
                pushad
                mov     eax,00020000h   ; before
                call    dword ptr ds:_fpAHFInitDivers
                popad
                ;-----------------------
MyOrgLoadSyst:  db      0E8h    ; call relative
                dd      00000000h
                ;-----------------------
                pushad
                mov     eax,00020001h   ; after
                call    dword ptr ds:_fpAHFInitDivers
                popad
                retn
Hook_LoadSyst   endp


;----- Frank 17.09.99 ---- taking over gp'2 DecodePicData (bugfix) ------
Hook_DecodePic  proc    near
                ; another temporary buffer (instead of 0x7800 extalp)
                mov     eax,ds:_picbufptr
                pushad
                ;-----------------------
MyOrgDecodePic: db      0E8h    ; call relative
                dd      00000000h
                ;-----------------------
                popad
                ;-----------------------
                ; esi == source packed data
                ; ecx == d_anzsvgabytes
                ; edi == ziel (offset v320_640_buf)
                ; eax == d_workfieldptr
                ;-----------------------
                call    dword ptr ds:_fpOnDecodePicData
Hook_DecodePic  endp


;----- Frank 17.09.99 ---- before gp2 destroyes all best splittimes ------
Hook_BefDestrBest proc    near
                pushad
                call    dword ptr ds:_fpAHFBefDestrBest
                popad
                ;-----------------------
MyOrgBDB:       db      0E9h    ; jump relative
                dd      00000000h
                ;-----------------------
Hook_BefDestrBest  endp


;----- Frank 24.10.99 ---- car ESI enters/leaves pitlane area ------
OldPlaState     db      00h
Hook_PitLaneArea proc    near
                push    eax
                mov     al,byte ptr [esi+16Ah]
                and     al,00001000b      ; kill everything but "inpitlane" bit
                mov     byte ptr ds:OldPlaState, al
                pop     eax
                ;-----------------------
MyOrgPLA:       db      0E8h    ; call relative
                dd      00000000h
                ;-----------------------
                push    eax
                mov     al,byte ptr [esi+16Ah]
                and     al,00001000b      ; kill everything but "inpitlane" bit
                cmp     byte ptr ds:OldPlaState, al
                pop     eax
                je      PLA_HookEnd
                ;-----
                or      esi,esi     ; no carstruct??
                jz      PLA_HookEnd
                cmp     byte ptr [esi+0A6h],0    ; there are some #0 cars, dunno why
                je      PLA_HookEnd              ; filter them out!
                pushad
                call    dword ptr ds:_fpAHFPitLaneArea
                popad
PLA_HookEnd:    retn
Hook_PitLaneArea endp


;----- Frank 21.12.99 ---- gp2 doing the track nr -> filename conversion ------
Hk_TrackNr2File proc    near
                ; ecx == track nr (0..15)
                ; esi == pointer to default file name field // "circuits\f1ct01.dat",0,"circuits\f1ct02.dat"...
                ;-----------------------
                call    dword ptr ds:_fpTrackNrToFilename
                ; esi must now point to a filename
                retn
MyOrgTNF:       db      0E9h    ; jump relative
                dd      00000000h
                ;-----------------------
Hk_TrackNr2File endp

;----- Frank 29.12.99 ---- gp2 having just loaded the image transp tab ------
Hk_OnImgTransLoad proc    near
                pushad
MyOrgOITL:      db      0E8h    ; call relative
                dd      00000000h
                popad
                call    dword ptr ds:_fpOnImgTranspTabLoad
                retn
                ;-----------------------
Hk_OnImgTransLoad endp


;-----------------------------------------------------------------------------------
; Format: [IDA-code-address], [hookcond ptr], [gp2lap hook function], [gp2lap org hook jump address]
;-----------------------------------------------------------------------------------
ahfcode_hooks:
                ;--- Frank --- added 8/98 ------
                dd      2cf36h, swAlwaysTrue, Hook_COP,             MyOrgCOPJmp             ; change of Position
                dd      2d0a3h, swAlwaysTrue, Hook_COP2,            MyOrgCOP2Jmp            ; change of Position2
                dd      2bf7eh, swAlwaysTrue, Hook_FES,             MyOrgFESJmp             ; for every segment
                dd      1635bh, swAlwaysTrue, Hook_DelTimes,        MyOrgDTJmp              ; for optional time reseting
                dd      6a383h, swAlwaysTrue, Hook_RealECP,         MyOrgRealECP    ; similar to (removed) Hook_ECP2, but one level higher
                dd      6b5c4h, swAlwaysTrue, Hook_RealECP_R,       MyOrgRealECP_R  ; like the hook at 6a383h, but for hotlap etc.
                dd      6af9ah, swAlwaysTrue, Hook_AfterGp2Init,    MyOrgAGIJmp             ; after all those init stuff gp2 does at startup
                dd      717c9h, swAlwaysTrue, DrawSvgaCamView,      MyOrgSCVJmp     ; within routine for drawing cam view
                dd      16268h, swAlwaysTrue, Hook_Split1,          MyOrgSpl1Jmp    ; 1st split passed (calc's done)
                dd      16294h, swAlwaysTrue, Hook_Split2,          MyOrgSpl2Jmp    ; 2nd split passed (calc's done)
                dd      37254h, swAlwaysTrue, Hook_CpitUpd,         MyOrgCpitUpd    ; car's cockpit gets updated
                dd      37241h, swAlwaysTrue, Hook_CamUpd,          MyOrgCamUpd     ; cameraview gets updated

                dd      7ae42h, swAlwaysTrue, Hook_GetOS,           MyOrgGetOS      ; gp2's determining OS
                dd      7af80h, swAlwaysTrue, Hook_InitSCard,       MyOrgInitSCard  ; gp2's initing sound card
                dd      7aecah, swAlwaysTrue, Hook_LoadSyst,        MyOrgLoadSyst   ; gp2's loading system
                dd      8dfe1h, swAlwaysTrue, Hook_DecodePic,       MyOrgDecodePic  ; gp2's uncompressing a pic (bugfix)

                dd      6e402h, _SupressFastestLap, Hook_PrnFastestLap,   MyOrgPFLJmp

                dd      91d7ah, _LinkEstablished, MyComSendChar,  MyOrgCSCJmp       ; for ipx link
                dd      91e4dh, _LinkEstablished, MyComSendChar,  MyOrgCSCJmp       ; for ipx link
                dd      87eb0h, _LinkEstablished, MyLinkMask,     MyOrgLMJmp        ; for ipx link (disable buttons)

                ;--- Frank --- added 9/99 ------
                dd      160c6h, swAlwaysTrue, Hook_BefDestrBest,    MyOrgBDB        ; before gp2 updates a potential new besttime

                dd      1433eh, swAlwaysTrue, Hook_PitLaneArea,    MyOrgPLA        ; a car enters/leaves pitlane area
                dd      1439dh, swAlwaysTrue, Hook_PitLaneArea,    MyOrgPLA        ; a car enters/leaves pitlane area
                dd      14a45h, swAlwaysTrue, Hook_PitLaneArea,    MyOrgPLA        ; a car enters/leaves pitlane area
                dd      1666ch, swAlwaysTrue, Hook_PitLaneArea,    MyOrgPLA        ; a car enters/leaves pitlane area
                dd      17e42h, swAlwaysTrue, Hook_PitLaneArea,    MyOrgPLA        ; a car enters/leaves pitlane area
                dd      2ae58h, swAlwaysTrue, Hook_PitLaneArea,    MyOrgPLA        ; a car enters/leaves pitlane area
                dd      2b698h, swAlwaysTrue, Hook_PitLaneArea,    MyOrgPLA        ; a car enters/leaves pitlane area
                dd      2b701h, swAlwaysTrue, Hook_PitLaneArea,    MyOrgPLA        ; a car enters/leaves pitlane area
                dd      2bbe7h, swAlwaysTrue, Hook_PitLaneArea,    MyOrgPLA        ; a car enters/leaves pitlane area
                dd      31171h, swAlwaysTrue, Hook_PitLaneArea,    MyOrgPLA        ; a car enters/leaves pitlane area
                dd      311ach, swAlwaysTrue, Hook_PitLaneArea,    MyOrgPLA        ; a car enters/leaves pitlane area

                ;--- Frank --- added 12/99 ------
                dd      93deeh, swAlwaysTrue, Hk_TrackNr2File,    MyOrgTNF        ; gp2 doing the track nr -> filename conversion
                dd      8dcc4h, swAlwaysTrue, Hk_OnImgTransLoad,  MyOrgOITL       ; gp2 having just loaded the image transp tab


;-- ipx unused --;                 ;dd              88018h, MyTryToReceive1, MyOrgTTRJmp1
;-- ipx unused --;                 ;dd              880feh, MyTryToReceive2, MyOrgTTRJmp2
;-- ipx unused --;                 ;dd              8fbbbh, MyTryToReceive3, MyOrgTTRJmp3
;-- ipx unused --;                 ;dd              91534h, MyTryToReceive4, MyOrgTTRJmp4
;-- ipx unused --;                 ;;dd              91654h, MyTryToReceive5, MyOrgTTRJmp5
;-- ipx unused --;                 ;dd              6fa4bh, MyProcessInCmds, MyOrgPICJmp
;-- ipx unused --;                 ;dd              9185eh, MyWSRCmds, MyOrgWSRCmdsJmp ; within lStoreRcvdCmd
;-- ipx unused --;                 ;dd              9156eh, MyWSCISAL, MyWSCISALJmp  ; within lSendCmdInSlotAL
;-- ipx unused --;                 ;;dd              6fa0eh, MyTTR2, MyOrgTTR2Jmp // andere location (unten)
;-- ipx unused --;                 ;;dd              6fbcfh, MyTTR2, MyOrgTTR2Jmp
                dd              0

;!!!!!!!!
;WARNING: there's a second table in lammcall.asm Check first for interferings...
;!!!!!!!!
