#pragma once

#include <cstdint>
#include <random>

const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

class Chip8 
{
public:
	Chip8();
	void open_ROM(char const* file_name);
private:
	/* 
	- A chip8 emulator will have 16 8-bit registers labelled V0 to VF
	- A register can hold a value from 0x00 to 0xFF

	- There are 4096 bytes of memory, which means memory address is stored from 0x000 to 0xFFF
	- 0x000 - 0x1FF -> reserved for chip8 interperator
	- 0x050 - 0x0A0 -> storage space for 16 built-in characters (0 to F)
	- 0x200 - 0xFFF -> instructions from the ROM stored

	*/

	//Number of registers
	uint8_t registers[16]{};
	//Memory size (bytes) of emulator
	uint8_t memory[4096]{};
	//Timer for timing, when 0, it stays 0. When given a value, it decrements at 60Hz
	uint8_t delay_timer{};
	//Sound timer for sound (same as delay_timer).
	uint8_t sound_timer{};
	//Stack pointer shows where in the 16-levels of stack the most recent value was placed
	uint8_t stack_pointer{};

	//Index register is used to store memory addresses for operations
	uint16_t index_register{};
	//Program Counter register holds the address of the next instruction to execute
	uint16_t program_counter{};
	// opcode is two bytes, where memory is addressed as a single byte,
	// But it needs to fetch the byte from program counter, and program counter + 1 (next instruction)
	uint16_t opcode{};
	//16-level stack is a way for CPU to keep track of the order of execution when it calls functions
	uint16_t stack[16]{};
	
	//Monochrome Display Memory (64 pixels width, 32 pixels length) - Only 2 colors repersented
	uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};
	
	//Instruction for random number
	//Random number engine class which generates pseudo-random numbers
	std::default_random_engine random_num_engine;
	std::uniform_int_distribution<uint8_t> random_byte; //Produces random integer values uniformlly distributed on a closed interval [a,b]


	//Instruction Functions -- > in chip8.cpp
	void OP_00E0();
	void OP_00EE();
	void OP_1nnn();
	void OP_2nnn();
	void OP_3xkk();
	void OP_4xkk();
	void OP_5xy0();
};