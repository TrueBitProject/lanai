#include <fstream>
#include <climits>
#include <iostream>
#include <string>
#include <exception>
#include <functional>
#include <map>

#include <elf.h>

#include "decoder.h"

using namespace std;

template <typename T>
T swap_endian(T u)
{
	union
	{
		T u;
		unsigned char u8[sizeof(T)];
	} source, dest;

	source.u = u;

	for (size_t k = 0; k < sizeof(T); k++)
		dest.u8[k] = source.u8[sizeof(T) - k - 1];

	return dest.u;
}

uint32_t loadInstruction(char const* _data)
{
	uint32_t ret = 0;
	for (unsigned i = 0; i < 4; i++)
		ret = (ret << 8) | (uint32_t(_data[i]) & 0xff);
	return ret;
}

bool condition(uint16_t condition, bool invert, MachineState const& state)
{
	bool takeBranch = false;
	if (condition == 0)
		takeBranch = true;
	else if (condition == 1)
		throw new exception;
	else if (condition == 2)
		throw new exception;
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

uint32_t operation(uint16_t op, uint32_t a, uint32_t b, MachineState& state, bool updateFlags)
{
	uint32_t result = 0;
	if (op == 0)
		result = a + b;
	else if (op == 1)
		throw new exception; // result = a + b + (state.carry ? 1 : 0);
	else if (op == 2)
		result = a + ~b + 1;
	else if (op == 3)
		throw new exception; // result = a + ~b + (state.carry ? 1 : 0);
	else if (op == 4)
		result = a & b;
	else if (op == 5)
		result = a | b;
	else if (op == 6)
		result = a ^ b;
	else if (op == 7)
		throw new exception; // shift

	if (updateFlags)
	{
		state.zero = result == 0;
		state.negative = result & uint32_t(0x80000000);
		if (op < 4)
		{
			uint32_t msb = 0x80000000;
			state.overflow = ((a & msb) == (b & msb) && (a & msb) != (result & msb));
		}
		else
			state.overflow = false;
	}
	return result;
}

void store32(uint32_t index, uint32_t data, MachineState& state)
{
	state.memory.resize(max<size_t>(state.memory.length(), index + 4), 0);
	for (size_t i = 0; i < 4; ++i)
		state.memory[index + i] = (data >> (24 - i * 8)) & 0xff;
}

uint32_t load32(uint32_t index, MachineState& state)
{
	state.memory.resize(max<size_t>(state.memory.length(), index + 4), 0);
	uint32_t result = 0;
	for (size_t i = 0; i < 4; ++i)
		result |= (uint32_t(state.memory[index + i]) << (24 - i * 8));
	return result;
}

void store8(uint32_t index, uint32_t data, MachineState& state)
{
	state.memory.resize(max<size_t>(state.memory.length(), index + 1), 0);
	state.memory[index] = data & 0xff;
}

uint32_t load8(uint32_t index, MachineState& state)
{
	state.memory.resize(max<size_t>(state.memory.length(), index + 1), 0);
	return uint32_t(state.memory[index]);
}

void hexsummary(string const& data)
{
	cout << hex << data.length() << endl;
	for (auto const& x: data)
		if (x != 0)
			cout << uint32_t(x);

}

void registerStore(uint16_t reg, uint32_t value, unsigned delayForPC, MachineState& state)
{
	if (reg == 0 || reg == 1)
		return;
	else if (&state.registers[reg] == &state.pc)
	{
		if (delayForPC == 0)
			state.pc = value;
		else
			state.pcDelay[delayForPC - 1] = value;
		for (unsigned i = delayForPC; i < 3; ++i)
		{
			value += 4;
			state.pcDelay[i] = value;
		}
	}
	else
		state.registers[reg] = value;
}

void run(string const& _text, uint32_t entrypoint)
{
	MachineState state;
	state.fp = 0x1000;
	state.sp = 0x1000;
	state.pcDelay[0] = entrypoint;
	state.pcDelay[1] = entrypoint + 4;
	state.pcDelay[2] = entrypoint + 8;

	while (true)
	{
		state.registers[0] = 0;
		state.registers[1] = 1;
		state.pc = state.pcDelay[0];
		state.pcDelay[0] = state.pcDelay[1];
		state.pcDelay[1] = state.pcDelay[2];
		state.pcDelay[2] = state.pcDelay[1] + 4;
		uint32_t instruction = loadInstruction(_text.data() + state.pc);
		unsigned opcode = instruction >> 28;
		cout << "PC: " << hex << state.pc << endl;
		cout << "Opcode: " << hex << opcode << endl;
		cout << "Mem: ";
		hexsummary(state.memory);
		cout << endl;
		uint16_t rd = (instruction >> 23) & 0x1f;
		uint16_t rs1 = (instruction >> 18) & 0x1f;
		uint16_t rs2 = (instruction >> 11) & 0x1f;
		bool p = (instruction >> 17) & 1;
		bool q = (instruction >> 16) & 1;
		uint16_t constant = instruction;

		if ((opcode & 8) == 0) // Register Immediate (RI)
		{
			cout << "RI -- opcode: " << opcode << " rd: " << rd << " rs1: " << rs1 << " f: " << p << " h: " << q << " constant: " << constant << endl;

			uint32_t constant32 = q ? constant << 16 : constant;
			if (opcode == 4)
				constant32 |= q ? uint32_t(0xffff) : uint32_t(0xffff0000);
			else if (opcode == 7)
			{
				if (q)
					constant32 = int32_t(int16_t(constant)) << 16;
			}
			uint32_t result = operation(opcode, state.registers[rs1], constant32, state, p);
			registerStore(rd, result, 1, state);
		}
		else if (opcode == 0xc) // Register Register (RR)
		{
			uint16_t op = (instruction >> 8) & 0x7;
			if (op == 7)
				throw new exception;
			uint32_t result = operation(op, state.registers[rs1], state.registers[rs2], state, p);
			registerStore(rd, result, 1, state);
		}
		else if ((opcode & 0xe) == 0x8) // Register Memory (RM)
		{
			bool store = opcode & 1;
			cout << "RM -- store: " << store << " rd: " << rd << " rs1: " << rs1 << " p: " << p << " q: " << q << " constant: " << constant << endl;
			int32_t constantSE = int16_t(constant);
			uint32_t ea = state.registers[rs1] + (p ? constantSE : 0);
			if (store)
				store32(ea, state.registers[rd], state);
			else
				registerStore(rd, load32(ea, state), 2, state);
			if (q)
			{
				if (&state.registers[rs1] == &state.pc)
					throw new exception; // TODO does this also have delay slots?
				state.registers[rs1] += constantSE;
			}
		}
		else if ((opcode & 0xe) == 0xa) // Register Register Memory (RRM)
		{
			bool store = opcode & 1;
			uint16_t op = (instruction >> 8) & 0x7;
			if (op == 7)
				throw new exception;
			bool y = instruction & 4;
			bool l = instruction & 2;
			bool e = instruction & 1;
			if (!y && !l)
				throw new exception;
			uint32_t ea = state.registers[rs1];
			if (p)
				ea = operation(op, state.registers[rs1], state.registers[rs2], state, false);
			if (store)
			{
				uint32_t val = state.registers[rd];
				if (y)
					store8(ea, val, state);
				else if (!l)
					throw new exception;
				else
					store32(ea, val, state);
			}
			else
			{
				uint32_t val = 0;
				if (y)
				{
					val = load8(ea, state);
					if (e)
						val = int32_t(int8_t(val));
					else
						val = val & 0xff;
				}
				else if (!l)
					throw new exception;
				else
					val = load32(ea, state);
				registerStore(rd, val, 2, state);
			}
			if (q)
			{
				if (&state.registers[rs1] == &state.pc)
					throw new exception; // TODO does this also have delay slots?
				state.registers[rs1] = operation(op, state.registers[rs1], state.registers[rs2], state, false);
			}
		}
		else if (opcode == 0xe) // Conditional branch
		{
			uint16_t cond = (instruction >> 24) & 7;
			uint32_t constantTimes4 = instruction & (((uint32_t(1) << 23) - 1) << 2);
			bool invert = instruction & 1;
			bool takeBranch = condition(cond, invert, state);
			cout << "BR: condition: " << cond << " i: " << invert << " constant*4: " << constantTimes4 << endl;
			if (takeBranch)
				registerStore(2 /* pc */, constantTimes4, 1, state);
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
	}
}

/// ELF tools

static inline Elf32_Shdr* elf_sheader(Elf32_Ehdr* hdr)
{
	return reinterpret_cast<Elf32_Shdr *>(reinterpret_cast<ptrdiff_t>(hdr) + swap_endian(hdr->e_shoff));
}

static inline Elf32_Shdr* elf_section(Elf32_Ehdr* hdr, int idx)
{
	return &elf_sheader(hdr)[idx];
}

static uint32_t elf_get_symval(Elf32_Ehdr* hdr, Elf32_Sym* symbol)
{
	if (swap_endian(symbol->st_shndx) == SHN_UNDEF)
	{
		cerr << "Value of external symbol requested." << endl;
		return 0;
	}
	else if (swap_endian(symbol->st_shndx) == SHN_ABS)
		return swap_endian(symbol->st_value);
	else
	{
		Elf32_Shdr* target = elf_section(hdr, swap_endian(symbol->st_shndx));
		return swap_endian(symbol->st_value) + swap_endian(target->sh_offset);
	}
}

static uint32_t elf_get_symval(Elf32_Ehdr* hdr, int table, uint idx)
{
	if (table == SHN_UNDEF || idx == SHN_UNDEF)
		return 0;
	Elf32_Shdr* symtab = elf_section(hdr, table);

	uint32_t symtab_entries = swap_endian(symtab->sh_size) / swap_endian(symtab->sh_entsize);
	if(idx >= symtab_entries)
	{
		cerr << "Symbol index out of range." << endl;
		throw new exception;
	}

	int symaddr = reinterpret_cast<ptrdiff_t>(hdr) + swap_endian(symtab->sh_offset);
	Elf32_Sym* symbol = &(reinterpret_cast<Elf32_Sym*>(symaddr))[idx];
	return elf_get_symval(hdr, symbol);
}

/// End of ELF tools

uint32_t link(string& _data)
{
	uint32_t entrypoint = 0;

	Elf32_Ehdr* hdr = reinterpret_cast<Elf32_Ehdr*>(const_cast<char*>(_data.data()));
	Elf32_Shdr* strtab = elf_section(hdr, swap_endian(hdr->e_shstrndx));
	// Iterate over section headers
	for (size_t i = 0; i < swap_endian(hdr->e_shnum); i++) {
		Elf32_Shdr* section = elf_section(hdr, i);

		// Apply relocations if this is a relocation section
		if (swap_endian(section->sh_type) == SHT_RELA) {
			// Process each entry in the table
			for (size_t idx = 0; idx < swap_endian(section->sh_size) / swap_endian(section->sh_entsize); idx++) {
				Elf32_Rela* reltab = &(reinterpret_cast<Elf32_Rela *>(reinterpret_cast<ptrdiff_t>(hdr) + swap_endian(section->sh_offset)))[idx];
				Elf32_Shdr* target = elf_section(hdr, swap_endian(section->sh_info));
				uint32_t ref = swap_endian(target->sh_offset) + swap_endian(reltab->r_offset);
				uint32_t symval = elf_get_symval(hdr, swap_endian(section->sh_link), ELF32_R_SYM(swap_endian(reltab->r_info)));
				if (ELF32_R_TYPE(swap_endian(reltab->r_info)) != 0x03)
				{
					cerr << "Unknown relocation type." << endl;
					throw new exception;
				}
				if (reltab->r_addend != 0)
				{
					cerr << "Nonzero addend." << endl;
					throw new exception;
				}
				for (size_t i = 0; i < 4; symval >>= 8, ++i)
					_data[ref + 3 - i] |= symval & 0xff;
			}
		}
		else if (swap_endian(section->sh_type) == SHT_SYMTAB)
		{
			for (uint32_t symoff = 0; symoff < swap_endian(section->sh_size); symoff += swap_endian(section->sh_entsize))
			{
				Elf32_Sym* symbol = reinterpret_cast<Elf32_Sym*>(
					reinterpret_cast<ptrdiff_t>(hdr) + swap_endian(section->sh_offset) + symoff
				);
				char const* name = reinterpret_cast<char const *>(hdr) + swap_endian(strtab->sh_offset) + swap_endian(symbol->st_name);
				if (string(name) == "run")
					entrypoint = elf_get_symval(hdr, symbol);
			}
		}
	}
	return entrypoint;
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
	in.seekg(0, in.end);
	size_t length = in.tellg();
	in.seekg(0, in.beg);
	string data;
	data.resize(length);
	in.read(const_cast<char*>(data.data()), length);

	uint32_t entrypoint = link(data);

	run(data, entrypoint);

	return 0;
}
