CC = gcc
PROGRAMS = chat log discovery public_ip

all: $(PROGRAMS)

chat: chat.c
	$(CC) -o chat chat.c

log: log.c
	$(CC) -o log log.c

discovery: discovery.c
	$(CC) -o discovery discovery.c

public_ip:
	$(CC) -o public_ip public_ip.c

clean:
	rm -f $(PROGRAMS)

.PHONY: all clean