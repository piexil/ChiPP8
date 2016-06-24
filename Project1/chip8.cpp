#include "chip8.h"
#include <windows.h>
#include <exception>
unsigned char fontset[80] = {
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
chip8::chip8(){
	init();
}
chip8::chip8(FILE* rom){
	init();
	load(rom);
}
void chip8::load(FILE* rom) {
	printf("loading rom.");
	fseek(rom, 0, SEEK_END);
	long lSize = ftell(rom);
	rewind(rom);
	char* buffer = (char *)malloc(sizeof(char) * lSize);
	if (buffer == NULL)
	{
		printf("\nMemory error");
		return;
	}
	size_t result = fread(buffer, 1, lSize, rom);
	if (result != lSize)
	{
		printf("\nReading error");
		return;
	}
	for (int i = 0; i < lSize; ++i) {
		memory[i + 512] = buffer[i];
		printf(".");
		Sleep(1);
	}
	printf("\n");
	free(buffer);
}
/*
	this function will process a single cycle of the chip8 computer 
	0 - nothing
	1 - cause a "beep" to be produced
	2 - clear the screen
	99 - error 
*/
int chip8::stepCycle() {
	//fetch block
	opcode = (memory[programCounter] << 8 | memory[programCounter + 1]); // opcode is a short so it's made up of two byutes from memory.
	ret = 0;
	printf("Executing: 0x%x @ Loc: %x & %x\n", opcode, programCounter, programCounter + 1);
	Sleep(100);
	//decode & execution block
	switch (opcode & 0xF000) {
		case 0x0000:
			ret = processZero(opcode);
			if (ret == 2) {
				for (int i = 0; i < 2048; i++) {
					gfx[i] = 0x0;
				}
			}
			programCounter += 2;
			break;
		case 0xA000:	//ANNN
			indexReg = opcode & 0x0FFF;
			programCounter += 2;
			break;
		case 0x1000:	//JP NNN
			programCounter = opcode & 0x0FFF;
			break;
		case 0x2000:	//CALL NNN
			stack[sp] = programCounter; //You don't want to save the current instruction 
			sp++;
			programCounter = opcode & 0x0FFF;
			break;
		case 0x3000:	// 	Skips the next instruction if VX equals NN.
			if (v[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
				programCounter += 2;
			}
			programCounter += 2;
			break;
		case 0x4000: // Skips the next instruction if VX doesn't equal NN.
			if ((v[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))) {
				programCounter += 2;
			}
			programCounter += 2;
			break;
		case 0x5000: // Skips the next instruction if VX equals VY
			if (v[(opcode & 0x0F00) >> 8] == (v[(opcode & 0x00F0) >> 4])) {
				programCounter += 2;
			}
			programCounter += 2;
			break;
		case 0x6000: //sets Vx to NN
			v[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			programCounter += 2;
			break;
		case 0x7000:
			v[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
			programCounter += 2;
			break;
		case 0x8000:
			aluOperation(opcode);
			programCounter += 2;
			break;
		case 0x9000: //SNE Skips the next instruction if VX ! equals VY
			if (v[(opcode & 0x0F00) >> 8] != (v[(opcode & 0x00F0) >> 4])) {
				programCounter += 2;
			}
			programCounter += 2;
			break;
		case 0xB000:
			programCounter = (opcode & 0x0FFF) + v[0];
			break;
		case 0xC000:
			v[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF) & (unsigned char)(rand() % 0xFF);
			programCounter += 2;
			break;
		case 0xD000: //Draw
			draw(opcode);
			programCounter += 2;
			break;
		case 0xE000:
			switch (opcode & 0x00FF) {
				case 0x009E: //Skips next instruction if Vx is pressed
					if (key[v[(opcode & 0x0F00) >> 8]] != 0) {
						programCounter += 2;
					}
					programCounter += 2;
					break;
				case 0x00A1: //Skips instruction if Vx isn't pressed
					if (key[v[(opcode & 0x0F00) >> 8]] == 0) {
						programCounter += 2;
					}
					programCounter += 2;
					break;
				default:
					
					ret = 99;
					break;
			}
			break;
		case 0xF000:
			finstruction(opcode);
			programCounter += 2;
			break;
		default:
			printf("Unknown opcode: %x\n", opcode);
			ret = 99;
	}


	//decrement phase
	if (delay_timer > 0) {
		--delay_timer;
	}
	if (sound_timer > 0) {
		if (sound_timer == 1){
			ret = 1;
		printf("BEEP!\n");
		}
		--sound_timer;
	}
	//return phase
	return ret;

}
void chip8::debugRender()
{
	// Draw
	for (int y = 0; y < 32; ++y)
	{
		for (int x = 0; x < 64; ++x)
		{
			if (gfx[(y * 64) + x] == 'Ì')
				printf("O");
			else
				printf(" ");
		}
		printf("\n");
	}
	printf("\n");
}
void chip8::finstruction(unsigned short opcode) {
	bool keyPress = false;
	switch (opcode & 0x00FF) {
		
	case 0x0007:
		v[(opcode & 0x0F00) >> 8] = delay_timer;
		break;
	case 0x000A:
		keyPress = false;

		for (int i = 0; i < 16; ++i)
		{
			if (key[i] != 0)
			{
				v[(opcode & 0x0F00) >> 8] = i;
				keyPress = true;
			}
		}

		// If we didn't received a keypress, skip this cycle and try again.
		if (!keyPress)
			return;

		break;
	case 0x0015:
		delay_timer = v[(opcode & 0x0F00) >> 8];
		break;
	case 0x0018:
		sound_timer = v[(opcode & 0x0F00) >> 8];
		break;
	case 0x001E:
		if (indexReg + v[(opcode & 0x0F00) >> 8] > 0xFFF) {	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
			v[0xF] = 1;
		}
		else {
			v[0xF] = 0;
		}
		indexReg += v[(opcode & 0x0F00) >> 8];
		break;
	case 0x0029:
			indexReg = v[(opcode & 0x0F00) >> 8] * 0x5;
			break;
		case 0x0033:
			memory[indexReg] = v[(opcode & 0x0F00) >> 8] / 100;
			memory[indexReg] = (v[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[indexReg] = (v[(opcode & 0x0F00) >> 8] % 100) % 10;
			break;
		case 0x0055:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				memory[indexReg + i] = v[i];
			indexReg += ((opcode & 0x0F00) >> 8) + 1;
			break;
		case 0x0065:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				v[i] = memory[indexReg + i];
			indexReg += ((opcode & 0x0F00) >> 8) + 1;
			break;
		default:
			printf("Unknown opcode: %x\n", opcode);
			break;
	}
}
void chip8::draw(unsigned short opcode) {
	unsigned short x = v[(opcode & 0x0F00) >> 8];
	unsigned short y = v[(opcode & 0x00F0) >> 4];
	unsigned short height = opcode & 0x000F;
	unsigned short pixel;

	v[0xF] = 0;
	for (int yline = 0; yline < height; yline++)
	{
		pixel = memory[indexReg + yline];
		for (int xline = 0; xline < 8; xline++)
		{
			if ((pixel & (0x80 >> xline)) != 0)
			{
				if (gfx[(x + xline + ((y + yline) * 64))] == 1)
				{
					v[0xF] = 1;
				}
				gfx[x + xline + ((y + yline) * 64)] ^= 1;
			}
		}
	}

	drawFlag = true;
	
}

void chip8::aluOperation(unsigned short opcode) {
	switch(opcode & 0x000F) {
		case 0x0000: // 8XY0
			v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x00F0) >> 4];
			break;
		case 0x0001: // X = X or Y
			v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x00F0) >> 4] | v[(opcode & 0x0F00) >> 8];
			break;
		case 0x0002: // and
			v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x00F0) >> 4] & v[(opcode & 0x0F00) >> 8];
			break;
		case 0x0003: // xor
			v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x00F0) >> 4] ^ v[(opcode & 0x0F00) >> 8];
			break;
		case 0x0004: // add. VF is set to true if carry
			if (v[(opcode & 0x00F0) >> 4] > (0xFF - v[(opcode & 0x0F00) >> 8]))
				v[0xF] = 1; //carry
			else
				v[0xF] = 0;
			v[(opcode & 0x0F00) >> 8] += v[(opcode & 0x00F0) >> 4];
			break;
		case 0x0005: // sub.  Vx = Vx - Vy. VF is set to false on "borrow"
			if (v[(opcode & 0x0F00) >> 8] > v[(opcode & 0x00F0) >> 4]) 
				v[0xF] = 0;
			else 
				v[0xF] = 1;			
			v[(opcode & 0x0F00) >> 8] -= v[(opcode & 0x00F0) >> 4];
			break;
		case 0x0006: // shift VX right by one. VF is set to least signifigant bit before shift
			v[0xF] = (v[(opcode & 0x0F00) >> 8] & 0x1);
			v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x0F00) >> 8] >> 1;
			break;
		case 0x0007: // sub VX = VY - VX; VF is set to fals eon borrow
			if (v[(opcode & 0x00F0) >> 4] > v[(opcode & 0x0F00) >> 8])
				v[0xF] = 0;
			else
				v[0xF] = 1;
			v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x00F0) >> 4] - v[(opcode & 0x0F00) >> 8];
			break;
		case 0x000E: // shift VX left by 1. VF = most signifigant bit
			v[0xF] = (v[(opcode & 0x0F00) >> 8]) >> 7;
			v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x0F00) >> 8] << 1;
			break;
	}
	
}
int chip8::processZero(unsigned short opcode) {
	switch (opcode & 0x000F) { // assume 0x00EX
		case 0x0000: //CLS | 0x00E0
			
			return 2;// clears screen
		case 0x000E: //RET | 0x00EE
			sp--;
			programCounter = stack[sp];
			break;

		default:
			std::cout << "Unknown opcode [0x0000]: 0x" + opcode << std::endl;
			return 99;
	}
	return 0;
}
void chip8::init() {
	programCounter = 0x200; //All chip8 applications are expected to have an entry point here. 
	opcode = 0;
	indexReg = 0;
	sp = 0;
	//zero registers
	for (int i = 0; i < 16; i++) {
		v[i] = 0;
	}
	for (int i = 0; i < 4095; i++) {
		memory[i] = 0;
	}
	//move the fontset into memory
	printf("loading fontset..........\n");
	for(int i = 0; i < 80; i++) {
		 memory[i] = fontset[i];
		 printf("%x %x\n", fontset[i], memory[i]);
	}
	//
}

	

void chip8::setMem(int location, unsigned char newMem[]) {
	if (sizeof(newMem) > 4095 - location) {
		throw std::invalid_argument("New memeory is more than max allocation amount. Memory not set");
		return;
	}
	for (int i = 0; i < sizeof(newMem); i++) {
		memory[location++] = newMem[i];
	}
}
unsigned char* chip8::getgfx() {
	return gfx;
}
chip8::~chip8(){
}
