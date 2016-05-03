OBJECT=fcntl
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
	make clean
	gcc -DSET_NONBLOCK_FLAG $(OBJECT).c -o $(OBJECT)
	@echo "=============== define SET_NONBLOCK_FLAG ==================="
	-./$(OBJECT)
	make clean
	make
	@echo "=============== not define SET_NONBLOCK_FLAG ==================="
	./$(OBJECT)

.PHONY : clean
clean:
	-rm $(OBJECT)
