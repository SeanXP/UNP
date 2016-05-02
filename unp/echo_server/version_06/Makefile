OBJECT=echo_server06 echo_client05
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
	@echo "================= start echo server (Listen Port: 9669) ======================"
	./echo_server06 &
	-netstat -an | grep "9669"
	@echo "================= start echo clinet and waiting INPUT... (end by EOF) ======================"
	./echo_client05 127.0.0.1
	sleep 1
	-netstat -an | grep "9669"
	-ps -a | grep "echo_server06"
	killall echo_server06

.PHONY : clean
clean:
	-rm $(OBJECT)
