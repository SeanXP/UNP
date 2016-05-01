OBJECT=echo_server echo_client
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

.PHONY : tag
test:
	make
	@echo "================= start echo server (Listen Port: 9669) ======================"
	./echo_server &
	-fg
	-netstat -an | grep "9669"
	@echo "================= start echo clinet and waiting INPUT... (end by EOF) ======================"
	./echo_client 127.0.0.1
	-netstat -an | grep "9669"
	killall echo_server

.PHONY : clean
clean:
	-rm $(OBJECT)
