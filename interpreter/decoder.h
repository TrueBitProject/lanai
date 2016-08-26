#include <vector>
#include <array>

struct MachineState
{
	std::vector<uint32_t> memory;
	std::array<uint32_t, 32> registers{{
		0, uint32_t(-1), 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	}};

	uint32_t& pc = registers[2];
	uint32_t& sp = registers[4];
	uint32_t& fp = registers[5];
	uint32_t& rv = registers[8];
	uint32_t& rr1 = registers[10];
	uint32_t& rr2 = registers[11];
	uint32_t& rca = registers[15];
};
