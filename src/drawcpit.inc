; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ

;		S u b r o u t i n e

DrawCockpitView proc near		; sub_713EC+7p rDispMainSVGA+7Cp
					; rDispMainVGA+64p

		;jmp	 dword ptr ds:MyDCPVOrgPtr


		pushad
		mov	eax,ds:b_UpdMirrors
		mov	byte ptr [eax],0
UseOrgDCV:

		mov	eax,ds:_pCameraViewSrc
		mov	esi,[eax]

		mov	eax,ds:b_UpdCockpit
		test	byte ptr [eax], 0FFh
		jz	short L_NoCkpitAct

		mov	eax,ds:pAddrCpitPCX
		mov	esi,[eax]

L_NoCkpitAct:				; DrawCockpitView+15j

		call	MyDrawFunc_	;mov	 edi, 0A0000h
		;mov	 edi,ds:_pLFBuf

		mov	ebx,ds:ScrCmdBuf
		jmp	L_ReadNextCmd
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

L_Bit15Set:				; DrawCockpitView+B8j
		and	ecx, 0FFFF7FFFh ; kill bit15

loc_718C2:				; DrawCockpitView+F8j
					; DrawCockpitView+103j
		mov	eax,ds:b_UpdCockpit
		test	byte ptr [eax], 0FFh
		jnz	short L_CkpitAct

		;------ probably skip cockpit data -----------
		add	esi, ecx
		add	edi, ecx
		jmp	L_ReadNextCmd
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

L_CkpitAct:				; DrawCockpitView+39j
		shr	ecx, 1		; bit0 -> CF
		jnb	short L_CXBit0WasNot

		;------ bit0 was set in CX -----------
		call	rCkpitColAdjust ; hier wird das Cockpit refreshed
		or	ecx, ecx
		jz	short L_CmdCXReady

L_CXBit0WasNot: 			; DrawCockpitView+43j
		shr	ecx, 1		; former bit1 -> CF
		jnb	short L_CXBit1WasNot
		call	rCkpitColAdjust ; hier wird irgendwie eine kleine rechte
		call	rCkpitColAdjust ; obere Ecke des Cockpits gemalt

L_CXBit1WasNot: 			; DrawCockpitView+50j
		or	ecx, ecx
		jz	short L_CmdCXReady

loc_718F0:				; DrawCockpitView+74j
		call	rCkpitColAdjust ; ein dword
		call	rCkpitColAdjust
		call	rCkpitColAdjust
		call	rCkpitColAdjust
		loop	loc_718F0	; CX times

L_CmdCXReady:				; DrawCockpitView+4Cj
					; DrawCockpitView+5Ej
		jmp	short L_ReadNextCmd
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

L_CmdIsNot0x0000:			; DrawCockpitView+BEj
					; DrawCockpitView+E7j
					; DrawCockpitView+EEj
		mov	eax,ds:b_UpdCockpit
		test	byte ptr [eax], 0FFh
		jz	short L_DrawMiGo

		;------ probably skip cockpit data? -----------
		add	esi, ecx
		add	edi, ecx
		jmp	short L_ReadNextCmd
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

L_DrawMiGo:				; DrawCockpitView+7Fj
		mov	eax,ds:b_UpdMirrors
		test	byte ptr [eax], 0FFh
		jns	short L_DispMore

		;---- Test -------
		;add	 edi, ecx	 ; EDI auf Anfand Spiegeldaten setzen?
		;add	 esi, ecx
		push	eax
		add	edi, ecx	; EDI auf Anfand Spiegeldaten setzen?
		push	edi

L_NextMirrPixel:			; DrawCockpitView+98j
		dec	edi
		lodsb
		mov	[edi], al	; spiegeldaten in die beiden spiegel
		loop	L_NextMirrPixel
		pop	edi
		pop	eax
		jmp	short L_ReadNextCmd
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

;----------------------------------------------------------------
;----- hier wird jetzt wirklich der Bildschirm ausgegeben -------
;----------------------------------------------------------------
L_DispMore:
		;lodsb
		;not   al
		;stosb
		;loop  L_DispMore
		;jmp	L_ReadNextCmd


       ;mov  edx,edi
       ;mov    esi,esi		    ; ESI := ZIEL (gp2 buf)
       ;mov	edi,ds:_picbufptr    ; EDI := QUELLE (baseptr mypicbuf)
       ;sub	edi,esi 	     ; EDI := Differenz ESI zu EDI
       ;shr ecx,2
;-----------------

       pushad
       ;mov	esi,esi 	     ; ESI := ZIEL (gp2 buf)
       ;mov	edi,edi 	     ; EDI := screenarea (within vpage)
       mov     edx,ds:_picbufptr    ; EDX := QUELLE (baseptr mypicbuf)

       mov     eax,ds:d_NewVideoPage
       mov     ecx,[eax]	     ; get videopage
       mov     ebx,8000h
       shl     ebx,cl
       and     ebx,70000h
       add     edx,ebx

NextPix:
       mov     al,byte ptr [edx]
       inc     edx
       or      al,al
       jz      PixTransp

       stosb
       inc     esi
       loop    NextPix
       popad
       jmp L_ReadNextCmd

PixTransp:
       movsb
       loop    NextPix
       popad
       jmp L_ReadNextCmd

		mov	eax, ecx
		sub	ecx, edi
		sub	ecx, eax
		and	ecx, 3
		sub	eax, ecx
		jle	short LEndBytes
		rep movsb
		mov	ecx, eax
		and	eax, 3
		shr	ecx, 2
		rep movsd
 LEndBytes:	add	ecx, eax
		rep movsb

;---------------------------------------------------------
;---- Jetzt kommt der Ausgabe-Abschnitt ------------------
;--- ne Art Befehlsscript an [ebx] abarbeiten ? ----------
;---------------------------------------------------------

L_ReadNextCmd:				; DrawCockpitView+27j
					; DrawCockpitView+3Fj
					; DrawCockpitView+76j
					; DrawCockpitView+85j
					; DrawCockpitView+9Cj ...
		movzx	ecx, word ptr [ebx]
		add	ebx, 2		; set to next cmd
		or	cx, cx		; cx == Bit15 gesetzt?
		js	L_Bit15Set	; up
		jnz	short L_CmdIsNot0x0000 ; up

		;------ the cmd is 0x0000 -----------
		movzx	ecx, word ptr [ebx] ; get 2nd cmd
		add	ebx, 2
		cmp	ecx, 5		; cx == 5?
		jb	short L_2ndCmdBelow5 ; jmp, wenn change vesabank befehl?
		jz	short L_2ndCmdIs5 ; if ecx = 5

		cmp	ecx, 7		; cx == 7?
		jnz	short loc_7196B

		;------ the 2nd cmd is 0x0007 -----------
		mov	eax,ds:b_UpdMirrors
		xor	byte ptr [eax], 0FFh  ; alle bits kippen
		jmp	short L_ReadNextCmd
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

loc_7196B:				; DrawCockpitView+D0j
		jb	short L_2ndCmdIs6
		jmp	short L_2ndCmdIsAbove7
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

L_2ndCmdIs5:				; DrawCockpitView+CBj
		movzx	ecx, word ptr [ebx] ; load 3rd cmd
		add	ebx, 2
		or	ecx, ecx	; 3rd cmd == 0?
		jnz	short L_CmdIsNot0x0000 ; jmp if != 0
		mov	ecx, 65536	; a whole page of data??
		jmp	short L_CmdIsNot0x0000
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

L_2ndCmdIs6:				; DrawCockpitView+DBj
		movzx	ecx, word ptr [ebx] ; load 3rd cmd
		add	ebx, 2
		or	ecx, ecx
		jnz	loc_718C2
		mov	ecx, 65536	; a whole page of data??
		jmp	loc_718C2
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

L_2ndCmdBelow5: 			; DrawCockpitView+C9j

		pushad
		mov	eax, ecx	; ECX was VESA bank to set
		mov	ebx,ds:d_NewVideoPage
		mov	[ebx], eax
		;cmp	 dword ptr ds:_pVesaPMFunc5,0
		;je	 L_CantSwitch
		;call	 dword ptr ds:_pVesaPMFunc5
		mov	ebx,ds:pSetVPageFunc
		call	dword ptr [ebx]
		popad
		and	edi, 0FFFFh
		add	edi, 0A0000h

		;and	 edi, 0FFFFh
		;add	 edi, 0A0000h

		jmp	short L_ReadNextCmd
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

L_2ndCmdIsAbove7:			; DrawCockpitView+DDj
		popad
		retn
DrawCockpitView endp

;----------------------------------------------------------------------

rCkpitColAdjust proc	near	    ; means currently all standard cockpits
		push	eax
		lodsb
		stosb
		pop	eax
		retn
rCkpitColAdjust endp
