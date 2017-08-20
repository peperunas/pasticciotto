vm-objects = vm.o vmas.o
pctf-objects = pasticciotto_server.o pasticciotto_client.o
test_files = tests/test_main.cpp tests/vm/test_vm.cpp tests/vmas/test_vmas.cpp
CXXFLAGS = -Wall

all: emulator polictf test
emulator: emulator/emulator.cpp $(vm-objects)
	$(CXX) $(CXXFLAGS) -o pasticciotto-emu.elf emulator/emulator.cpp $(vm-objects)
polictf: $(vm-objects) $(pctf-objects)
	$(CXX) $(CXXFLAGS) -o pasticciotto-client.elf pasticciotto_client.o $(vm-objects)
	$(CXX) $(CXXFLAGS) -o pasticciotto-server.elf pasticciotto_server.o $(vm-objects)
debug: CXXFLAGS += -DDBG -g
debug: all
vm.o: vm/vm.cpp vm/vm.h
	$(CXX) $(CXXFLAGS) -c vm/vm.cpp
vmas.o: vm/vmas.cpp vm/vmas.h
	$(CXX) $(CXXFLAGS) -c vm/vmas.cpp
pasticciotto_server.o: polictf/server/pasticciotto_server.cpp
	$(CXX) $(CXXFLAGS) -c polictf/server/pasticciotto_server.cpp
pasticciotto_client.o: polictf/client/pasticciotto_client.cpp
	$(CXX) $(CXXFLAGS) -c polictf/client/pasticciotto_client.cpp
test: $(test_files) $(vm-objects)
	$(CXX) $(CXXFLAGS) -o pasticciotto-tests.elf $(test_files) $(vm-objects)
	@./pasticciotto-tests.elf

.PHONY: clean
clean:
	rm pasticciotto*.elf
	rm $(pctf-objects) $(vm-objects)