#include <string>
#include <iostream>
#include <iomanip>
#include <exception>
#include <functional>
#include <map>

#include "interpreter.h"

using namespace std;


uint32_t Interpreter::run(uint32_t entrypoint, bool verbose)
{
	uint32_t exitpoint = (uint32_t(1) << 31) & ~uint32_t(3);
	initState(entrypoint, exitpoint, 0x1000);
	while (true)
	{
		steps++;
		updatePC();
		if (verbose)
			printState();

		if (pc() == exitpoint)
		{
			// Program terminated
			cout << "Program terminated after " << dec << steps << " steps." << endl;
			cout << "Return value: 0x" << setfill('0') << setw(8) << hex << state.rv << endl;
			return state.rv;
		}

		uint32_t instruction = currentInstruction();
		unsigned opcode = instruction >> 28;
		uint16_t rd = (instruction >> 23) & 0x1f;
		uint16_t rs1 = (instruction >> 18) & 0x1f;
		uint16_t rs2 = (instruction >> 11) & 0x1f;
		bool p = (instruction >> 17) & 1;
		bool q = (instruction >> 16) & 1;
		uint16_t constant = instruction;

		if ((opcode & 8) == 0) // Register Immediate (RI)
		{
			if (verbose)
				cout << "RI -- " << operationName(opcode) << " rd: " << rd << " rs1: " << rs1 << " f: " << p << " h: " << q << " constant: " << constant << endl;

			uint32_t constant32 = q ? constant << 16 : constant;
			if (opcode == 4)
				constant32 |= q ? uint32_t(0xffff) : uint32_t(0xffff0000);
			else if (opcode == 7)
				constant32 = int32_t(int16_t(constant));
			uint32_t result = operation(opcode, reg(rs1), constant32, p, q);
			regStore(rd, result, 1);
		}
		else if (opcode == 0xc) // Register Register (RR)
		{
			uint16_t op = (instruction >> 8) & 0x7;
			if (verbose)
				cout << "RR -- " << operationName(op) << " rs1: " << rs1 << " rs2: " << rs2 << " f: " << p << constant << endl;
			if (op == 7)
				throw new exception;
			uint32_t result = operation(op, reg(rs1), reg(rs2), p);
			regStore(rd, result, 1);
		}
		else if ((opcode & 0xe) == 0x8) // Register Memory (RM)
		{
			bool store = opcode & 1;
			if (verbose)
				cout << "RM -- store: " << store << " rd: " << rd << " rs1: " << rs1 << " p: " << p << " q: " << q << " constant: " << constant << endl;
			int32_t constantSE = int16_t(constant);
			uint32_t ea = reg(rs1) + (p ? constantSE : 0);
			if (store)
				memStoreWord(ea, reg(rd));
			else
				regStore(rd, memWord(ea), 2);
			if (q)
			{
				if (isPC(rs1))
					throw new exception; // TODO does this also have delay slots?
				regStore(rs1, reg(rs1) + constantSE, 0);
			}
		}
		else if ((opcode & 0xe) == 0xa) // Register Register Memory (RRM)
		{
			bool store = opcode & 1;
			uint16_t op = (instruction >> 8) & 0x7;
			if (verbose)
				cout << "RRM -- store: " << store << " rd: " << rd << " rs1: " << rs1 << " p: " << p << " q: " << q << " constant: " << constant << endl;
			if (op == 7)
				throw new exception;
			bool y = instruction & 4;
			bool l = instruction & 2;
			bool e = instruction & 1;
			if (!y && !l)
				throw new exception;
			uint32_t ea = reg(rs1);
			if (p)
			{
				if (op == 7) throw new exception;
				ea = operation(op, reg(rs1), reg(rs2), false);
			}
			if (store)
			{
				uint32_t val = reg(rd);
				if (y)
					memStoreByte(ea, val);
				else if (!l)
					throw new exception;
				else
					memStoreWord(ea, val);
			}
			else
			{
				uint32_t val = 0;
				if (y)
				{
					val = memByte(ea);
					if (e)
						val = int32_t(int8_t(val));
					else
						val = val & 0xff;
				}
				else if (!l)
					throw new exception;
				else
					val = memWord(ea);
				regStore(rd, val, 2);
			}
			if (q)
			{
				if (isPC(rs1))
					throw new exception; // TODO does this also have delay slots?
				if (op == 7) throw new exception;
				regStore(rs1, operation(op, reg(rs1), reg(rs2), false), 0);
			}
		}
		else if (opcode == 0xe) // Conditional branch
		{
			uint16_t cond = (instruction >> 25) & 7;
			uint32_t value = 0;
			bool invert = instruction & 1;
			bool takeBranch = condition(cond, invert);
			if (verbose)
				cout << "BR: condition: " << cond << " i: " << invert << " constant: " << constant << endl;
			uint16_t targetReg = 2; // pc
			if (instruction & 0x2)
			{
				if (instruction & (uint32_t(1) << 24))
					value = reg(rs1) + (instruction & (((uint32_t(1) << 14) - 1) << 2));
				else
				{
					value = takeBranch;
					targetReg = rs1;
				}
			}
			else
				value = instruction & (((uint32_t(1) << 23) - 1) << 2);
			if (takeBranch)
				regStore(targetReg, value, 1);
		}
		else if (opcode == 0xf) // Special Load/Store and others
		{
			if (!p && q)
			{
				cout << "Opcode 0xf with !p && q not implemented." << endl;
				throw new exception;
			}
			uint32_t value = (uint32_t(rs1) << 16) | constant;
			if (verbose)
				cout << "SLS: rd: " << rd << " addr upper: " << rs1 << " addr lower: " << constant  << endl;
			if (q)
				memStoreWord(value, reg(rd));
			else
			{
				if (!p)
					value = memWord(constant);
				regStore(rd, value, 2);
			}
		}
		else if (opcode == 0xd) // Special instructions (popc, leadz, trailz)
		{
			cout << "Opcode 0xd not implemented." << endl;
			throw new exception;
		}
		else
		{
			cout << "Invalid opcode." << endl;
			throw new exception;
		}
	}
}

void Interpreter::initState(uint32_t entrypoint, uint32_t exitpoint, uint32_t memsize)
{
	state.fp = memsize;
	state.sp = memsize;
	memStoreWord(state.fp, exitpoint);
	regStore(2, entrypoint, 0);
}

void Interpreter::updatePC()
{
	state.pc = state.pcDelay[0];
	state.pcDelay[0] = state.pcDelay[1];
	state.pcDelay[1] = state.pcDelay[2];
	state.pcDelay[2] = state.pcDelay[1] + 4;
}

void Interpreter::printState()
{
	cout << "---------------------------------------------------" << endl;
	printMemory();
	cout << "Regs:" << endl;
	cout << "pc/2:   " << hex << reg(2) << " [ " <<
		state.pcDelay[0] << ", " <<
		state.pcDelay[1] << ", " <<
		state.pcDelay[2] << " ]" <<
		endl;
	cout << "r3:     " << hex << reg(3) << endl;
	cout << "sp/4:   " << hex << reg(4) << endl;
	cout << "fp/5:   " << hex << reg(5) << endl;
	cout << "rv/8:   " << hex << reg(8) << endl;
	cout << "rca/15: " << hex << reg(15);
	for (unsigned i = 4; i < 32; i ++)
	{
		if (i % 4 == 0)
			cout << endl << hex << setw(2) << i << ":";
		cout << " " << hex << setfill('0') << setw(8) << reg(i);
	}
	cout << endl;
	cout << "Flags: " << (state.zero ? "Z" : "z") << (state.overflow ? "V" : "v") << (state.negative ? "N" : "n") << (state.carry ? "C" : "c") << endl;
}

void Interpreter::printMemory()
{
	string const& data = state.memory;
	cout << "Mem: " << endl;
	cout << hex;
	unsigned rowlength = 32;
	for (size_t i = 0; i < data.length(); i += rowlength)
	{
		if (data.substr(i, rowlength) == string(rowlength, 0))
			continue;
		cout << setw(4) << i << ":";
		for (size_t j = 0; j < rowlength; j++)
		{
			if (j % 4 == 0)
				cout << " ";
			cout << setfill('0') << setw(2) << unsigned(uint8_t(data[i + j]));
		}
		cout << endl;
	}
}

uint32_t Interpreter::currentInstruction() const
{
	return readWord(code, pc());
}

bool Interpreter::condition(uint16_t condition, bool invert) const
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
		throw new exception;
	if (invert)
		takeBranch = !takeBranch;

	return takeBranch;
}

uint32_t Interpreter::operation(uint16_t op, uint32_t a, uint32_t b, bool updateFlags, bool arithmeticShift)
{
	uint32_t result = 0;
	bool carry = false;
	if (op < 4)
	{
		// 0: add, 1: addc, 2: sub, 3: subb
		uint32_t bmod = op >= 2 ? ~b: b;
		uint32_t c = 0;
		if (op == 1)
			c = state.carry;
		else if (op == 2)
			c = 1;
		else if (op == 3)
			c = !state.carry;
		result = a + bmod + c;
		if (updateFlags && (uint64_t(a) + uint64_t(bmod) + uint64_t(c) >= (uint64_t(1) << 32)))
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
		int32_t amount = int32_t(b);
		if (amount > 31 || amount < -31)
			throw new exception;
		if (amount >= 0)
		{
			result = a << amount;
			carry = (uint64_t(a) << amount) & (uint64_t(1) << 32);
		}
		else
			result = a >> (-amount);
	}

	if (updateFlags)
	{
		uint32_t msb = 0x80000000;
		if (op != 3 || state.zero)
			state.zero = result == 0;
		state.negative = result & msb;
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

string Interpreter::operationName(uint16_t op)
{
	if (op == 0)
		return "add";
	else if (op == 1)
		return "addc";
	else if (op == 2)
		return "sub";
	else if (op == 3)
		return "subb";
	else if (op == 4)
		return "and";
	else if (op == 5)
		return "or";
	else if (op == 6)
		return "xor";
	else if (op == 7)
		return "shift";
	return "???";
}

uint32_t Interpreter::reg(uint16_t i) const
{
	if (i == 0)
		return 0;
	if (i == 1)
		return uint32_t(-1);
	else
		return state.registers[i - 2];
}

void Interpreter::regStore(uint16_t reg, uint32_t value, unsigned delayForPC)
{
	if (reg == 0 || reg == 1)
		return;
	else if (isPC(reg))
	{
		value = value & ~uint32_t(3);
		for (unsigned i = delayForPC; i < 3; ++i, value += 4)
			state.pcDelay[i] = value;
	}
	else
		state.registers[reg - 2] = value;
}

uint32_t Interpreter::readWord(string const& data, uint32_t pos)
{
	if (pos > data.size() - 4)
		throw new exception;
	uint32_t ret = 0;
	for (unsigned i = 0; i < 4; i++)
		ret = (ret << 8) | (uint32_t(data[pos + i]) & 0xff);
	return ret;
}

uint32_t Interpreter::readByte(string const& data, uint32_t pos)
{
	if (pos >= data.size())
		throw new exception;
	return uint8_t(data[pos]);
}

uint32_t Interpreter::memWord(uint32_t index)
{
	index &= ~uint32_t(3);
	state.memory.resize(max<size_t>(state.memory.length(), index + 4), 0);
	return readWord(state.memory, index);
}

uint32_t Interpreter::memByte(uint32_t index)
{
	state.memory.resize(max<size_t>(state.memory.length(), index + 1), 0);
	return readByte(state.memory, index);
}

void Interpreter::memStoreWord(uint32_t pos, uint32_t value)
{
	pos &= ~uint32_t(3);
	state.memory.resize(max<size_t>(state.memory.length(), pos + 4), 0);
	for (size_t i = 0; i < 4; ++i)
		state.memory[pos + i] = (value >> (24 - i * 8)) & 0xff;
}

void Interpreter::memStoreByte(uint32_t pos, uint32_t value)
{
	state.memory.resize(max<size_t>(state.memory.length(), pos + 1), 0);
	state.memory[pos] = value & 0xff;
}
