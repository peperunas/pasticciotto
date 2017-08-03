# Architecture

Pasticciotto uses the Harvard Architecture meaning its code is separated from its data and also from its stack. This allowed me to materialize my idea for the **PoliCTF** challenge: I could run the code the partecipants assembled without any hassle!
![Structure]

There are 8 general purpose registers (`R0` to `S3`) with `S0 -> S3` being "scratch" ones. There is a `RP` register (Return Pointer), the `SP` register (Stack Pointer) and obviously the `IP` (Instruction Pointer).

Every instruction varies from `2` to `4` bytes long: the opcode has a fixed size (`1` byte).

# Opcode encryption

The VM needs a decryption key to run: the opcodes are "encrypted" with the key by the assembler. The encryption algorithm is the `RC4` key scheduling shuffle. Once the values are shuffled, the `opcodes` are assigned according to their definition order.

```python
key_ba = bytearray(key, 'utf-8')
# RC4 KSA! :-P
arr = [i for i in range(256)]
j = 0
for i in range(len(arr)):
    j = (j + arr[i] + key_ba[i % len(key)]) % len(arr)
    arr[i], arr[j] = arr[j], arr[i]

for i, o in enumerate(ops):
    o.set_value(arr[i])
```

# Addressing modes
## Absolute
```
JMPI 0x200 # jumps to code[0x200]
CALL foo   # jumps to the foo() function
```
## Register direct
Every opcode ending with `R` uses this addressing mode. E.g:
```
MOVR R0, R1 # R0 = R1
MULR R0, R1 # R0 *= R1
```

## Immediate
Every opcode ending with `I` uses this addressing mode. E.g:
```
MOVI R0, 0x2 # R0 = 0x2
MULI R0, 0x2 # R0 *= 0x2
```
# Assembling, labels and functions

The enclosed assembler recognizes **labels** and **functions**. The **main** function has to be defined. Here is an example:

```
def foo:
addi r0, 0x3
movi r1, 0x1
retn

def main: # main is mandatory
movi r0, 0xff
nope
jmpi label # jumping to label
nope
addi r0, 0x2
label: # defining a label
grmn
call foo
shit
```
In order to jump to a label or a function, an *immediate type* jump has to be used (`JMPI, JPBI, JPAI`, etc...). The `CALL` instruction is used to save where the program has to restore its execution after a function call.

The assembler puts the **main** function as first in the code section meaning its code will be located at offset 0. Every other function will follow.

![Functions]

# Instruction set
The instruction set I come out wants to be "RISC"-oriented but I have to admit that it is more "CISC"-oriented *(Confusing Instruction Set Computer)*.
Also, since I decided that every instruction had to be 4 chars long, some name adaptation may have encountered some quality issue... (yes, `POP`, I'm looking at you)

**The syntax used is the Intel one!**

There **three types** of instructions:
1. with 2 operands (*imm2reg*, *reg2imm*, *byt2reg*, *reg2reg*)
2. with 1 operand
3. with no operand at all (*single*)

![Instruction]
## MOVI
```
Full name: MOVe Immediate to register
Usage: MOVI R0, 0x00
Effect: R0 contains the value 0x00
```
## MOVR
```
Full name: MOVe Register to register
Usage: MOVR R1, R0
Effect: R0 is copied into R1
```
## LODI
```
Full name: LOaD Immediate offset @ data section to register
Usage: LODI R0, 0x0
Effect: R0 contains data[0x0]
```
## LODR
```
Full name: LOaD offset in Register @ data section to register
Usage: LODR R1, R0
Effect: R1 contains data[R1]
```
## STRI
```
Full name: SToRe @ immediate offset in data section from register
Usage: STRI 0x0, R0
Effect: data[0x0] contains R0
```
## STRR
```
Full name: SToRe @ offset of Register in data section from register
Usage: STRR R1, R0
Effect: data[R1] contains R0
```
## ADDI
```
Full name: ADD Immediate to register
Usage: ADDI R0, 0x1
Effect: R0 is incremented by 0x1
```
## ADDR
```
Full name: ADD Register to register
Usage: ADDR R1, R0
Effect: R1 is incremented by R0
```
## SUBI
```
Full name: SUBstract Immediate from register
Usage: SUBI R0, 0x1
Effect: R0 is decremented by 0x1
```
## SUBR
```
Full name: SUBstract Register from register
Usage: SUBR R1, R0
Effect: R1 is decremented by R0
```
## ANDB
```
Full name: AND Byte (immediate)
Usage: ANDB R0, 0xFF
Effect: R0's lower byte is and-ed by 0xFF.
```
## ANDW
```
Full name: AND Word (immediate)
Usage: ANDW R0, 0xFFFF
Effect: R0's is and-ed by 0xFFFF.
```
## ANDR
```
Full name: AND Register
Usage: ANDR R0, R1
Effect: R0 is and-ed by R1.
```
## YORB
```
Full name: (Y)OR Byte (immediate)
Usage: YORB R0, 0xFF
Effect: R0's lower byte is or-ed by 0xFF.
```
## YORW
```
Full name: (Y)OR Word (immediate)
Usage: YORW R0, 0xFFFF
Effect: R0's is or-ed by 0xFFFF.
```
## YORR
```
Full name: (Y)OR Register
Usage: YORR R0, R1
Effect: R0 is or-ed by R1.
```
## XORB
```
Full name: XOR Byte (immediate)
Usage: XORB R0, 0xFF
Effect: R0's lower byte is xor-ed by 0xFF.
```
## XORW
```
Full name: XOR Word (immediate)
Usage: XORW R0, 0xFFFF
Effect: R0 is xor-ed by 0xFFFF.
```
## XORR
```
Full name: XOR Register
Usage: XORR R0, R1
Effect: R0 is xor-ed by R1.
```
## NOTR
```
Full name: NOT Register
Usage: NOTR R0
Effect: Bitwise negation of R0.
```
## MULI
```
Full name: MULtiply by Immediate
Usage: MULI R0, 2
Effect: R0 is multiplied by 2.
```
## MULR
```
Full name: MULtiply by Register
Usage: MULR R0, R1
Effect: R0 is multiplied by R1.
```
## DIVI
```
Full name: DIVide by Immediate
Usage: DIVI R0, 2
Effect: R0 is divided by 2. The remainder is not stored.
```
## DIVR
```
Full name: DIVide by Register 
Usage: DIVR R0, R1
Effect: R0 is divided by R1. The remainder is not stored.
```
## SHLI
```
Full name: SHift Left by Immediate
Usage: SHLI R0, 2
Effect: Effect: R0 is shifted 2 bits to the left.
```
## SHLR
```
Full name: SHift Left by Register
Usage: SHLR R0, R1
Effect: R0 is shifted R1 bits to the left.
```
## SHRI
```
Full name: SHift Right by Immediate
Usage: SHRI R0, 2
Effect: Effect: R0 is shifted 2 bits to the right.
```
## SHRR
```
Full name: SHift Right by Register
Usage: SHRR R0, R1
Effect: R0 is shifted R1 bits to the right.
```
## PUSH
```
Full name: PUSH
Usage: PUSH R1
Effect: Pushes R1 on top of the stack.
```
## POOP
```
Full name: POP (+ 1 free 'O')
Usage: POOP R1
Effect: Retrieves the element on top of the stack and puts it in R1.
```
## CMPB
```
Full name: CoMPare register to Byte
Usage: CMPB R0, 0xff
Effect: Compares R0 to 0xFF (R0's lower byte) and sets the ZF and CF flags.
```
## CMPW
```
Full name: CoMPare register to Word
Usage: CMPW R0, 0xffff
Effect: Compares R0 to 0xFFFF and sets the ZF and CF flags.
```
## CMPR
```
Full name: CoMPare register to Register
Usage: CMPR R0, R1
Effect: Compares R0 to R1 and sets the ZF and CF flags.
```
## JMPI
```
Full name: JuMP to Immediate
Usage: JMPI 0x00
Effect: Unconditional jump to 0x00
```
## JMPR
```
Full name: JuMP to Register
Usage: JMPR R0
Effect: Unconditional jump to R0
```
## JPAI
```
Full name: JumP if Above to Immediate
Usage: JPAI 0x00
Effect: Jumps to code[0x00] according to last comparison
```
## JPAR
```
Full name: JumP if Above to Register
Usage: JPAR R0
Effect: Jumps to code[R0] according to last comparison
```
## JPBI
```
Full name: JumP if Below or equal to Immediate
Usage: JPBI 0x00
Effect: Jumps to code[0x00] according to last comparison
```
## JPBR
```
Full name: JumP if Below or equal to Register
Usage: JPBR R0
Effect: Jumps to code[R0] according to last comparison
```
## JPEI
```
Full name: JumP if Equal to Immediate
Usage: JPEI 0x00
Effect: Jumps to code[0x00] according to last comparison
```
## JPER
```
Full name: JumP if Equal to Register
Usage: JPER R0
Effect: Jumps to code[R0] according to last comparison
```
## JPNI
```
Full name: JumP if Not equal to Immediate
Usage: JPNI 0x00
Effect: Jumps to code[0x00] according to last comparison
```
## JPNR
```
Full name: JumP if Not equal to Register
Usage: JPNR R0
Effect: Jumps to code[R0] according to last comparison
```
## CALL
```
Full name: CALL function
Usage: CALL *function*
Effect: Saves the next instruction address into RP and jumps to the start of the function
```
## RETN
```
Full name: RETurN
Usage: RETN
Effect: Restores the RP into the IP and jumps to the IP 
```
## SHIT
```
Full name: Well...
Usage: SHIT
Effect: Halts the execution
```
## NOPE
```
Full name: NOP(e)
Usage: NOPE
Effect: Does nothing for an instruction
```
## GRMN
```
Full name: GeRMaNo
Usage: GRMN
Effect: Sets every register (excluding IP and RP) to GG (0x4747)
```
## DEBG
```
Full name: DEBuG
Usage: DEBG
Effect: Prints the status of every register and the flags. 

NOTE: The DBG preprocessor flag has to be enabled!
```

[Instruction]: ./res/instruction.png
[Structure]: ./res/structure.png
[Functions]: ./res/functions.png
