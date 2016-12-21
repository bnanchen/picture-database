CFLAGS += -I/usr/local/opt/openssl/include
CFLAGS += -std=c99
CFLAGS += $$(pkg-config vips --cflags)
LDFLAGS += -L./libmongoose

LDLIBS += $$(pkg-config vips --libs) -lm 
LDLIBS += -lcrypto -lm 
LDLIBS += -ljson-c
LDLIBS += -lmongoose


all: pictDBM pictDB_server

pictDBM: db_list.o pictDBM.o db_utils.o error.o db_create.o db_delete.o image_content.o dedup.o db_insert.o db_read.o pictDBM_tools.o


pictDB_server: pictDB_server.o db_list.o db_utils.o error.o db_create.o db_delete.o image_content.o dedup.o db_insert.o db_read.o pictDBM_tools.o

db_list.o: db_list.c 

error.o: error.c error.h
pictDBM.o: pictDBM.c pictDB.h
db_utils.o: db_utils.c
db_create.o : db_create.c
db_delete.o: db_delete.c
image_content.o: image_content.c image_content.h
dedup.o: dedup.c dedup.h
db_insert.o: db_insert.c
db_read.o: db_read.c
pictDBM_tools.o: pictDBM_tools.c pictDBM_tools.h
pictDB_server.o: pictDB_server.c


clean: 
	rm -rf *.o

mrproper: clean
	rm -rf pictDBM

