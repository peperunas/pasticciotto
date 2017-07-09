import sys
import re
import struct
import copy
import argparse


class AssemblerException(Exception):
    pass


class InvalidRegister(AssemblerException):

    def __init__(self, register):
        super().__init__("Invalid register: {}".format(register))


class InvalidOperation(AssemblerException):

    def __init__(self, operation):
        super().__init__("Invalid operation: {}".format(operation))


class ExpectedImmediate(AssemblerException):

    def __init__(self, value):
        super().__init__("Expected immediate, got {}".format(value))


class ExpectedRegister(AssemblerException):

    def __init__(self, value):
        super().__init__("Expected register, got {}".format(value))


class IPOverwrite(AssemblerException):

    def __init__(self, instruction=None):
        if instruction:
            super().__init__("IP can't be overwritten. Instruction: {}".format(instruction))
        else:
            super().__init__("IP can't be overwritten.")


class InvalidValue(AssemblerException):

    def __init__(self, instruction):
        super().__init__("Invalid value while assembling: {}".format(instruction))


class VMAssembler:

    def __init__(self, key, data):
        self.data = data
        self.assembled_code = bytearray()
        self.functions = []
        self.encrypt_ops(key)
        self.parse_functions()
        self.resolve_functions_offsets()
        self.resolve_symbols()
        self.disassembly()
        main = next((x for x in self.functions if x.name == "main"), None)
        if main == None:
            print("Main has to be defined")
            return

    def parse_functions(self):
        cur_fun_size = 0
        cur_fun_name = None
        fun_start = 0

        # first parse to get every function name
        for i, line in enumerate(self.data):
            match = function_re.match(line)
            if match:
                if cur_fun_name:
                    f = VMFunction(cur_fun_name, self.data[fun_start:i])
                    self.functions.append(f)
                cur_fun_name = match.group(1)
                fun_start = i + 1
        f = VMFunction(cur_fun_name, self.data[fun_start:i + 1])
        self.functions.append(f)

        # putting main in first position in order to assemble it first
        for i, f in enumerate(self.functions):
            if f.name == "main" and i is not 0:
                self.functions[0], self.functions[
                    i] = self.functions[i], self.functions[0]
                break

    def resolve_functions_offsets(self):
        # calculating functions offsets
        for i in range(1, len(self.functions)):
            prev_fun_tot_size = self.functions[
                i - 1].size() + self.functions[i - 1].offset
            self.functions[i].set_offset(prev_fun_tot_size)
        return

    def resolve_symbols(self):
        for f in self.functions:
            for i in f.instructions:
                symcall = symcall_re.match(str(i))
                if symcall:
                    symname = symcall.group(1)
                    # checking if it's a jump to a label
                    if symname in [y.label for x in self.functions for y in x.instructions]:
                        fun = next(
                            (x for x in self.functions for y in x.instructions if symname == y.label), None)
                        offset = f.offset_of_label(symname) + f.offset
                    # function
                    elif symname in [x.name for x in self.functions]:
                        offset = next(
                            (x.offset for x in self.functions if x.name == symname), None)
                    else:
                        print("SYMBOL \"{}\" NOT FOUND!".format(symname))
                    i.args[0].set_value(offset)

    def parse(self):
        for f in self.functions:
            for i in f.instructions:
                action = getattr(self, "{}".format(i.opcode.method))
                action(i)

    def disassembly(self):
        for f in self.functions:
            print("FUNCTION {}".format(f.name))
            for idx, ins in enumerate(f.instructions):
                print("{}:\t{}".format(
                    hex(f.offset + f.offset_of_instruction(idx)), ins))

    def imm2reg(self, instruction):
        """
        Intel syntax -> REG, IMM
        """
        opcode = instruction.opcode
        reg = instruction.args[0]
        imm = instruction.args[1]
        if reg.name == "ip":
            raise IPOverwrite(instruction)
        if not imm.isimm():
            raise ExpectedImmediate(imm)
        if not reg.isreg():
            raise ExpectedRegister(reg)
        if not opcode.uint8() or not reg.uint8() or not imm.uint16():
            raise InvalidValue(instruction)
        self.assembled_code += opcode.uint8() + reg.uint8() + imm.uint16()
        return

    def reg2reg(self, instruction):
        """
        Intel syntax -> DST_REG, SRC_REG
        """
        opcode = instruction.opcode
        dst_reg = instruction.args[0]
        src_reg = instruction.args[1]
        if dst_reg.name == "ip" or src_reg.name == "ip":
            raise IPOverwrite(instruction)
        if not dst_reg.isreg():
            raise ExpectedRegister(dst_reg)
        if not src_reg.isreg():
            raise ExpectedRegister(src_reg)
        if not opcode.uint8() or not dst_reg.uint8() or not src_reg.uint8():
            raise InvalidValue(instruction)
        byte_with_nibbles = struct.pack("<B", dst_reg.uint8()[0] << 4 ^ (
            src_reg.uint8()[0] & 0b00001111))
        self.assembled_code += opcode.uint8() + byte_with_nibbles
        return

    def reg2imm(self, instruction):
        """
        Intel syntax -> IMM, REG
        """
        opcode = instruction.opcode
        imm = instruction.args[0]
        reg = instruction.args[1]
        if reg.name == "ip":
            raise IPOverwrite(instruction)
        if not imm.isimm():
            raise ExpectedImmediate(imm)
        if not reg.isreg():
            raise ExpectedRegister(reg)
        if not opcode.uint8() or not reg.uint8() or not imm.uint16():
            raise InvalidValue(instruction)
        self.assembled_code += opcode.uint8() + imm.uint16() + reg.uint8()
        return

    def byt2reg(self, instruction):
        """
        Intel syntax -> REG, [BYTE]IMM
        """
        opcode = instruction.opcode
        reg = instruction.args[0]
        imm = instruction.args[1]
        if reg.name == "ip":
            raise IPOverwrite(instruction)
        if not imm.isimm():
            raise ExpectedImmediate(imm)
        if not reg.isreg():
            raise ExpectedRegister(reg)
        if not opcode.uint8() or not reg.uint8() or not imm.uint8():
            raise InvalidValue(instruction)
        self.assembled_code += opcode.uint8() + reg.uint8() + imm.uint8()
        return

    def regonly(self, instruction):
        """
        Instruction with only an argument: a register
        """
        opcode = instruction.opcode
        reg = instruction.args[0]
        if reg.name == "ip":
            raise IPOverwrite(instruction)
        if not reg.isreg():
            raise ExpectedRegister(reg)
        if not opcode.uint8() or not reg.uint8():
            raise InvalidValue(instruction)
        self.assembled_code += opcode.uint8() + reg.uint8()
        return

    def immonly(self, instruction):
        """
        Instruction with only an argument: an immediate
        """
        opcode = instruction.opcode
        imm = instruction.args[0]
        if not imm.isimm():
            raise ExpectedImmediate(imm)
        if not opcode.uint8() or not imm.uint16():
            raise InvalidValue(instruction)
        self.assembled_code += opcode.uint8() + imm.uint16()
        return

    def jump(self, instruction):
        imm_op_re = re.compile("^([cC][aA][lL]{2})$|(.*[iI])$")
        reg_op_re = re.compile(".*[rR]$")
        symcall = symcall_re.match(str(instruction))
        dst = instruction.args[0]
        # define the kind of jump: to immediate or to register
        if imm_op_re.match(instruction.opcode.name):
            self.immonly(instruction)
        elif reg_op_re.match(instruction.opcode.name):
            self.regonly(instruction)
        else:
            raise AssemblerException()

    def single(self, instruction):
        """
        Instruction with no arguments
        """
        opcode = instruction.opcode
        self.assembled_code += opcode.uint8()
        return

    def encrypt_ops(self, key):
        key_ba = bytearray(key, 'utf-8')
        olds = copy.deepcopy(ops)

        # RC4 KSA! :-P
        arr = [i for i in range(256)]
        j = 0
        for i in range(len(arr)):
            j = (j + arr[i] + key_ba[i % len(key)]) % len(arr)
            arr[i], arr[j] = arr[j], arr[i]

        for i, o in enumerate(ops):
            o.set_value(arr[i])

        for o, n in zip(olds, ops):
            print("{} : {}->{}".format(o.name, hex(o.value), hex(n.value)))


class VMFunction:

    def __init__(self, name, code):
        self.name = name
        self.offset = 0
        self.instructions = []

        # populating instructions
        i = 0
        while i < len(code):
            line = code[i]
            ins = instruction_re.match(line)
            label = label_re.match(line)
            if label:
                label_name = label.group(1)
                self.instructions.append(
                    VMInstruction(code[i + 1], label_name))
                i += 2
            elif ins:
                self.instructions.append(VMInstruction(line))
                i += 1
            else:
                raise InvalidOperation(line)

    def size(self):
        size = 0
        for i in self.instructions:
            size += i.size
        return size

    def set_offset(self, offset):
        self.offset = offset

    def offset_of_label(self, label):
        offset = 0
        for i in self.instructions:
            if str(i.label) == str(label):
                break
            offset += i.size

        return offset

    def offset_of_instruction(self, idx):
        offset = 0
        for i, ins in enumerate(self.instructions):
            if i == idx:
                break
            offset += ins.size
        return offset

    def __repr__(self):
        return "{}: size {}, offset {}".format(self.name, hex(self.size()), hex(self.offset))


class VMInstruction:
    """
    Represents an instruction the VM recognizes.
    e.g: MOVI [R0, 2]
          ^       ^
        opcode  args
    """

    def __init__(self, line, label=None):
        self.opcode = None
        self.args = []
        self.size = 0
        self.label = label

        ins = instruction_re.match(line)
        symcall = symcall_re.match(line)

        opcode = ins.group(1)
        self.opcode = next((x for x in ops if x.name == opcode), None)
        if self.opcode == None:
            raise InvalidOperation(opcode)
        self.size = ops_sizes[self.opcode.method]
        args = [x for x in ins.groups()[1:] if x is not None]
        for a in args:
            if immediate_re.match(a) or symcall:
                # directly append the immediate
                self.args.append(VMComponent(a, a))
                continue
            elif register_re.match(a):
                # create a VM component for a register
                reg = next((x for x in regs if x.name == a), None)
                if reg == None:
                    raise InvalidRegister(a)
                self.args.append(reg)
                continue

    def __repr__(self):
        return "{} {}".format(self.opcode.name, ", ".join([x.name for x in self.args]))


class VMComponent:
    """
    Represents a register, operation or an immediate the VM recognizes
    """

    def __init__(self, name, value, method=None):
        self.name = name.casefold()
        self.value = value
        self.method = method

    def __repr__(self):
        return "{}".format(self.name)

    def set_name(self, name):
        self.name = name

    def set_value(self, value):
        self.value = value

    def uint8(self):
        numre = re.compile("^[0-9]+$")
        if isinstance(self.value, int):
            return struct.pack("<B", self.value)
        elif self.value.startswith("0x"):
            return struct.pack("<B", int(self.value, 16))
        elif numre.match(self.value):  # only numbers
            return struct.pack("<B", int(self.value))
        return None

    def uint16(self):
        numre = re.compile("^[0-9]+$")
        if isinstance(self.value, int):
            return struct.pack("<H", self.value)
        elif self.value.startswith("0x"):
            return struct.pack("<H", int(self.value, 16))
        elif numre.match(self.value):  # only numbers
            return struct.pack("<H", int(self.value))
        return None

    def isreg(self):
        if self.name not in [x.casefold() for x in reg_names]:
            return False
        return True

    def isop(self):
        if self.name not in [x[0].casefold() for x in op_names]:
            return False
        return True

    def isimm(self):
        name_alpha = alpha_re.match(str(self.name))
        value_alpha = alpha_re.match(str(self.value))
        name_imm = immediate_re.match(str(self.name))
        value_imm = immediate_re.match(str(self.value))

        if name_alpha and value_alpha and not name_imm and not value_imm:
            return False
        return True

op_names = [["MOVI", "imm2reg"],
            ["MOVR", "reg2reg"],
            ["LODI", "imm2reg"],
            ["LODR", "reg2reg"],
            ["STRI", "reg2imm"],
            ["STRR", "reg2reg"],
            ["ADDI", "imm2reg"],
            ["ADDR", "reg2reg"],
            ["SUBI", "imm2reg"],
            ["SUBR", "reg2reg"],
            ["ANDB", "byt2reg"],
            ["ANDW", "imm2reg"],
            ["ANDR", "reg2reg"],
            ["YORB", "byt2reg"],
            ["YORW", "imm2reg"],
            ["YORR", "reg2reg"],
            ["XORB", "byt2reg"],
            ["XORW", "imm2reg"],
            ["XORR", "reg2reg"],
            ["NOTR", "regonly"],
            ["MULI", "imm2reg"],
            ["MULR", "reg2reg"],
            ["DIVI", "imm2reg"],
            ["DIVR", "reg2reg"],
            ["SHLI", "imm2reg"],
            ["SHLR", "reg2reg"],
            ["SHRI", "imm2reg"],
            ["SHRR", "reg2reg"],
            ["PUSH", "regonly"],
            ["POOP", "regonly"],
            ["CMPB", "byt2reg"],
            ["CMPW", "imm2reg"],
            ["CMPR", "reg2reg"],
            ["JMPI", "jump"],
            ["JMPR", "jump"],
            ["JPAI", "jump"],
            ["JPAR", "jump"],
            ["JPBI", "jump"],
            ["JPBR", "jump"],
            ["JPEI", "jump"],
            ["JPER", "jump"],
            ["JPNI", "jump"],
            ["JPNR", "jump"],
            ["CALL", "jump"],
            ["RETN", "single"],
            ["SHIT", "single"],
            ["NOPE", "single"],
            ["GRMN", "single"]]

reg_names = ["R0", "R1", "R2", "R3", "S0", "S1", "S2", "S3", "IP", "RP", "SP"]
ops_sizes = {"reg2reg": 2,
             "imm2reg": 4,
             "reg2imm": 4,
             "byt2reg": 3,
             "regonly": 2,
             "immonly": 3,
             "jump": 3,
             "single": 1}

ops = [VMComponent(le[0], i, le[1]) for i, le in enumerate(op_names)]
regs = [VMComponent(s.casefold(), i) for i, s in enumerate(reg_names)]
instruction_re = re.compile(
    "^([\w]{4})(?:(?:\ *\#.*)|(?:\ +(?:([\w]+)\ *(?:,[\ ]*([\w]+))?)(?:\ *\#.*)?))?$")  # 1: opcode 2+: args
function_re = re.compile("(?:def\ )([a-zA-Z]*)\:(?:\ *\#.*)?$")
immediate_re = re.compile("(?:0x)?[0-9a-fA-F]+$")
alpha_re = re.compile("^[a-zA-Z]*$")
register_re = re.compile("(^[rRsS][0-4]$)|([iIrRsS][pP]$)")
label_re = re.compile("^([a-zA-Z]+)\:(?:\ *\#.*)?$")
symcall_re = re.compile(
    "^(?:[jJ][pPmM][pPaAbBeEnN][iIrR]|(?:[cC][aA][lL]{2}))\ +([\w]+)(?:\ *\#.*)?$")
commentline_re = re.compile("^\ *\#.*")


def main():
    parser = argparse.ArgumentParser(description='Optional app description')
    parser.add_argument(
        'opcodes_key', help='The key used to encrypt the opcodes')
    parser.add_argument('asmfile', help='The Pasticciotto assembly file')
    parser.add_argument('outfile', help='The output file')
    parser.add_argument('--debug', action='store_true',
                        help='Enables the DEBG opcode')
    args = parser.parse_args()

    if args.debug:
        global ops
        ops.append(VMComponent("DEBG", len(ops), "single"))
    with open(args.asmfile, 'r') as f:
        filedata = f.readlines()
    filedata = [x.strip() for x in filedata if x.strip()
                and not commentline_re.match(x)]

    vma = VMAssembler(args.opcodes_key, filedata)
    print(vma.functions)
    vma.parse()

    with open(args.outfile, 'wb') as f:
        f.write(vma.assembled_code)

if __name__ == '__main__':
    main()
