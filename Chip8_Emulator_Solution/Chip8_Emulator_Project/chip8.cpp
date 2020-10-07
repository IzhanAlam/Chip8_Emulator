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
//There are 16 characters at 5 bytes each -> Need an array of 80 bytes

const unsigned int num_of_fonts = 80;
//Fonts start in memory at 0x50
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
	: random_num_engine(std::chrono::system_clock::now().time_since_epoch().count()) //Use the sustem clock for the random number engine
{
	//Chip8 memory startes form 0x200, the program counter registry is set at this
	//This is the first instruction to be executed
	program_counter = start_mem;

	//Load fonts into memory starting at 0x50
	for (unsigned int i = 0; i < num_of_fonts; ++i) 
	{
		memory[font_start_mem + i] = fonts[i];
	}

	//An instrucvtion which places a random number to a rtegister
	//In this case, the system clock can be used to generate the random number

	//Generates random number between 0 and 255 --> stored as random_byte
	random_byte = std::uniform_int_distribution<uint8_t>(0, 255U); //Initalize RNG


}

//The chip8 emulator has 34 instructions to emulate : http://mattmik.com/files/chip8/mastering/chip8.html

//Code 00E0: CLS: Clear the display --> Set the entire video buffer to zeroes
//Another way to do this is to use a for loop to iterate to the end of array and set to 0
void Chip8::OP_00E0()//CLS
{
	memset(video, 0, sizeof(video));
}

// 00EE: RET: return from a subroutine --> top of stack has adrerss of one instruction past the one that calls the subroutine
//Put that instruction back into the program counter
void Chip8::OP_00EE() //RET
{
	//decrement stack pointer
	--stack_pointer;
	program_counter = stack[stack_pointer];
}

//1nnn: Jumps to a location nnn. The interpreter sets the program counter to nnn
//No stack interaction is needed as a jump does not recall the origin.
void Chip8::OP_1nnn() // JP addr
{
	uint16_t address = opcode & 0x0FFFu;
	program_counter = address;
}

//2nnn: Call subroutine at nnnn. 
//When subroutine is called, the current Program counter is put on the top of the stack
//
void Chip8::OP_2nnn() //Call addr
{
	uint16_t address = opcode & 0x0FFFu;

	stack[stack_pointer] = program_counter;
	++stack_pointer;
	program_counter = address;
}

//3xkk: To skip next instruction if Vx = kk 
//Since the program counter has been incremented by 2 in cycle, an increment of 2 will skip the next instruction
void Chip8::OP_3xkk()  // SE Vx, byte
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] == byte) 
	{
		program_counter += 2;
	}

}

//4xkk: Skip next instruction if Vx != kk
void Chip8::OP_4xkk()  // SNE Vx, byte
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] != byte) 
	{
		program_counter += 2;
	}

}


//5xy0 - SE Vx, Vy
// This is to skip the next instruction if Vx = Vy
void Chip8::OP_5xy0() // SE Vx, Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] == registers[Vy]) 
	{
		program_counter += 2;
	}

}

//6xkk: Set Vk to be equal to kk (byte)
void Chip8::OP_6xkk() // LD VX, byte
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = byte;
}

//7xkk: Set Vx = Vx + kk
void Chip8::OP_7xkk() //ADD Vx, byte
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] += byte;
}

//8xy0: Set Vx = Vy
void Chip8::OP_8xy0() //Add Vx, byte
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] = registers[Vy];
}

//8xy1: Set Vx to Vy or Vx
void Chip8::OP_8xy1() // OR Vx,Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
}

//8xy2: Set Vx = Vx and Vy
void Chip8::OP_8xy2() // AND Vx,Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] &= registers[Vy];
}

//8xy3: Set Vx = Vx XOR Vy
void Chip8::OP_8xy3() 
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] ^= registers[Vy];
}

//8xy4: Set Vx = Vx+Vy, set VF = carry
// Vx and Vy is added together, but if the sum is > 9 bits (255), VF is then set to 1
//Otherwise it is set to 0. Lowest 8 bits of the sum are kept and stored in Vx
//ADD with overflow flag
void Chip8::OP_8xy4()  // ADD Vx,Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = registers[Vx] + registers[Vy];

	if (sum > 255U)
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] = sum & 0xFFu;
}

//8xy5: Set Vx = Vx - Vy
// VF = Not borrow
// Vx > Vy, then VF = 1, else 0. Vy is then subtracted from Vx and stored in Vx
void Chip8::OP_8xy5() // SUB Vx, Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] > registers[Vy])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] -= registers[Vy];
}

//8xy6: Set Vx = Vx SHR 1 -> if least-sig bit of Vx = 1, then VF = 1, else 0. Then divide Vx by 2
void Chip8::OP_8xy6() //SHR Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	registers[0xF] = (registers[Vx] & 0x1u);
	registers[Vx] >>= 1;
}

//8xy7: Set Vy - Vx, VF = Not Borrow
// In the case Vy > Vx, then VF = 1, else 0
//Then Vx is sibtracted from Vy, results stored in Vx
void Chip8::OP_8xy7() //SUBN Vx,Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vy] > registers[Vx])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] = registers[Vy] - registers[Vx];
}

//8xyE: If most significant but of Vx = 1, then VF = 1, else 0
//Then Vx is multiplied by 2
//Left shift performed (*2) and most significant bit saved in register VF
void Chip8::OP_8xyE()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save MSB in VF
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

	registers[Vx] <<= 1;
}

//9xy0 : Skip the next instruction if Vx != Vy
// Increment by 2 to skip
void Chip8::OP_9xy0()//SNE Vx, Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}
}

//Annn: Set I = nnn
void Chip8::OP_Annn() //LD I, addr
{
	uint16_t address = opcode & 0x0FFFu;

	index = address;
}

//Bnnn: jump to location nnn + V0
void Chip8::OP_Bnn() // JP V0, addr
{
	uint16_t address = opcode & 0x0FFFu;

	program_counter = registers[0] + address;
}

//Cxkk: Set Vx to a random byte and kk
void Chip8::OP_Cxkk() //RND Vx, byte
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = random_byte(random_num_engine) & byte;
}


//Dxyn: To display a sprite starting at memory location I at Vx,Vy, where Vf is collision
//Iterated over sprite row by row and column by column. 
//AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHJ
void Chip8::OP_Dxyn()
{


	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	//
	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = memory[index + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			// Sprite pixel is on
			if (spritePixel)
			{
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

//Ex9E: Skips the next instruction if key with the value of Vx is pressed
//increment by 2 to skip instruction

void Chip8::OP_Ex9E() //SKP Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (keypad[key])
	{
		program_counter += 2;
	}
}

//ExA1: Skip next instruction if key witht he value of Vx is not pressed
//increment by 2
void Chip8::OP_ExA1() //SKNP Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (!keypad[key])
	{
		program_counter += 2;
	}
}

//Fx07: Set Vx to the delay timer value
void Chip8::OP_Fx07() //LD Vx, DT
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] = delayTimer;
}

//Fx0A: Wait for key press, and store the value of the key press into Vx
//decrement the program counter by 2 whenever a keypad value is not detected

void Chip8::OP_Fx0A() //LD Vx, K
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;


	if (keypad[0])
	{
		registers[Vx] = 0;
	}
	else if (keypad[1])
	{
		registers[Vx] = 1;
	}
	else if (keypad[2])
	{
		registers[Vx] = 2;
	}
	else if (keypad[3])
	{
		registers[Vx] = 3;
	}
	else if (keypad[4])
	{
		registers[Vx] = 4;
	}
	else if (keypad[5])
	{
		registers[Vx] = 5;
	}
	else if (keypad[6])
	{
		registers[Vx] = 6;
	}
	else if (keypad[7])
	{
		registers[Vx] = 7;
	}
	else if (keypad[8])
	{
		registers[Vx] = 8;
	}
	else if (keypad[9])
	{
		registers[Vx] = 9;
	}
	else if (keypad[10])
	{
		registers[Vx] = 10;
	}
	else if (keypad[11])
	{
		registers[Vx] = 11;
	}
	else if (keypad[12])
	{
		registers[Vx] = 12;
	}
	else if (keypad[13])
	{
		registers[Vx] = 13;
	}
	else if (keypad[14])
	{
		registers[Vx] = 14;
	}
	else if (keypad[15])
	{
		registers[Vx] = 15;
	}
	else
	{
		program_counter -= 2;
	}
}

//Fx15: Set the delay timer to be equal to Vx
void Chip8::OP_Fx15() //LD DT, Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	delay_timer = registers[Vx];
}

//Fx18: set sound timer to be equal to Vx
void Chip8::OP_Fx18() //LD ST, Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	sound_timer = registers[Vx];
}

//Fx1E: Set I = I + Vx
void Chip8::OP_Fx1E() //ADD I, Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	index_register += registers[Vx];
}

//Fx29: Set I to the location of sprite for Vx
//FOnt characters are located at 0x50 that are 5 bytes each
//the address of the first byte of any character can be obtained by taking the offset from the start address
void Chip8::OP_Fx29() //LD F, Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];

	index_register = FONTSET_START_ADDRESS + (5 * digit);
}

//Fx33:Store the BCD repersentation of Vx in memory locations at i, i+1, and i+2
//interpretor takes decimal value of Vx:
//Hundreds digit: In memory at location i
//Tens Digit: In memory at locaiton i + 1
//Ones Digit: In memory at locaiton i + 2
void Chip8::OP_Fx33() //LD B, Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];

	memory[index + 2] = value % 10;
	value /= 10;

	memory[index + 1] = value % 10;
	value /= 10;

	memory[index] = value % 10;
}

//Fx55: Stores registers V0 through Vx in memory starting at location I
void Chip8::OP_Fx55() //LD[i], Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		memory[index + i] = registers[i];
	}
}

//Fx65: Read registers V0 through Vx from memory starting at location i
void Chip8::OP_Fx65() //LD VX, [I]
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[index + i];
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