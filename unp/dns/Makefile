OBJECT=dns dns_x getservbyname getaddrinfo
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
	./dns www.taobao.com
	@echo "================================"
	dig www.taobao.com
	@echo "================================"
	./dns_x 8.8.8.8
	@echo "================================"
	dig -x 8.8.8.8

.PHONY : clean
clean:
	-rm $(OBJECT)
