# PoliCTF 2017 Challenge

This is the description displayed on the challenge page:

> We found this executable and we think it must have something in common with the baddies' infrastructure.
> We would be glad to understand what **`data`** they are hiding from us...

# How to deploy

The challenge can be deployed by running the `create_files.sh` helper script and by running the `server.elf` behind a proxy (xinetd). The server needs to access the files in the `res/` directory.

# Write-up

The client provided to the partecipants is just the VM running the code `encrypt.pstc` in the `asms/` directory:

```asm
def datastrlen:
###############
# r0 = offset of str in data
# retval (r0) = strlen
###############
push r1
push r2
push r3
movr s2, r0
movi s1, 0
lodr s0, s2
cmpb s0, 0
jpei exit
loop:
movi s2, 0
addi s1, 1
addr s2, s1
lodr s0, s2
cmpb s0, 0
jpni loop
exit:
movr r0, s1
poop r3
poop r2
poop r1
retn

def round: # round(uint16_t text[2])
#################
# r0 = offset of text[0] in data
# r1 = offset of text[1] in data
# r2 = text[0]
# r3 = text[1]
# retval = void
################
push r1
push r2
push r3
lodr r2, r0 # text[0]
lodr r3, r1 # text[1]
movi s0, 0 # i
movi s1, 0 # sum
loop:
push s0 # saving i
addi s1, 0x626f # sum += delta
push s1 # saving sum
# s0 and s1 will be used as tmps
#########
# calc v0
#########
movr s0, r3
shli s0, 4
addi s0, 0x7065 # s0 = (text[1] << 4) + k0
movr s1, r3
poop s3 # restoring sum in s3
#addr s1, s3 # s1 = text[1] + sum
push s3 # saving sum again
xorr s0, s1 # s0 = ((text[1] << 4) + k0) ^ (text[1] + sum)
push s0
movr s0, r3
shri s0, 5
addi s0, 0x7065 # s0 = (text[1] >> 5) + k1
poop s1
xorr s0, s1 # s0 = ((text[1] << 4) + k0) ^ (text[1] + sum) ^ ((text[1] >> 5) + k1)
addr r2, s0 # r2 += s0
#########
# calc v1
#########
movr s0, r2
shli s0, 4
addi s0, 0x7275 # s0 = (text[0] << 4) + k2
movr s1, r2
poop s3 # restoring sum in s3
#addr s1, s3 # s1 = text[0] + sum
push s3 # saving sum again
xorr s0, s1 # s0 = ((text[0] << 4) + k2) ^ (text[0] + sum)
push s0
movr s0, r2
shri s0, 5
addi s0, 0x6e73 # s0 = (text[0] >> 5) + k3
poop s1
xorr s0, s1 # s0 = ((text[0] << 4) + k2) ^ (text[0] + sum) ^ ((text[0] >> 5) + k3)
addr r3, s0 # r3 += s0
######
# end loop
#####
poop s1 # restoring sum
poop s0 # restoring i
addi s0, 1
cmpb s0, 127 # while (i < 128)
jpbi loop
# saving the values
strr r0, r2
strr r1, r3
poop r3
poop r2
poop r1
retn

def main:
grmn
movi r0, 0xadde
movi r1, 0x0bb0
stri 0, r0
stri 2, r1
movi r0, 0x0bb0
movi r1, 0xcefa
stri 0x4, r0
stri 0x6, r1
movi r0, 0
call datastrlen
movr r2, r0
movi s0, 0
encrypt:
push s0
movi r0, 0
movi r1, 2
addr r0, s0
addr r1, s0
call round
poop s0
addi s0, 4
cmpr s0, r2
jpbi encrypt
lodi r0, 0
lodi r1, 2
lodi r2, 4
lodi r3, 6
shit
```

Once the VM bytecode has been reversed, the partecipant had to write a custom assembler to **decrypt the server's VM data section** using the same algorithm shown above. 

An example of a working decryption algorithm is `decrypt.pstc` in the `asms/` directory:

```asm
def datastrlen:
###############
# r0 = offset of str in data
# retval (r0) = strlen
###############
push r1
push r2
push r3
movr s2, r0
movi s1, 0
lodr s0, s2
cmpb s0, 0
jpei exit
loop:
movi s2, 0
addi s1, 1
addr s2, s1
lodr s0, s2
cmpb s0, 0
jpni loop
exit:
movr r0, s1
poop r3
poop r2
poop r1
retn

def round: # round(uint16_t text[2])
#################
# r0 = offset of text[0] in data
# r1 = offset of text[1] in data
# r2 = text[0]
# r3 = text[1]
# retval = void
################
push r1
push r2
push r3
lodr r2, r0 # text[0]
lodr r3, r1 # text[1]
movi s0, 0 # i
movi s1, 0 # sum
loop:
push s0 # saving i
# s0 and s1 will be used as tmps
#########
# calc v1
#########
movr s0, r2
shli s0, 4
addi s0, 0x7275 # s0 = (text[0] << 4) + k2
movr s1, r2
xorr s0, s1 # s0 = ((text[0] << 4) + k2) ^ text[0]
push s0
movr s0, r2
shri s0, 5
addi s0, 0x6e73 # s0 = (text[0] >> 5) + k3
poop s1
xorr s0, s1 # s0 = ((text[0] << 4) + k2) ^ text[0] ^ ((text[0] >> 5) + k3)
subr r3, s0 # r3 -= s0
#########
# calc v0
#########
movr s0, r3
shli s0, 4
addi s0, 0x7065 # s0 = (text[1] << 4) + k0
movr s1, r3
xorr s0, s1 # s0 = ((text[1] << 4) + k0) ^ text[1]
push s0
movr s0, r3
shri s0, 5
addi s0, 0x7065 # s0 = (text[1] >> 5) + k1
poop s1
xorr s0, s1 # s0 = ((text[1] << 4) + k0) ^ text[1] ^ ((text[1] >> 5) + k1)
subr r2, s0 # r2 -= s0
######
# end loop
#####
poop s0 # restoring i
addi s0, 1
cmpb s0, 127 # while (i < 128)
jpbi loop
# saving the values
strr r0, r2
strr r1, r3
poop r3
poop r2
poop r1
retn

def main:
movi r0, 0
call datastrlen
movr r2, r0
movi s0, 0
decrypt:
push s0
movi r0, 0
movi r1, 2
addr r0, s0
addr r1, s0
call round
poop s0
addi s0, 4
cmpr s0, r2
jpbi decrypt
shit
```

The python wrapper used to solve the challenge, `exploit-test.py`, can be found in the `server/` directory.

```python
from pwn import *
import subprocess

key_re = re.compile(".*\"(.*)\".*")
r = remote("pasticciotto.chall.polictf.it", 31337)

first = r.recv()
key = key_re.match(first).group(1)
print("Using key: {}".format(key))
subprocess.check_call(["python3", "../../assembler/assembler.py", "{}".format(key), "../asms/decrypt.pstc", "./out.pasticciotto"])
with open("./out.pasticciotto") as f:
    data = f.read()
r.send("{}\n".format(len(data)))
print(r.recv())
r.send("{}\n".format(data))
print(r.recv(100000))
```

## Challenge output

```
$ python ./exploit-test.py
[+] Opening connection to pasticciotto.chall.polictf.it on port 31337: Done
Using key: 9XM6SvFPvN8qiLi
movi : 0x0->0x39
movr : 0x1->0x20
lodi : 0x2->0x9a
lodr : 0x3->0x1d
stri : 0x4->0xa9
strr : 0x5->0xd2
addi : 0x6->0x38
addr : 0x7->0x8f
subi : 0x8->0xd
subr : 0x9->0x64
andb : 0xa->0xa6
andw : 0xb->0x22
andr : 0xc->0x97
yorb : 0xd->0xda
yorw : 0xe->0x51
yorr : 0xf->0x48
xorb : 0x10->0x12
xorw : 0x11->0x70
xorr : 0x12->0xb6
notr : 0x13->0x37
muli : 0x14->0xa5
mulr : 0x15->0xc
divi : 0x16->0xd5
divr : 0x17->0xf4
shli : 0x18->0xdc
shlr : 0x19->0xc3
shri : 0x1a->0x6e
shrr : 0x1b->0xb5
push : 0x1c->0xe1
poop : 0x1d->0x88
cmpb : 0x1e->0x2c
cmpw : 0x1f->0x3a
cmpr : 0x20->0x35
jmpi : 0x21->0xbb
jmpr : 0x22->0x52
jpai : 0x23->0x4f
jpar : 0x24->0x90
jpbi : 0x25->0xd4
jpbr : 0x26->0x11
jpei : 0x27->0xe5
jper : 0x28->0x45
jpni : 0x29->0xbc
jpnr : 0x2a->0xbe
call : 0x2b->0x32
retn : 0x2c->0x7e
shit : 0x2d->0x6d
nope : 0x2e->0x6a
grmn : 0x2f->0xe6
FUNCTION main
0x0:	movi r0, 0
0x4:	call datastrlen
0x7:	movr r2, r0
0x9:	movi s0, 0
0xd:	push s0
0xf:	movi r0, 0
0x13:	movi r1, 2
0x17:	addr r0, s0
0x19:	addr r1, s0
0x1b:	call round
0x1e:	poop s0
0x20:	addi s0, 4
0x24:	cmpr s0, r2
0x26:	jpbi decrypt
0x29:	shit 
FUNCTION round
0x2a:	push r1
0x2c:	push r2
0x2e:	push r3
0x30:	lodr r2, r0
0x32:	lodr r3, r1
0x34:	movi s0, 0
0x38:	movi s1, 0
0x3c:	push s0
0x3e:	movr s0, r2
0x40:	shli s0, 4
0x44:	addi s0, 0x7275
0x48:	movr s1, r2
0x4a:	xorr s0, s1
0x4c:	push s0
0x4e:	movr s0, r2
0x50:	shri s0, 5
0x54:	addi s0, 0x6e73
0x58:	poop s1
0x5a:	xorr s0, s1
0x5c:	subr r3, s0
0x5e:	movr s0, r3
0x60:	shli s0, 4
0x64:	addi s0, 0x7065
0x68:	movr s1, r3
0x6a:	xorr s0, s1
0x6c:	push s0
0x6e:	movr s0, r3
0x70:	shri s0, 5
0x74:	addi s0, 0x7065
0x78:	poop s1
0x7a:	xorr s0, s1
0x7c:	subr r2, s0
0x7e:	poop s0
0x80:	addi s0, 1
0x84:	cmpb s0, 127
0x87:	jpbi loop
0x8a:	strr r0, r2
0x8c:	strr r1, r3
0x8e:	poop r3
0x90:	poop r2
0x92:	poop r1
0x94:	retn 
FUNCTION datastrlen
0x95:	push r1
0x97:	push r2
0x99:	push r3
0x9b:	movr s2, r0
0x9d:	movi s1, 0
0xa1:	lodr s0, s2
0xa3:	cmpb s0, 0
0xa6:	jpei exit
0xa9:	movi s2, 0
0xad:	addi s1, 1
0xb1:	addr s2, s1
0xb3:	lodr s0, s2
0xb5:	cmpb s0, 0
0xb8:	jpni loop
0xbb:	movr r0, s1
0xbd:	poop r3
0xbf:	poop r2
0xc1:	poop r1
0xc3:	retn 
[main: size 0x2a, offset 0x0, round: size 0x6b, offset 0x2a, datastrlen: size 0x2f, offset 0x95]
Go ahead then!

Congratulations!
The flag is: flag{m4nc14t1b1_stu_bellu_p4sticci0tt0}

[*] Closed connection to pasticciotto.chall.polictf.it port 31337
```