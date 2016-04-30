OBJECT=daytimetcpcli daytimetcpsrv
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
	sudo ./daytimetcpsrv &
	sleep 1
	./daytimetcpcli 127.0.0.1
	sudo killall daytimetcpsrv
.PHONY : clean
clean:
	-rm $(OBJECT)
