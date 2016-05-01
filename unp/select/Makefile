OBJECT=select_srv client
CC=gcc
FLAGS=

.PHONY : TARGET
TARGET:$(OBJECT)

#-------------------------------
$(OBJECT):%:%.c
	$(CC) $< $(FLAGS) -o $@
#-------------------------------

.PHONY : tag
tag:
	ctags -R .

.PHONY : test
test:
	make
	./select_srv &
	./client 127.0.0.1

.PHONY : clean
clean:
	-rm $(OBJECT)
