VERSION = 0
CC = cl.exe
GLUTROOT = ..\freeglut
LEMONROOT = ..\vendor\lemon
!IF DEFINED(JAKMUSE_OPTS) && "$(JAKMUSE_OPTS)" == "debug"
CFLAGS = /Od /c /EHa /I. /Zi /arch:SSE2 /DVERSION=$(VERSION)
!ELSE
CFLAGS = /Ox /c /EHa /I. /arch:SSE2 /DVERSION=$(VERSION)
!ENDIF
#CFLAGS = /c /EHsc /I. /Zi /arch:SSE2 /DVERSION=$(VERSION) /RTC1 /analyze  /Ge /GS /Gs
LD = link.exe
LDOPTS = /OUT:jakmuse.exe /DEBUG /PDB:jakmuse.pdb
LIBS = 

.SUFFIXES:.cpp .hpp .h .obj

OBJS = tokenizer.obj parser.obj blocks.obj blocks_interpreter.obj notes_interpreter.obj jakmuse.obj jakmuse_wave.obj log.obj

jakmuse.exe: $(OBJS)
	$(LD) $(LDOPTS) $(OBJS) $(LIBS)

tokenizer.cpp: parser.h

parser.h: parser.cpp

parser.cpp: $(LEMONROOT)\lemon.exe parser.y
	$(LEMONROOT)\lemon.exe parser.y
	if EXIST parser.cpp del parser.cpp
	ren parser.c parser.cpp

{}.cpp{}.obj::
	$(CC) $(CFLAGS) $<

$(LEMONROOT)\lemon.exe: $(LEMONROOT)\lemon.c $(LEMONROOT)\lempar.c
	$(CC) /Fe:$(LEMONROOT)\lemon.exe $(LEMONROOT)\lemon.c

clean:
	del /q *.obj jakmuse.exe $(LEMONROOT)\lemon.exe parser.cpp parser.out parser.h *.ilk *.pdb
