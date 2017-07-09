#!/bin/sh

OUT_DIRECTORY="out"

if [ ! -d "$OUT_DIRECTORY" ]; then
    mkdir $OUT_DIRECTORY
fi

# tea algos
gcc ./tea_cversion/tea-decrypt.c -o ./$OUT_DIRECTORY/decrypt.elf
gcc ./tea_cversion/tea-encrypt.c -o ./$OUT_DIRECTORY/encrypt.elf

# client / server
g++ ./server/pasticciotto_server.cpp ../vm/vm.cpp ../vm/vmas.cpp -o ./$OUT_DIRECTORY/server.elf
g++ ./client/pasticciotto_client.cpp ../vm/vm.cpp ../vm/vmas.cpp -o ./$OUT_DIRECTORY/client.elf

# debug versions
g++ ./server/pasticciotto_server.cpp ../vm/vm.cpp ../vm/vmas.cpp -DDBG -o ./$OUT_DIRECTORY/server-debug.elf
g++ ./client/pasticciotto_client.cpp ../vm/vm.cpp ../vm/vmas.cpp -DDBG -o ./$OUT_DIRECTORY/client-debug.elf

# stripping symbols
strip -s $OUT_DIRECTORY/*
