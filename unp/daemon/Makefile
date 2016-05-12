OBJECT=daytimetcpsrv3
CC=gcc
FLAGS=

.PHONY : TARGET
TARGET:$(OBJECT)

#-------------------------------
$(OBJECT):%:%.c daemon_init.c
	$(CC) $^ $(FLAGS) -o $@
#-------------------------------

.PHONY : tag
tag:
	ctags -R .

.PHONY : test
test:
	./$(OBJECT)

.PHONY : clean
clean:
	-rm $(OBJECT)
