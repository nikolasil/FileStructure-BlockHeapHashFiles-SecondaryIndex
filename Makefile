CC = gcc
CFLAFS = -g -Wall

prog: clean main.o util.o heap.o hash.o hashing.o sht.o
	$(CC) $(CFLAFS) -o prog main.o util.o heap.o hash.o hashing.o sht.o BF/BF_64.a -no-pie -lcrypto
main.o:
	$(CC) $(CFLAFS) -c main.c 
util.o:
	$(CC) $(CFLAFS) -c util.c 
heap.o:
	$(CC) $(CFLAFS) -c HP/heap.c 
hash.o: 
	$(CC) $(CFLAFS) -c HT/hash.c 
hashing.o:
	$(CC) $(CFLAFS) -c Hashing/hashing.c
sht.o:
	$(CC) $(CFLAFS) -c SHT/sht.c
.PHONY: clean
clean:
	rm -f prog main.o heap.o util.o hash.o hashing.o sht.o file sfile Bucket* Secondary*