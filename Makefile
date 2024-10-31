CC = gcc
CFLAGS = 
LDFLAGS = -I. -lm

SRCS = servere.c utils.c cJSON.c
SRCS_CLIENT = client.c utils.c cJSON.c

all: servere client

servere: $(SRCS)
	$(CC) -o servere $(SRCS) $(LDFLAGS)

client: $(SRCS_CLIENT)
	$(CC) -o client $(SRCS_CLIENT) $(LDFLAGS)

clean:
	rm -f servere client

.PHONY: all clean