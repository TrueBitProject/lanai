pragma solidity ^0.4.0;

library MachineState {
    struct State {
        bytes code;
    	bytes mem;
    	uint[30] registers;
    	uint[3] pcDelay;

    	bool zero;
    	bool negative;
    	bool overflow;
    	bool carry;
    }
    using MachineState for MachineState.State;
    
    function init(bytes memory code, uint entrypoint, uint exitpoint, uint memsize)
        internal
        returns (State memory state)
    {
        state.mem = new bytes(memsize);
        state.code = code;
        state.registers[3] = memsize - 8; // FP
        state.registers[2] = memsize - 8; // SP
        state.memStoreWord(state.registers[3], exitpoint);
        state.regStore(2, entrypoint, 0);
    }
    function memStoreWord(State self, uint pos, uint value) internal {
        pos &= ~uint(3);
	    self.mem[pos + 0] = byte(uint8(value / 0x1000000));
	    self.mem[pos + 1] = byte(uint8(value / 0x10000));
	    self.mem[pos + 2] = byte(uint8(value / 0x100));
	    self.mem[pos + 3] = byte(uint8(value / 0x1));
    }
    function memStoreByte(State self, uint pos, uint value) internal {
	    self.mem[pos] = byte(uint8(value));
    }
    function memWord(State self, uint pos) internal returns (uint) {
        pos &= ~uint(3);
        return readWord(self.mem, pos);
    }
    function memByte(State self, uint pos) internal returns (uint) {
        return readByte(self.mem, pos);
    }
    function regStore(State self, uint reg, uint value, uint delayForPC)
        internal
    {
    	if (reg == 0 || reg == 1)
    		return;
    	else if (isPC(reg))
    	{
    		value = value & ~uint(3);
    		for (uint i = delayForPC; i < 3; ++i) {
    			self.pcDelay[i] = value;
    			value += 4;
    		}
    	}
    	else
    		self.registers[reg - 2] = value;
    }
    function reg(State self, uint r) internal returns (uint) {
    	if (r == 0)
    		return 0;
	    if (r == 1)
		    return uint(-1);
    	else
	    	return self.registers[r - 2];
    }
    function isPC(uint reg) internal returns (bool) {
        return reg == 2;
    }
    function pc(State state) internal returns (uint) {
        return state.reg(2);
    }
    function nextPC(State self) internal returns (uint) {
        return self.pcDelay[0];
    }
    function readWord(bytes memory data, uint pos) internal returns (uint) {
    	uint ret = 0;
    	for (uint i = 0; i < 4; i++)
    		ret = (ret * 256) | readByte(data, pos + i);
    	return ret;
    }
    function readByte(bytes memory data, uint pos) internal returns (uint) {
        return uint(uint8(data[pos]));
    }
    function currentInstruction(State self) internal returns (uint) {
        return readWord(self.code, self.pc());
    }
    function updatePC(State self) internal {
        self.registers[0] = self.pcDelay[0];
        self.pcDelay[0] = self.pcDelay[1];
        self.pcDelay[1] = self.pcDelay[2];
        self.pcDelay[2] = self.pcDelay[1] + 4;
    }
    /// @return (x >> endbit) & ((1 << len) - 1)
    function bitslice(uint x, uint endbit, uint len) returns (uint) {
        x /= uint(2)**endbit;
        return x & (uint(2)**len - 1);
    }

    function operation(
        State state,
        uint op, uint a, uint b,
        bool updateFlags, bool arithmeticShift
    ) internal returns (uint) {
    	uint result = 0;
    	bool carry = false;
    	a = uint32(a);
    	b = uint32(b);
    	if (op < 4)
    	{
    		// 0: add, 1: addc, 2: sub, 3: subb
    		uint bmod = uint32(op >= 2 ? ~b: b);
    		uint c = 0;
    		if (op == 1)
    			c = state.carry ? 1 : 0;
    		else if (op == 2)
    			c = 1;
    		else if (op == 3)
    			c = state.carry ? 0 : 1;
    		result = a + bmod + c;
    		if (updateFlags && (a + bmod + c >= 0x100000000))
    			carry = true;
    	}
    	else if (op == 4)
    		result = a & b;
    	else if (op == 5)
    		result = a | b;
    	else if (op == 6)
    		result = a ^ b;
    	else if (op == 7)
    	{
    		int amount = int32(b);
    		if (amount > 31 || amount < -31)
    			throw;
    		if (amount >= 0)
    		{
    			result = a * uint(2)**uint(amount);
    			carry = (result & 0x100000000) > 0;
    		}
    		else
    			result = a / uint(2)**uint(-amount);
    	}
    	result = uint32(result);
    
    	if (updateFlags)
    	{
    		uint msb = 0x80000000;
    		if (op != 3 || state.zero)
    			state.zero = result == 0;
    		state.negative = (result & msb) > 0;
    		if (op < 2)
    			state.overflow = (((a & msb) == (b & msb)) && ((a & msb) != (result & msb)));
    		else if (op < 4)
    			state.overflow = (((a & msb) == ((~b) & msb)) && ((a & msb) != (result & msb)));
    		else
    			state.overflow = false;
    		state.carry = carry;
    	}
    	return result;
    }
    struct Instruction {
        uint instruction;
        uint opcode;
        uint rd;
        uint rs1;
        uint rs2;
        uint p;
        uint q;
        uint constant_;
    }
    function condition(State memory state, uint condition, bool invert)
        internal
        returns (bool)
    {
    	bool takeBranch = false;
    	if (condition == 0)
    		takeBranch = true;
    	else if (condition == 1)
    		takeBranch = state.carry && !state.zero;
    	else if (condition == 2)
    		takeBranch = !state.carry;
    	else if (condition == 3)
    		takeBranch = !state.zero;
    	else if (condition == 4)
    		takeBranch = !state.overflow;
    	else if (condition == 5)
    		takeBranch = !state.negative;
    	else if (condition == 6)
    	{
    		if (invert)
    			return (state.negative && !state.overflow) || (!state.negative && state.overflow);
    		else
    			return (state.negative && state.overflow) || (!state.negative && !state.overflow);
    	}
    	else if (condition == 7)
    	{
    		if (invert)
    			return state.zero || (state.negative && !state.overflow) || (!state.negative && state.overflow);
    		else
    			return (state.negative && state.overflow && !state.zero) || (!state.negative && !state.overflow && !state.zero);
    	}
    	else
    		throw;
    	if (invert)
    		takeBranch = !takeBranch;
    
    	return takeBranch;
    }
    function decodeInstruction(uint instruction)
        internal
        returns (Instruction memory)
    {
        return Instruction({
            instruction: instruction,
            opcode: bitslice(instruction, 28, 4),
    		rd: bitslice(instruction, 23, 5),
    		rs1: bitslice(instruction, 18, 5),
    		rs2: bitslice(instruction, 11, 5),
    		p: bitslice(instruction, 17, 1),
    		q: bitslice(instruction, 16, 1),
    		constant_: bitslice(instruction, 0, 16)
        });
    }
    function instrRI(State memory self, Instruction memory i) internal
    {
    	uint constant32 = i.constant_ * 2**(16 * i.q);
		if (i.opcode == 4)
			constant32 |= i.q > 0 ? uint(0xffff) : uint(0xffff0000);
		else if (i.opcode == 7)
			constant32 = uint(int(int16(uint16(i.constant_))));
		uint result = self.operation(i.opcode, self.reg(i.rs1), constant32, i.p > 0, i.q > 0);
		self.regStore(i.rd, result, 1);
    }
    function instrRR(State memory self, Instruction memory i) internal
    {
		uint op = bitslice(i.instruction, 8, 3);
		if (op == 7)
			throw;
		uint result = self.operation(op, self.reg(i.rs1), self.reg(i.rs2), i.p > 0, false);
		self.regStore(i.rd, result, 1);
    }
    function instrRM(State memory self, Instruction memory i) internal
    {
		bool store = (i.opcode & 1) > 0;
		uint constantSE = uint(int(int16(uint16(i.constant_))));
		uint ea = self.reg(i.rs1) + (i.p > 0 ? constantSE : 0);
		if (store)
			self.memStoreWord(ea, self.reg(i.rd));
		else
			self.regStore(i.rd, self.memWord(ea), 2);
		if (i.q > 0) {
			if (isPC(i.rs1))
				throw; // TODO does this also have delay slots?
			self.regStore(i.rs1, self.reg(i.rs1) + constantSE, 0);
		}
    }
    function instrBR(State memory self, Instruction memory i) internal
    {
		uint cond = bitslice(i.instruction, 25, 3);
		uint value = 0;
		bool invert = (i.instruction & 1) > 0;
		bool takeBranch = condition(self, cond, invert);
		uint targetReg = 2; // pc
		if ((i.instruction & 0x2) > 0) {
			if ((i.instruction & 0x1000000) > 0)
				value = self.reg(i.rs1) + (i.instruction & 0xfffc);
			else
			{
				value = takeBranch ? 1 : 0;
				targetReg = i.rs1;
			}
		}
		else
			value = i.instruction & 0x1fffffc;
		if (takeBranch)
			self.regStore(targetReg, value, 1);
    }
    function instrRRM(State memory self, Instruction memory i) internal {
		bool store = (i.opcode & 1) > 0;
		uint op = bitslice(i.instruction, 8, 3);
		if (op == 7)
			throw;
		bool y = (i.instruction & 4) > 0;
		bool l = (i.instruction & 2) > 0;
		bool e = (i.instruction & 1) > 0;
		if (!y && !l)
			throw;
		uint ea = self.reg(i.rs1);
		if (i.p > 0) {
			if (op == 7) throw;
			ea = self.operation(op, self.reg(i.rs1), self.reg(i.rs2), false, false);
		}
		if (store) {
			uint val = self.reg(i.rd);
			if (y)
				self.memStoreByte(ea, val);
			else if (!l)
				throw;
			else
				self.memStoreWord(ea, val);
		} else {
			val = 0;
			if (y)
			{
				val = self.memByte(ea);
				if (e)
					val = uint(int(int8(val)));
				else
					val = val & 0xff;
			}
			else if (!l)
				throw;
			else
				val = self.memWord(ea);
			self.regStore(i.rd, val, 2);
		}
		if (i.q > 0) {
			if (isPC(i.rs1))
				throw; // TODO does this also have delay slots?
			if (op == 7) throw;
			uint result = self.operation(op, self.reg(i.rs1), self.reg(i.rs2), false, false);
			self.regStore(i.rs1, result, 0);
		}
	}
	function instrSLS(State memory self, Instruction memory i) internal
	{
		if (i.p == 0 && i.q != 0)
    		throw;
		uint value = (i.rs1 * 0x10000) | i.constant_;
		if (i.q != 0)
			self.memStoreWord(value, self.reg(i.rd));
		else {
			if (i.p == 0)
				value = self.memWord(i.constant_);
			self.regStore(i.rd, value, 2);
		}
	}
    function step(State self) internal {
        self.updatePC();
        Instruction memory i = decodeInstruction(self.currentInstruction());
        if (i.opcode & 8 == 0) {
            self.instrRI(i);
		} else if (i.opcode == 0xc) {
		    self.instrRR(i);
		} else if ((i.opcode & 0xe) == 0x8) {
		    self.instrRM(i);
		} else if ((i.opcode & 0xe) == 0xa) {
		    self.instrRRM(i);
		} else if (i.opcode == 0xe) {
		    self.instrBR(i);
		} else if (i.opcode == 0xf) {
		    self.instrSLS(i);
		} else
		    throw;
    }
}

contract Lanai {
    using MachineState for MachineState.State;
    function shl(uint x, uint i) internal returns (uint) {
        return x * 2**i;
    }

    event StateLog(uint step, uint pc, uint rv);
    function runForSteps(uint n) returns (uint) {
        uint exitpoint = 0xfffff0;
        var state = getExampleProg();
        for (uint i = 0; i < n && state.nextPC() != exitpoint; i++) {
            state.step();
            StateLog(i, state.reg(2), state.reg(8));
        }
        return state.reg(8);
    }

    function getExampleProg() internal returns (MachineState.State memory) {
        bytes memory code = hex"7f454c46010201000000000000000000000100f400000001000000000000000000000170000000000034000000000028000600019293fffc02900008221000109016fff4f40200148116fffc021400008296fff800636c616e672076657273696f6e20342e302e302028687474703a2f2f6c6c766d2e6f72672f6769742f636c616e672e6769742036353632303466666234356262663035363130313236356433616534383131363338313834633137292028687474703a2f2f6c6c766d2e6f72672f6769742f6c6c766d2e676974206336363262376561653363346666643434636534326538353032346439343031356163356230386129000000000000000000000000000000000000000000002500000000000000000400fff100000010000000000000002012000002002e74657874002e636f6d6d656e74006d61696e002e6e6f74652e474e552d737461636b002f6d6e742f6d696e696d616c2e63002e737472746162002e73796d7461620000000000000000000000000000000000000000000000000000000000000000000000000000000000000000340000000300000000000000000000012c0000004400000000000000000000000100000000000000010000000100000006000000000000003400000020000000000000000000000004000000000000000700000001000000300000000000000054000000a60000000000000000000000010000000100000015000000010000000000000000000000fa00000000000000000000000000000001000000000000003c000000020000000000000000000000fc0000003000000001000000020000000400000010";
        uint entrypoint = 0x34;
        return MachineState.init(code, entrypoint, 0xfffff0, 0x100);
    }
    function test() returns (uint) {
        bytes memory code = hex"7f454c46010201000000000000000000000100f400000001000000000000000000000170000000000034000000000028000600019293fffc02900008221000109016fff4f40200148116fffc021400008296fff800636c616e672076657273696f6e20342e302e302028687474703a2f2f6c6c766d2e6f72672f6769742f636c616e672e6769742036353632303466666234356262663035363130313236356433616534383131363338313834633137292028687474703a2f2f6c6c766d2e6f72672f6769742f6c6c766d2e676974206336363262376561653363346666643434636534326538353032346439343031356163356230386129000000000000000000000000000000000000000000002500000000000000000400fff100000010000000000000002012000002002e74657874002e636f6d6d656e74006d61696e002e6e6f74652e474e552d737461636b002f6d6e742f6d696e696d616c2e63002e737472746162002e73796d7461620000000000000000000000000000000000000000000000000000000000000000000000000000000000000000340000000300000000000000000000012c0000004400000000000000000000000100000000000000010000000100000006000000000000003400000020000000000000000000000004000000000000000700000001000000300000000000000054000000a60000000000000000000000010000000100000015000000010000000000000000000000fa00000000000000000000000000000001000000000000003c000000020000000000000000000000fc0000003000000001000000020000000400000010";
        uint entrypoint = 0x34;
        var state = MachineState.init(code, entrypoint, 0xfffff0, 0x100);
        return state.pc();
    }
}
