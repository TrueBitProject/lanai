#include <vector>
#include <array>

struct MachineState
{
	std::string memory;
	std::array<uint32_t, 30> registers{{
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	}};

	uint32_t& pc = registers[0];
	uint32_t& sp = registers[2];
	uint32_t& fp = registers[3];
	uint32_t& rv = registers[6];
	uint32_t& rr1 = registers[8];
	uint32_t& rr2 = registers[9];
	uint32_t& rca = registers[13];
	uint32_t pcDelay[3] = {uint32_t(-1), uint32_t(-1), uint32_t(-1)};
	bool zero = false;
	bool negative = false;
	bool overflow = false;
	bool carry = false;

	// Calling conventions:
	// memory at fp contains return address, r6 holds first argument, rv/r8 receives return value
};

class Interpreter
{
public:
	Interpreter(std::string const& code): code(code) {}

	uint32_t run(uint32_t entrypoint, bool verbose = false);

private:
	void initState(uint32_t entrypoint, uint32_t exitpoint, uint32_t memsize);

	void updatePC();

	void printState();
	void printMemory();

	uint32_t currentInstruction() const;

	bool condition(uint16_t condition, bool invert) const;
	uint32_t operation(uint16_t op, uint32_t a, uint32_t b, bool updateFlags, bool arithmeticShift = false);

	static std::string operationName(uint16_t op);

	/// @returns the current value of the register i
	uint32_t reg(uint16_t i) const;
	void regStore(uint16_t i, uint32_t value, unsigned delayForPC);
	uint32_t pc() const { return reg(2); }

	static bool isPC(uint16_t i) { return i == 2; }

	static uint32_t readWord(std::string const& data, uint32_t pos);
	static uint32_t readByte(std::string const& data, uint32_t pos);

	uint32_t memWord(uint32_t pos);
	uint32_t memByte(uint32_t pos);

	void memStoreWord(uint32_t pos, uint32_t value);
	void memStoreByte(uint32_t pos, uint32_t value);

	uint32_t steps = 0;

	MachineState state;

	std::string const code;
};
