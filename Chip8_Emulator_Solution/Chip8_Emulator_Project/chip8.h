#pragma once

#include <cstdint>
#include <random>

const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;
const unsigned int KEY_COUNT = 16;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int REGISTER_COUNT = 16;
const unsigned int STACK_LEVELS = 16;

class Chip8 
{
public:
	Chip8();
	void open_ROM(char const* file_name);
	void Cycle();
	uint8_t keypad[KEY_COUNT]{};
	//Monochrome Display Memory (64 pixels width, 32 pixels length) - Only 2 colors repersented
	uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};
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
	

	
	//Instruction for random number
	//Random number engine class which generates pseudo-random numbers
	//std::default_random_engine random_num_engine;
	uint8_t random_byte; //Produces random integer values uniformlly distributed on a closed interval [a,b]


	//Instruction Functions -- > in chip8.cpp
	void Table0();
	void Table8();
	void TableE();
	void TableF();

	// Do nothing
	void OP_NULL();

	// CLS
	void OP_00E0();

	// RET
	void OP_00EE();

	// JP address
	void OP_1nnn();

	// CALL address
	void OP_2nnn();

	// SE Vx, byte
	void OP_3xkk();

	// SNE Vx, byte
	void OP_4xkk();

	// SE Vx, Vy
	void OP_5xy0();

	// LD Vx, byte
	void OP_6xkk();

	// ADD Vx, byte
	void OP_7xkk();

	// LD Vx, Vy
	void OP_8xy0();

	// OR Vx, Vy
	void OP_8xy1();

	// AND Vx, Vy
	void OP_8xy2();

	// XOR Vx, Vy
	void OP_8xy3();

	// ADD Vx, Vy
	void OP_8xy4();

	// SUB Vx, Vy
	void OP_8xy5();

	// SHR Vx
	void OP_8xy6();

	// SUBN Vx, Vy
	void OP_8xy7();

	// SHL Vx
	void OP_8xyE();

	// SNE Vx, Vy
	void OP_9xy0();

	// LD I, address
	void OP_Annn();

	// JP V0, address
	void OP_Bnnn();

	// RND Vx, byte
	void OP_Cxkk();

	// DRW Vx, Vy, height
	void OP_Dxyn();

	// SKP Vx
	void OP_Ex9E();

	// SKNP Vx
	void OP_ExA1();

	// LD Vx, DT
	void OP_Fx07();

	// LD Vx, K
	void OP_Fx0A();

	// LD DT, Vx
	void OP_Fx15();

	// LD ST, Vx
	void OP_Fx18();

	// ADD I, Vx
	void OP_Fx1E();

	// LD F, Vx
	void OP_Fx29();

	// LD B, Vx
	void OP_Fx33();

	// LD [I], Vx
	void OP_Fx55();

	// LD Vx, [I]
	void OP_Fx65();


	//0x5 --> 15
	//0xE --> 14
	//Ox65 --> 101
	//Function pointer arrays
	typedef void (Chip8::*Chip8Func)();
	Chip8Func table[0xF + 1]{ &Chip8::OP_NULL };
	Chip8Func table0[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func table8[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func tableE[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func tableF[0x65 + 1]{ &Chip8::OP_NULL };

};