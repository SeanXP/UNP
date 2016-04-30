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
	# use port 13 (reserve port: 0~1024), need root authority;
	sudo ./daytimetcpsrv &
	sleep 1
	./daytimetcpcli 127.0.0.1
	sudo killall daytimetcpsrv
.PHONY : clean
clean:
	-rm $(OBJECT)
