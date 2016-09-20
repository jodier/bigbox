all:
	#####################################################################
	# COMMON                                                            #
	#####################################################################

	gcc -O3 -Wall -c -o src/common/client.o src/common/client.c
	gcc -O3 -Wall -c -o src/common/hash.o src/common/hash.c
	gcc -O3 -Wall -c -o src/common/hash_table.o src/common/hash_table.c
	gcc -O3 -Wall -c -o src/common/log.o src/common/log.c
	gcc -O3 -Wall -c -o src/common/rio.o src/common/rio.c
	gcc -O3 -Wall -c -o src/common/server.o src/common/server.c
	gcc -O3 -Wall -c -o src/common/server_pooler.o src/common/server_pooler.c

	mkdir -p lib

	ar rcs lib/libbigbox-common.a src/common/*.o

	#####################################################################
	# SERVER                                                            #
	#####################################################################

	gcc -O3 -Wall -c -o src/server/daemon.o src/server/daemon.c

	gcc -L lib -o bigbox-server src/server/*.o -lbigbox-common

	#####################################################################
	# CLIENT                                                            #
	#####################################################################

	gcc -O3 -Wall -c -o src/client/cli.o src/client/cli.c

	gcc -L lib -o bigbox-cli src/client/*.o -lbigbox-common

	#####################################################################

clean:
	rm -fr src/common/*.o src/server/*.o src/client/*.o bigbox-server bigbox-cli
