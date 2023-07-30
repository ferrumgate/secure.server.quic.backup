CFLAGS = -fPIC -Wall -W -O0 -g -ggdb -std=gnu17 -DHASH_FUNCTION=HASH_FNV  -I$(shell pwd)/../external/libs/include -I$(shell pwd)/../external/libs/include/lsquic
#LDFLAGS = -shared  -o libferrum.so.1.0.0 -L$(shell pwd)/../external/libs/lib -luv -lssl -lcrypto -lnghttp2
LDFLAGS =  -L$(shell pwd)/../external/libs/lib  -luv -lssl  -lcrypto  -llsquic -lhiredis -lz -lm


CFLAGSTEST =  -Wall -Wno-unused-function -W -O0 -g -ggdb -std=gnu17 -DHASH_FUNCTION=HASH_FNV   -I$(shell pwd)/../src -I$(shell pwd)/../external/libs/include -I$(shell pwd)/../external/libs/include/lsquic
LDFLAGSTEST = -L$(shell pwd)/../external/libs/lib -lcmocka -lcrypto -lssl  -llsquic -luv -lhiredis -lz -lm


SRC = src
TEST = test
OBJS_SERVER = ./ferrum_common.o ./ferrum_util.o ./ferrum_log.o ./ferrum_server.o \
				./ferrum_socket.o ./ferrum_resolve.o ./ferrum_timer.o \
				./ferrum_server_main.o 
 				

OBJS_CLIENT = ./ferrum_common.o ./ferrum_util.o ./ferrum_log.o ./ferrum_client.o\
				./ferrum_socket.o ./ferrum_resolve.o ./ferrum_timer.o \
				./ferrum_client_main.o 


OBJSTEST =  ./server_client/udpecho.o \
			../src/ferrum_common.o ../src/ferrum_util.o ../src/ferrum_log.o \
			../src/ferrum_client.o ../src/ferrum_server.o ../src/ferrum_resolve.o \
			../src/ferrum_socket.o ../src/ferrum_timer.o \
			./test_ferrum_client.o ./test_ferrum_util.o \
			./test_ferrum_socket.o ./test_ferrum_timer.o \
			./test_ferrum_resolve.o \
			./test.o
			




ifeq ($(TEST),TRUE)
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGSTEST)
else
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

endif

all: clean
	@cd $(SRC) && make -f ../Makefile client
	@cd $(SRC) && make -f ../Makefile server


client: $(OBJS_CLIENT)
	$(CC) -o secure.client.quic $(OBJS_CLIENT) $(LDFLAGS)

server: $(OBJS_SERVER)
	$(CC) -o secure.server.quic $(OBJS_SERVER) $(LDFLAGS)
	


check:
	@cd $(TEST) && make TEST=TRUE -f ../Makefile testrun
checkvalgrind:
	@cd $(TEST) && make TEST=TRUE -f ../Makefile testrunvalgrind
buildtest:
	@cd $(TEST) && make TEST=TRUE -f ../Makefile test



test: $(OBJSTEST)
	$(CC) -o ferrum.test   $(OBJSTEST) $(LDFLAGSTEST)

testrun: test
	LD_LIBRARY_PATH=$(shell pwd)/../external/libs/lib  SSLKEYLOGFILE=/home/hframed/ssl-key.log  ./ferrum.test
testrunvalgrind: test
	LD_LIBRARY_PATH=$(shell pwd)/../external/libs/lib  valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all   --gen-suppressions=all --suppressions=$(shell pwd)/valgrind.options  ./ferrum.test



clean:
	find ./$(SRC) -name "*.o" -type f -delete
	find ./$(TEST) -name "*.o" -type f -delete -not -path ./$(TEST)/docker_bind
	rm -rf $(SRC)/secure.server.quic
	rm -rf $(SRC)/secure.client.quic
	rm -rf $(TEST)/test
	rm -rf output
	rm -rf out

