OBJECT=echo_server echo_client
CC=gcc
FLAGS=

TARGET:$(OBJECT)

#-------------------------------
$(OBJECT):%:%.c
	$(CC) $< $(FLAGS) -o $@
#-------------------------------


clean:
	-rm $(OBJECT)
tag:
	ctags -R .
	
