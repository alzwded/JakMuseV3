CC = cl.exe
GLUTROOT = freeglut
CFLAGS = /Ot /c /EHsc /I. /I"$(GLUTROOT)\include" /Zi /arch:SSE2
LD = link.exe
LDOPTS = /OUT:composer.exe /LIBPATH:"$(GLUTROOT)\lib" /DEBUG /PDB:composer.pdb
LIBS = freeglut.lib

.SUFFIXES:.cpp .hpp .h .obj

OBJS = composer.obj document.obj

composer.exe: $(OBJS) freeglut.dll
	$(LD) $(LDOPTS) $(OBJS) $(LIBS)

freeglut.dll:
	copy $(GLUTROOT)\bin\freeglut.dll .

{}.cpp{}.obj::
	$(CC) $(CFLAGS) $<

clean:
	del /s *.obj composer.exe
