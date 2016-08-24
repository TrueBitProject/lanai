#include <fstream>
#include <iostream>
#include <string>
#include <exception>

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
	while (true)
	{
		uint32_t instruction = loadBigEndian(_text.data() + state.pc);
		unsigned opcode = instruction >> 28;
		cout << "Opcode: " << hex << opcode << endl;
		uint16_t rd = (instruction >> 23) & 0x1f;
		uint16_t rs1 = (instruction >> 18) & 0x1f;
		bool p = (instruction >> 17) & 1;
		bool q = (instruction >> 16) & 1;
		uint16_t constant = instruction;

		if ((opcode & 8) == 0) // Register Immediate (RI)
		{
			cout << "RI -- opcode: " << opcode << " rd: " << rd << " rs1: " << rs1 << " f: " << p << " h: " << q << " constant: " << constant << endl;
			cout << "Opcode not implemented." << endl;
//			throw new exception;
		}
		else if (opcode == 0xc) // Register Register (RR)
		{
			cout << "Opcode not implemented." << endl;
			throw new exception;
		}
		else if ((opcode & 0xe) == 0x8) // Register Memory (RM)
		{
			bool store = opcode & 1;
			cout << "RM -- store: " << store << " rd: " << rd << " rs1: " << rs1 << " p: " << p << " q: " << q << " constant: " << constant << endl;

			cout << "Opcode not implemented." << endl;
			//throw new exception;
		}
		else if ((opcode & 0xe) == 0xa) // Register Register Memory (RRM)
		{
			cout << "Opcode not implemented." << endl;
			throw new exception;
		}
		else if (opcode == 0xe) // Conditional
		{
			cout << "Opcode not implemented." << endl;
			throw new exception;
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

	run(data);

	return 0;
}
