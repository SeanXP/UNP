OBJECT=select_srv client
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
test:
	make
	./select_srv &
	./client 127.0.0.1
