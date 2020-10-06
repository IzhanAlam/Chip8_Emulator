#include "chip8.h"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <random>

//FUnction to load the contents of a ROM file to save the instructions in memory

//Store insturctions to memory as stated in chip8.h (starts at 0x200)
const unsigned int start_mem = 0x200;

//A ROM expects 16 characters at a certain locaiton to write characters onto screen
// Putting these characters into memory

//Recall that a register is able to hold any value form 0x00 to 0xFF
//The characters are sprites -> Each sprite is 5 bytes
//There are 16 characters at 5 butes each -> Need an array of 80 bytes

const unsigned int num_of_fonts = 80;
//Fonts start in memmory at 0x50
const unsigned int font_start_mem = 0x50;


uint8_t fonts[num_of_fonts] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


//Initializer
Chip8::Chip8()
{
	//Chip8 memory startes form 0x200, the program counter registry is set at this
	//This is the first instruction to be executed
	program_counter = start_mem;

	//Load fonts into memory starting at 0x50
	for (unsigned int i = 0; i < num_of_fonts; ++i) 
	{
		memory[font_start_mem + i] = fonts[i];
	}


}

void Chip8::open_ROM(char const* file_name)
{
	//Need to open the file as binary
	//Move file pointer to the end
	std::ifstream file(file_name, std::ios::binary | std::ios::ate);


	FILE * po_File = fopen(file_name, "rb");
	//Standard read file
	if (file.is_open()) 
	{
		//Check file size, and buffer to hold contents:
		std::streampos size = file.tellg();
		char* buffer = new char[size];

		//Back to beginning to fill buffer
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		//Load ROM into chip8 memory starting at 0x200
		for (long i = 0; i < size; ++i)
		{
			memory[start_mem + i] = buffer[i];
		}

		delete[] buffer;
	}
}