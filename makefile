OBJS	= c_src/shared_mem_init.o c_src/table.o 
OBJS2	= c_src/receptionist.o c_src/table.o
OBJS3	= c_src/visitor.o c_src/table.o
OBJS4	= c_src/monitor.o c_src/table.o
SOURCE	= c_src/shared_mem_init.c c_src/receptionist.c c_src/table.c c_src/visitor.c c_src/monitor.c
HEADER	= include/shared_mem_init.h include/table.h
CC		= gcc
FLAGS	= -g -c -I include

all: $(OBJS) $(OBJS2) $(OBJS3) $(OBJS4) c_src/memory.o c_src/shared_mem_init.o
	$(CC) -g $(OBJS) -o shared_mem_init -lrt -lpthread
	$(CC) -g $(OBJS2) -o receptionist -lrt -lpthread
	$(CC) -g $(OBJS3) -o visitor -lrt -lpthread
	$(CC) -g $(OBJS4) -o monitor -lrt -lpthread
	$(CC) -g c_src/memory.o -o memory -lrt -lpthread

shared_mem_init.o: c_src/shared_mem_init.c
	$(CC) $(FLAGS) c_src/shared_mem_init.c -lrt -lpthread

receptionist.o: c_src/receptionist.c
	$(CC) $(FLAGS) c_src/receptionist.c -lrt -lpthread

visitor.o: c_src/visitor.c
	$(CC) $(FLAGS) c_src/visitor.c -lrt -lpthread

table.o: c_src/table.c
	$(CC) $(FLAGS) c_src/table.c 

monitor.o: c_src/monitor.c
	$(CC) $(FLAGS) c_src/monitor.c -lrt -lpthread

memory.o: c_src/memory.c
	$(CC) $(FLAGS) c_src/memory.c

valgrind: $(OBJS) $(OBJS2) $(OBJS3) $(OBJS4)
	$(CC) -g $(OBJS) -o shared_mem_init -lrt -lpthread
	$(CC) -g $(OBJS2) -o receptionist -lrt -lpthread
	$(CC) -g $(OBJS3) -o visitor -lrt -lpthread
	$(CC) -g $(OBJS4) -o monitor -lrt -lpthread
	valgrind ./shared_mem_init -s sdi2200169
	valgrind ./receptionist -d 1 -s sdi2200169

clean:
	rm -f $(OBJS) shared_mem_init
	rm -f $(OBJS2) receptionist
	rm -f $(OBJS3) visitor
	rm -f $(OBJS4) monitor
	rm -f c_src/memory.o memory

run: $(OBJS) $(OBJS2) $(OBJS3) $(OBJS4)
	$(CC) -g $(OBJS) -o c_src/shared_mem_init -lrt -lpthread
	$(CC) -g $(OBJS2) -o c_src/receptionist -lrt -lpthread
	$(CC) -g $(OBJS3) -o c_src/visitor -lrt -lpthread
	$(CC) -g $(OBJS4) -o c_src/monitor -lrt -lpthread
	./shared_mem_init -s sdi2200169
	./receptionist -d 2 -s sdi2200169 

visit:
	./visitor -d 3 -s sdi2200169

monit:
	./monitor -s sdi2200169

example: 
	./shared_mem_init 0 1 

mem:
	./memory