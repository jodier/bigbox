all:
	#####################################################################
	# COMMON                                                            #
	#####################################################################

	gcc -O3 -c -o src/common/log.o src/common/log.c

	gcc -O3 -c -o src/common/client.o src/common/client.c
	gcc -O3 -c -o src/common/server.o src/common/server.c

	gcc -O3 -c -o src/common/rio.o src/common/rio.c

	#####################################################################
	# SERVER                                                            #
	#####################################################################

	gcc -O3 -c -o src/server/daemon.o src/server/daemon.c
	gcc -o bigbox-server src/server/daemon.o

	#####################################################################
	# CLIENT                                                            #
	#####################################################################

	gcc -O3 -c -o src/client/cli.o src/client/cli.c
	gcc -o bigbox-cli src/server/daemon.o

	#####################################################################
