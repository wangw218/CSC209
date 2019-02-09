PORT=54215
CFLAGS = -DPORT=$(PORT) -g -Wall -std=gnu99

all: helpcentre hcq_server

helpcentre: helpcentre.o hcq.o 
	gcc $(CFLAGS) -o helpcentre helpcentre.o hcq.o

hcq_server: hcq_server.o hcq.o
	gcc $(CFLAGS) -o hcq_server hcq_server.o hcq.o

helpcentre.o: helpcentre.c hcq.h
	gcc $(CFLAGS) -c helpcentre.c

hcq.o: hcq.c hcq.h
	gcc $(CFLAGS) -c hcq.c

hcq_server.o: hcq_server.c hcq.h
	gcc $(CFLAGS) -c hcq_server.c
	


clean: 
	rm helpcentre hcq_server *.o  
