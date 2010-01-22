PLATFORMS=(linux macosx mingw)

none:
	@echo "Please specify a target platform: $(PLATFORMS)"

linux:
	make -f makefile.linux

macosx:
	make -f makefile.macosx

mingw:
	make -f makefile.mingw

clean:
	rm -f *.o *.so *.dylib *.lo *.la *.dll *.exe pptest puptest

# how to get this to depend on linux/macosx/mingw
test:
	./pptest
	./puptest
