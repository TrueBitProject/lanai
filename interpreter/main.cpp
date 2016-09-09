#include <fstream>
#include <climits>
#include <iostream>
#include <iomanip>
#include <string>

#include "interpreter.h"
#include "linker.h"

using namespace std;

string readfile(string const& file)
{
	ifstream in(file, std::ifstream::binary);
	if (!in)
		throw new exception;

	in.seekg(0, in.end);
	size_t length = in.tellg();
	in.seekg(0, in.beg);

	string data;
	data.resize(length);
	in.read(const_cast<char*>(data.data()), length);

	return data;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "Usage: " << argv[0] << " <file.o>" << endl;
		cout << "  Runs the function called \"run\" in the specified lanai object file." << endl;
		return 1;
	}
	string data = readfile(argv[1]);

	uint32_t entrypoint = Linker::link(data);
	cout << "Linked code: ";
	for (uint32_t i = 0; i < data.size(); ++i)
		cout << hex << setfill('0') << setw(2) << unsigned(uint8_t(data[i]));
	cout << endl;
	cout << "Entrypoint: 0x" << hex << entrypoint << endl;

	Interpreter interpreter(data);
	interpreter.run(entrypoint, 0x10000, false);

	return 0;
}
