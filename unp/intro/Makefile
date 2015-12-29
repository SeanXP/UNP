OBJECT=daytimetcpcli daytimetcpsrv
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
	sudo ./daytimetcpsrv &
	./daytimetcpcli 127.0.0.1
