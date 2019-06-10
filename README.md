# Pasticciotto

![TravisCI] [![Say Thanks!](https://img.shields.io/badge/Say%20Thanks-!-1EAEDB.svg)](https://saythanks.io/to/peperunas)
------------------
![Pasticciotto]

# What is this?
Pasticciotto is a virtual machine which can be used to obfuscate code. It was developed for the **PoliCTF 17** as a reversing challenge.

The key feature is its **opcode "shuffling"**: their actual values are determined by a password. (More in [`IMPLEMENTATION.md`](./IMPLEMENTATION.md))

I wanted to experiment with VM obfuscation since it was a topic that caught my attention while reversing challenges for various CTFs. So, I decided to write one **from scratch** in order to understand better how instruction set architectures are implemented! 

The design and the implementation behind Pasticciotto are not state-of-the-art but hey, it works! :D

# Why "Pasticciotto"?
In Italian, "Pasticciotto" has two meanings! 

The first one is **"little mess"** which perfectly describes how I put up this project. The second one is a typical dessert from Southern Italy, Salento! It's filled with cream! Yum!

# Quick start

You can use `pasticciotto` in your own binary! It's easy to do!

## Assemble!
Let's say you want to run this `C` code into `pasticciotto`:
```c
void main() {
    uint16_t i, a, b;
    a = 0;
    b = 0x10;

    for (i = 0; i < b; i++) {
        a += b;
    }
    return;
}
```

It can be translated into this `pasticciotto`'s assembly snippet:
```
$ cat example.pstc
def main:
movi r0, 0x0  # a
movi r1, 0x10 # b
movi s1, 0x0  # i
loop:
addr r0, r1
addi s1, 1
cmpr s1, r1
jpbi loop
shit
```
Let's assemble it with key `HelloWorld`:
```
$ python3 assembler.py HelloWorld example.pstc example_assembled.pstc
```

Now we are ready to embed the VM in a `.c` program:
```c++
#include "vm/vm.h"
#include <fstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    /*
    In order to create the bytecode for pasticciotto, you can use
    the assembler in the assembler/ directory. You can include it with
    `xxd -i example_assembled.pstc`
    */
    unsigned char example_assembled_pstc[] = {
    0x32, 0x00, 0x00, 0x00, 0x32, 0x01, 0x10, 0x00, 0x32, 0x05, 0x00, 0x00,
    0xaf, 0x01, 0xcf, 0x05, 0x01, 0x00, 0x8b, 0x51, 0xc5, 0x0c, 0x00, 0x0c
    };
    unsigned int example_assembled_pstc_len = 24;
    unsigned char key[] = {
    0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x0a
    };


    puts("I should try to eat a pasticciotto...\n");
    VM vm(key, example_assembled_pstc, example_assembled_pstc_len);
    vm.run();
    return 0;
}
```
That's it!

## Accessing to the VM's sections and registers

The VM **data / code / stack sections** are represented through the `VMAddrSpace` object. It is defined [here](vm/vmas.h). The **registers** are in a `uint16_t` array in the `VM` object defined [here](vm/vm.h).

```c++
void foo() {
    // creating the VM with some code
    VM vm(key, code, codelen);

    // accessing the data section
    printf("First data byte: 0x%x", VM.addrSpace()->getData()[0]);
    // accessing the code section
    printf("First code byte: 0x%x", VM.addrSpace()->getCode()[0]);    
    // accessing the stack section
    printf("First stack byte: 0x%x", VM.addrSpace()->getStack()[0]);
    // accessing the IP register
    printf("The IP is: 0x%x", VM.regs(IP));
    return;
}
```


# What about the challenge?
You can find the client and the server under the `polictf/` directory. I have also written a small writeup. Check it out!

# Compiling

## Requisites

1. `CMake`

## Quick start

    mkdir build
    cmake ..
    # or, if you want debug info:
    # cmake -DPASTICCIOTTO_DEBUG=On ..
    make

## CMake targets

| Target name             | Description                            |
| ----------------------- | -------------------------------------- |
| `pasticciotto-emulator` | Builds pasticciotto's emulator         |
| `polictf`               | Builds PoliCTF's client and server     |
| `polictf-client`        | Builds PoliCTF's client                |
| `polictf-server`        | Builds PoliCTF's server                |
| `pasticciotto-tests`    | Builds pasticciotto's test executable. |

If the `PASTICCIOTTO_DEBUG` flag is passed to `cmake` during the configuration phase, the targets will be compiled with debug symbols and additional debug information.


# Implementation details
Check out the file [IMPLEMENTATION.MD](./IMPLEMENTATION.md) to understand how the VM works and which operations it can do! Watch out for some spoilers if you haven't completed the challenge though!

# Contributions

I wanted to polish the VM even more but I haven't got the time to do it. There are rough edges for sure!

Any contribution is **very** welcome! Feel free to open issues and pull requests!

# License
```
Copyright 2017 Giulio De Pasquale

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software 
without restriction, including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons 
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or 
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.
```
[Pasticciotto]: ./res/pasticciotto.png
[TravisCI]: https://travis-ci.org/peperunas/pasticciotto.svg?branch=master
