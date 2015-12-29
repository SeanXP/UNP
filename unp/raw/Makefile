OBJECT=raw
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
	
