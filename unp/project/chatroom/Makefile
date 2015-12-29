OBJECT=server client
CC=gcc
FLAGS= -pthread
SERVER_PORT=1359

TARGET:$(OBJECT)

#-------------------------------
$(OBJECT):%:%.c
	$(CC) $< $(FLAGS) -o $@
#-------------------------------


clean:
	-rm $(OBJECT)
tag:
	-ctags -R .
#查看TCP连接情况
netstat:
	-netstat -an | grep $(SERVER_PORT)
#程序执行完后, 需要手动删除创建的共享内存数据以及同步信号量
#ipcs / ipcrm -m / ipcrm -s指令
