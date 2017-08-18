vm-objects = vm.o vmas.o
pctf-objects = pasticciotto_server.o pasticciotto_client.o
CXXFLAGS = -Wall

all: emulator polictf
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

.PHONY: clean
clean:
	rm pasticciotto*.elf
	rm $(pctf-objects) $(vm-objects)