server: ../model/model.o ../open62541-lib/open62541.o server.o
	gcc ../model/model.o ../open62541-lib/open62541.o server.o -o server

../model/model.o: ../model/model.c
	gcc -c ../model/model.c -DUA_ENABLE_AMALGAMATION=true -o ../model/model.o

../open62541-lib/open62541.o: ../open62541-lib/open62541.c
	gcc -c -std=c99 ../open62541-lib/open62541.c -o ../open62541-lib/open62541.o

server.o: server.c
	gcc -c server.c -o server.o

clean:
	rm *.o server

run:
	./server

