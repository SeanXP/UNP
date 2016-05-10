OBJECT=daytimetcpcli2 daytimetcpsrv2
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
	sudo ./daytimetcpsrv2 daytime &
	sleep 1
	./daytimetcpcli2 localhost daytime
	sudo killall daytimetcpsrv2
.PHONY : clean
clean:
	-rm $(OBJECT)
