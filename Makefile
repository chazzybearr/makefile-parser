BUILDDIR=build
SRCDIR=src

pmake : $(BUILDDIR)/pmake.o $(BUILDDIR)/parse.o $(BUILDDIR)/run_make.o
	$(MAKE) -C $(BUILDDIR)
	gcc -Wall -g -o pmake $(BUILDDIR)/pmake.o $(BUILDDIR)/parse.o $(BUILDDIR)/run_make.o

$(BUILDDIR)/pmake.o : 

$(BUILDDIR)/parse.o :

$(BUILDDIR)/run_make.o :


clean :
	rm pmake
	make clean -C $(BUILDDIR)
