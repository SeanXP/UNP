OBJECT=udp_server udp_client
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
	./udp_server &
	-netstat -an | grep "9669"
	./udp_client 127.0.0.1
	killall udp_server
.PHONY : clean
clean:
	-rm $(OBJECT)
