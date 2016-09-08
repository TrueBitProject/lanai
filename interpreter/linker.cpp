#include <fstream>
#include <iostream>
#include <climits>
#include <string>
#include <exception>
#include <functional>
#include <map>

#include <elf.h>

#include "linker.h"

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

uint32_t Linker::link(string& _data)
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
					//throw new exception;
				}
				if (reltab->r_addend != 0)
				{
					cerr << "Nonzero addend." << endl;
					//throw new exception;
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
				if (string(name) == "main")
					entrypoint = elf_get_symval(hdr, symbol);
			}
		}
	}
	return entrypoint;
}
