CC = cl.exe
GLUTROOT = freeglut
CFLAGS = /Ot /c /EHsc /I. /I"$(GLUTROOT)\include" /Zi /arch:SSE2
LD = link.exe
LDOPTS = /OUT:composer.exe /LIBPATH:"$(GLUTROOT)\lib" /DEBUG /PDB:composer.pdb
LIBS = freeglut.lib

.SUFFIXES:.cpp .hpp .h .obj

OBJS = composer.obj document.obj document_display.obj

composer.exe: $(OBJS) freeglut.dll
	$(LD) $(LDOPTS) $(OBJS) $(LIBS)

freeglut.dll:
	copy $(GLUTROOT)\bin\freeglut.dll .

parser.cpp: vendor\lemon\lemon.exe parser.y
	vendor\lemon\lemon.exe parser.y
	if EXIST parser.cpp del parser.cpp
	ren parser.c parser.cpp

{}.cpp{}.obj::
	$(CC) $(CFLAGS) $<

vendor\lemon\lemon.exe: vendor\lemon\lemon.c vendor\lemon\lempar.c
	$(CC) /Fe:vendor\lemon\lemon.exe vendor\lemon\lemon.c

clean:
	del /s *.obj composer.exe vendor\lemon\lemon.exe parser.cpp parser.out parser.h
