#include <fstream>
#include <iostream>
#include <string>
#include <exception>
#include <functional>
#include <map>

#include "decoder.h"

using namespace std;

uint32_t loadBigEndian(char const* _data)
{
	uint32_t ret = 0;
	for (unsigned i = 0; i < 4; i++)
		ret = (ret << 8) | (uint32_t(_data[i]) & 0xff);
	return ret;
}

void run(string const& _text)
{
	MachineState state;
	state.registers[4] = 0x10000;
	state.memory.resize(0x20000);
	while (true)
	{
		uint32_t instruction = loadBigEndian(_text.data() + state.pc);
		unsigned opcode = instruction >> 28;
		cout << "PC: " << hex << state.pc << endl;
//		cout << "Mem: ";
//		for (size_t i = 0; i < state.memory.size(); i++)
//			cout << state.memory[i];
//		cout << endl;
		uint16_t rd = (instruction >> 23) & 0x1f;
		uint16_t rs1 = (instruction >> 18) & 0x1f;
		bool p = (instruction >> 17) & 1;
		bool q = (instruction >> 16) & 1;
		uint16_t constant = instruction;

		if ((opcode & 8) == 0) // Register Immediate (RI)
		{
			cout << "RI -- opcode: " << opcode << " rd: " << rd << " rs1: " << rs1 << " f: " << p << " h: " << q << " constant: " << constant << endl;

			uint32_t constant32 = q ? constant << 16 : constant;
			if (opcode == 0)
				state.registers[rd] = state.registers[rs1] + constant32;
			else if (opcode == 1) // TDOO should be "addc"
				state.registers[rd] = state.registers[rs1] + constant32;
			else if (opcode == 2)
				state.registers[rd] = state.registers[rs1] - constant32;
			else if (opcode == 3) // TODO should be "subb"
				state.registers[rd] = state.registers[rs1] - constant32;
			else if (opcode == 4)
			{
				constant32 |= q ? uint32_t(0xffff) : uint32_t(0xffff0000);
				state.registers[rd] = state.registers[rs1] & constant32;
			}
			else if (opcode == 5)
				state.registers[rd] = state.registers[rs1] | constant32;
			else if (opcode == 6)
				state.registers[rd] = state.registers[rs1] ^ constant32;
			else if (opcode == 7)
			{
				if (q)
					constant32 = int32_t(int16_t(constant)) << 16;
				throw new exception; // shift
			}
			// TODO flags / "condition bits"
		}
		else if (opcode == 0xc) // Register Register (RR)
		{
			cout << "Opcode 0xc not implemented." << endl;
			throw new exception;
		}
		else if ((opcode & 0xe) == 0x8) // Register Memory (RM)
		{
			// TODO: "the constant is sign-extended for this instruction"
			bool store = opcode & 1;
			cout << "RM -- store: " << store << " rd: " << rd << " rs1: " << rs1 << " p: " << p << " q: " << q << " constant: " << constant << endl;
			int32_t constantSE = int16_t(constant);
			uint32_t ea = state.registers[rs1] + (p ? constantSE : 0);
			if (store)
				state.memory[ea] = state.registers[rd];
			else
				state.registers[rd] = state.memory[ea];
			if (q)
				state.registers[rs1] += constantSE;
		}
		else if ((opcode & 0xe) == 0xa) // Register Register Memory (RRM)
		{
			cout << "Opcode RRM not implemented." << endl;
			throw new exception;
		}
		else if (opcode == 0xe) // Conditional branch
		{
			uint16_t condition = (instruction >> 24) & 7;
			uint32_t constantTimes4 = instruction & (((uint32_t(1) << 23) - 1) << 2);
			bool invert = instruction & 1;
			bool takeBranch = false;
			if (condition == 0)
				takeBranch = true;
			else
				throw new exception;
			if (invert)
				takeBranch = !takeBranch;
			cout << "BR: condition: " << condition << " i: " << invert << " constant*4: " << constantTimes4 << endl;
			if (takeBranch)
			{
				state.pc = constantTimes4;
				continue;
			}
		}
		else if (opcode == 0xf) // Special Load/Store and others
		{
			cout << "Opcode not implemented." << endl;
			throw new exception;

		}
		else if (opcode == 0xd) // Special instructions (popc, leadz, trailz)
		{
			cout << "Opcode not implemented." << endl;
			throw new exception;
		}
		else
		{
			cout << "Invalid opcode." << endl;
			throw new exception;
		}
		state.pc += 4;
	}
}

void link(string& _data)
{
	// TODO read this from the elf header
	vector<uint32_t> symbols{{0x24, 0x44, 0x54, 0x64}};
	map<uint32_t, size_t> references{{0x1c, 0}, {0x34, 3}, {0x3c, 1}, {0x4c, 2}, {0x5c, 0}};

	// TODO do we need to decode the instruction or do we always insert it at the same point?
	for (auto const& ref: references)
	{
		if ((uint32_t(_data[ref.first]) & 0xf0) != 0xe0)
		{
			cout << "Error linking at " << hex << ref.first << ": not a br: " << (unsigned(_data[ref.first]) >> 4) << endl;
			//throw new exception; // not a BR instruction
		}
		uint32_t target = symbols[ref.second];
		if (target % 4 != 0)
		{
			cout << "Error linking at " << hex << ref.first << ": Invalid alignment." << endl;
			throw new exception; // invalid alignment
		}
		for (size_t i = 0; i < 4; target >>= 8, ++i)
			_data[ref.first + 3 - i] |= target & 0xff;
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "Usage: " << argv[0] << " <file.o>" << endl;
		cout << "  Runs the function called \"run\" in the specified lanai object file." << endl;
		return 1;
	}
	ifstream in(argv[1], std::ifstream::binary);
	if (!in)
		return 1;
	// Read this from the elf header, currently, this is hardcoded for the example
	in.seekg(0x34, in.beg);
	string data;
	data.resize(0x70);
	in.read(const_cast<char*>(data.data()), 0x70);

	link(data);

	run(data);

	return 0;
}
