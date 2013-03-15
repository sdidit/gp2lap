        ;---- link stuff -------
        EXTRN   _fpAHFGotoCockpit       :dword
        EXTRN   _fpAHFComSendCharNow    :dword
        EXTRN   _fpAHFTryToReceive      :dword
        EXTRN   _fpAHFTryToReceiveQ     :dword

        EXTRN   _pRcvLnkCmd      :dword
        EXTRN   _t_CmdsToBeSend  :dword

        ;---- imported indirect switches -----
        EXTRN   _LinkEstablished        :dword


;-------------------------------------------------------------------

MyLinkMask      proc    near
; disable some buttons within link menu
                or      eax,01F97FF0h   ; magic bitmask
MyOrgLMJmp:     db      0E9h    ; jmp relative
                dd      00000000h
MyLinkMask      endp

MyComSendChar   proc    near
; EDI == rmcode ptr
; DX == com port
                pushad
                call    dword ptr ds:_fpAHFComSendCharNow
                popad
                retn
MyOrgCSCJmp:    db      0E9h    ; jmp relative
                dd      00000000h
MyComSendChar   endp

;-------------------------------------------------------------------

MyTryToReceive1 proc    near
                pushad
                mov     eax,1
                call    dword ptr ds:_fpAHFTryToReceive
                popad
MyOrgTTRJmp1:   db      0E9h    ; jmp relative
                dd      00000000h
                ;pushfd
                ;pushad
                ;mov     eax,1
                ;call    dword ptr ds:_fpAHFTryToReceiveQ
                ;popad
                ;popfd
                ;retn
MyTryToReceive1 endp

;-------------------------------------------------------------------

MyTryToReceive2 proc    near
                pushad
                mov     eax,2
                call    dword ptr ds:_fpAHFTryToReceive
                popad
MyOrgTTRJmp2:   db      0E9h    ; jmp relative
                dd      00000000h
MyTryToReceive2 endp
;-------------------------------------------------------------------

MyTryToReceive3 proc    near
                pushad
                mov     eax,3
                call    dword ptr ds:_fpAHFTryToReceive
                popad
MyOrgTTRJmp3:   db      0E9h    ; jmp relative
                dd      00000000h
MyTryToReceive3 endp
;-------------------------------------------------------------------

MyTryToReceive4 proc    near
                pushad
                mov     eax,4
                call    dword ptr ds:_fpAHFTryToReceive
                popad
MyOrgTTRJmp4:   db      0E9h    ; jmp relative
                dd      00000000h
MyTryToReceive4 endp
;-------------------------------------------------------------------

MyTryToReceive5 proc    near
                pushad
                mov     eax,5
                call    dword ptr ds:_fpAHFTryToReceive
                popad
MyOrgTTRJmp5:   db      0E9h    ; jmp relative
                dd      00000000h
MyTryToReceive5 endp

;-------------------------------------------------------------------

