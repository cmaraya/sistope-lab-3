
discos_out= lab3

discos_headers = 
discos_source  = $(discos_headers:.h=.c) discos.c
discos_objects = $(discos_source:.c=.o)		

HEB=	-lpthread
CC     = gcc
CFLAGS = -Wall

depends = .depends

build : $(discos_out) 

$(discos_out) : $(discos_objects)
	$(CC) $(CFLAGS) -o $@ $^ -lm $(HEB)		

$(objects) :
	$(CC) $(CFLAGS) -c -o $@ $*.c -lm $(HEB)

$(depends) : $(discos_source) $(discos_headers)
	@$(CC) -MM $(discos_source) > $@


clean :
	$(RM) $(discos_out) $(discos_objects) $(zipfile) $(depends)

.PHONY : build zip clean

sinclude $(depends)
