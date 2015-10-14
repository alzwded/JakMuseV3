CC = cl.exe
GLUTROOT = ..\freeglut
LEMONROOT = ..\vendor\lemon
CFLAGS = /Ot /c /EHsc /I. /Zi /arch:SSE2
LD = link.exe
LDOPTS = /OUT:jakmuse.exe /DEBUG /PDB:jakmuse.pdb
LIBS = 

.SUFFIXES:.cpp .hpp .h .obj

OBJS = tokenizer.obj parser.obj blocks.obj blocks_interpreter.obj notes_interpreter.obj jakmuse.obj

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
