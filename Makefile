all:
	#####################################################################
	# LUA                                                               #
	#####################################################################

	cd src/common/lua/src && $(MAKE) generic

	#####################################################################
	# COMMON                                                            #
	#####################################################################

	gcc -std=c99 -O0 -g -Wall -c -o src/common/client.o src/common/client.c
	gcc -std=c99 -O0 -g -Wall -c -o src/common/hash.o src/common/hash.c
	gcc -std=c99 -O0 -g -Wall -c -o src/common/hash_table.o src/common/hash_table.c
	gcc -std=c99 -O0 -g -Wall -c -o src/common/log.o src/common/log.c
	gcc -std=c99 -O0 -g -Wall -c -o src/common/loop/http.o src/common/loop/http.c
	gcc -std=c99 -O0 -g -Wall -c -o src/common/loop/tcp.o src/common/loop/tcp.c
	gcc -std=c99 -O0 -g -Wall -c -o src/common/lua.o src/common/lua.c
	gcc -std=c99 -O0 -g -Wall -c -o src/common/rio.o src/common/rio.c
	gcc -std=c99 -O0 -g -Wall -c -o src/common/server.o src/common/server.c
	gcc -std=c99 -O0 -g -Wall -c -o src/common/server_pooler.o src/common/server_pooler.c

	mkdir -p lib

	ar rcs lib/libbigbox-common.a src/common/*.o src/common/loop/*.o src/common/lua/src/*.o && ranlib lib/libbigbox-common.a

	#####################################################################
	# SERVER                                                            #
	#####################################################################

	python src/server/bin2h.py src/server/html/index.html index_html > src/server/html/index.h

	#####################################################################

	gcc -std=c99 -O0 -g -Wall -c -o src/server/daemon.o src/server/daemon.c

	gcc -std=c99 -L lib -o bigbox-server src/server/*.o -lbigbox-common -lpthread

	#####################################################################
	# CLIENT                                                            #
	#####################################################################

	gcc -std=c99 -O0 -g -Wall -c -o src/client/cli.o src/client/cli.c

	gcc -std=c99 -L lib -o bigbox-cli src/client/*.o -lbigbox-common

	#####################################################################

clean:
	cd src/common/lua/src && $(MAKE) clean

	rm -fr src/common/*.o src/server/*.o src/client/*.o lib/*.a bigbox-server bigbox-cli
