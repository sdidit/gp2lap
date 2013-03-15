################################################################
# Paths

BUILDDIR=build
SRCDIR=src
DOCDIR=doc
INCDIR=inc
LIBDIR=lib
OUTDIR=out
PUBDIR=pub


################################################################
# Settings

#DEBUG=1
#TEST=1
#AUTH=1
#SOCKCLNT=1

WSTUB=$(BUILDDIR)\wstub.dat
ZIPFILES=$(PUBDIR)\gp2lap.exe $(BUILDDIR)\gp2lap.pif $(BUILDDIR)\gp2lap.cfg $(DOCDIR)\gp2lap.txt $(DOCDIR)\faq.txt
PUBDOCFILES=$(PUBDIR)\gp2lap_faq.txt $(PUBDIR)\gp2lap_txt.html $(PUBDIR)\gp2lapfr.txt

################################################################
# Tools

CC=wcc386
LINK=wlink
AS=wasm
RM=del
CP=copy
MKDIR=mkdir
ZIP=zip
COMPRESS=build\upx


################################################################
# Tool options

CCOPTS=-i=$(INCDIR) -bt=dos4g -sg -w=5 -wcd=201 -wcd=202 -wcd=300 -wcd=301 -wcd=400
LINKOPTS=Libpath $(LIBDIR) system dos4g option STUB=$(WSTUB)
ASOPTS=-bt=dos4g
RMOPTS=/e /q
CPOPTS=
MKDIROPTS=
ZIPOPTS=-9 -j
COMPRESSOPTS=-1

!ifdef TEST
CCOPTS=$(CCOPTS) -dTEST
WWWDIR=www\beta
!endif

!ifdef DEBUG
CCOPTS=$(CCOPTS) -hc -d3 -dTEST
LINKOPTS=$(LINKOPTS) debug Codeview option CVPACK
!endif

!ifndef SOCKCLNT
LINKOPTS=$(LINKOPTS) option stack=20000
!else
LINKOPTS=$(LINKOPTS) option stack=300000
CCOPTS=$(CCOPTS) -i=inc\watt -dSOCKCLNT
!endif

!ifdef AUTH
CCOPTS=$(CCOPTS) -i=inc -dAUTH
!endif


################################################################
# Targets

OBJS=	$(OUTDIR)\gp2lap.obj $(OUTDIR)\gp2hook.obj $(OUTDIR)\gp2log.obj			\
		 $(OUTDIR)\lammcall.obj $(OUTDIR)\basiclog.obj $(OUTDIR)\misc.obj		\
		 $(OUTDIR)\track.obj $(OUTDIR)\cfgmain.obj $(OUTDIR)\myfntlib.obj		\
         $(OUTDIR)\svgabmp.obj $(OUTDIR)\timinpic.obj $(OUTDIR)\rbehind.obj		\
		 $(OUTDIR)\timinstd.obj $(OUTDIR)\dpmi.obj $(OUTDIR)\vislog.obj			\
		 $(OUTDIR)\miscahf.obj $(OUTDIR)\int9.obj $(OUTDIR)\rlaptime.obj		\
		 $(OUTDIR)\myint21.obj $(OUTDIR)\fixint2f.obj $(OUTDIR)\vesa.obj		\
		 $(OUTDIR)\trackmap.obj $(OUTDIR)\gp2glob.obj $(OUTDIR)\trackinf.obj	\
		 $(OUTDIR)\carinfo.obj $(OUTDIR)\keyqueue.obj $(OUTDIR)\keyhand.obj		\
		 $(OUTDIR)\gp2pics.obj $(OUTDIR)\gp2misc.obj $(OUTDIR)\gp2str.obj		\
		 $(OUTDIR)\prflog.obj

!ifdef AUTH
OBJS=$(OBJS) $(OUTDIR)\auth.obj
!endif

SRCOBJS=$(SRCDIR)\ipx\ipxesr.obj
LIBS=

!ifdef SOCKCLNT
OBJS=$(OBJS) $(OUTDIR)\sockclnt.obj
!ifeq LIBS
LIBS=$(LIBDIR)\wattcpwf.lib
!else
LIBS=$(LIBS) $(LIBDIR)\wattcpwf.lib
!endif
!endif

!ifdef AUTH
!ifeq LIBS
LIBS=$(LIBDIR)\auth.lib
!else
LIBS=$(LIBS) $(LIBDIR)\auth.lib
!endif
!endif

!ifeq LIBS
LIBRARY=
!else
LIBRARY=LIBRARY $(LIBS: =,)
!endif


$(OUTDIR)\gp2lap.exe:	$(OBJS) $(SRCOBJS) $(LIBS)
	@$(LINK) @<<
		$(LINKOPTS) NAME $@ FILE $(OBJS: =,), $(SRCOBJS: =,) $(LIBRARY)
<<

{$(SRCDIR)}.c{$(OUTDIR)}.obj:
	@$(CC) @<<
		$(CCOPTS) -fo=$@ -fr=$^*.err $<
<<

{$(SRCDIR)\fonts}.c{$(OUTDIR)}.obj:
	@$(CC) @<<
		$(CCOPTS) -fo=$@ -fr=$^*.err $<
<<

{$(SRCDIR)\ipx}.c{$(OUTDIR)}.obj:
	@$(CC) @<<
		$(CCOPTS) -fo=$@ -fr=$^*.err $<
<<

{$(SRCDIR)\svga}.c{$(OUTDIR)}.obj:
	@$(CC) @<<
		$(CCOPTS) -fo=$@ -fr=$^*.err $<
<<

{$(SRCDIR)\timing}.c{$(OUTDIR)}.obj:
	@$(CC) @<<
		$(CCOPTS) -fo=$@ -fr=$^*.err $<
<<

{$(SRCDIR)}.asm{$(OUTDIR)}.obj:
	@$(AS) $(ASOPTS) -fo=$@ -fr=$^*.err $<

$(PUBDIR)\gp2lap_faq.txt:	$(DOCDIR)\faq.txt
	@$(CP) $(CPOPTS) $(DOCDIR)\faq.txt $@

$(PUBDIR)\gp2lap_txt.html:	$(DOCDIR)\gp2lap_txt.html
	@$(CP) $(CPOPTS) $(DOCDIR)\gp2lap_txt.html $@

$(PUBDIR)\gp2lapfr.txt:	$(DOCDIR)\gp2lapfr.txt
	@$(CP) $(CPOPTS) $(DOCDIR)\gp2lapfr.txt $@

$(PUBDIR)\gp2lap.exe:	$(OUTDIR)\gp2lap.exe
	@$(CP) $(CPOPTS) $(OUTDIR)\gp2lap.exe $@

$(PUBDIR)\gp2lap.zip:	$(ZIPFILES)
	@$(COMPRESS) $(COMPRESSOPTS) $(PUBDIR)\gp2lap.exe
	-@$(RM) $(RMOPTS) $@
	@$(ZIP) $(ZIPOPTS) $@ $(ZIPFILES)
	-@$(RM) $(RMOPTS) $(PUBDIR)\gp2lap.exe

pubdocs:	$(PUBDOCFILES)


init:
	-@$(MKDIR) $(MKDIROPTS) $(OUTDIR) $(PUBDIR)

publish:	$(PUBDIR)\gp2lap.zip pubdocs

clean:
	-@$(RM) $(RMOPTS) $(OUTDIR)\*.obj $(OUTDIR)\*.err

cleanall:	clean
	-@$(RM) $(RMOPTS) $(OUTDIR)\gp2lap.exe $(PUBDIR)\gp2lap.zip $(PUBDOCFILES)

all:	publish clean
