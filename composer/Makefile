CC = cl.exe
GLUTROOT = ..\freeglut
LEMONROOT = ..\vendor\lemon
CFLAGS = /Ot /c /EHsc /I. /I"$(GLUTROOT)\include" /Zi /arch:SSE2
LD = link.exe
LDOPTS = /OUT:composer.exe /LIBPATH:"$(GLUTROOT)\lib" /DEBUG /PDB:composer.pdb
LIBS = freeglut.lib

.SUFFIXES:.cpp .hpp .h .obj

OBJS = composer.obj document.obj document_display.obj tokenizer.obj document_open.obj parser.obj

composer.exe: $(OBJS) freeglut.dll
	$(LD) $(LDOPTS) $(OBJS) $(LIBS)

freeglut.dll:
	copy $(GLUTROOT)\bin\freeglut.dll .

document_open.cpp: parser.h

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
	del /q *.obj composer.exe $(LEMONROOT)\lemon.exe parser.cpp parser.out parser.h .\freeglut.dll *.ilk *.pdb
