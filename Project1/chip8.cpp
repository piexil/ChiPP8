#include "chip8.h"

#include <exception>

chip8::chip8(){
	init();
}
chip8::chip8(FILE* rom){
	chip8();
	load(rom);
}
void chip8::load(FILE* rom) {
	unsigned char buffer[3484];
	fread(buffer, 1, 3484, rom);
	for (int i = 0; i < 3484; i++) {
		memory[i + 512] = buffer[i];
	}
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
	opcode = memory[programCounter] << 8 | memory[programCounter + 1]; // opcode is a short so it's made up of two byutes from memory.
	int ret = 0;
	
	//decode & execution block
	switch (opcode & 0xF000) {
		case 0x0000:
			ret = processZero(opcode);
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
			stack[++sp] = programCounter + 2; //You don't want to save the current instruction 
			programCounter = opcode & 0x0FFF;
			break;
		case 0x3000:	// 	Skips the next instruction if VX equals NN.
			if (v[opcode & 0x0F00 >> 8] == (opcode & 0x00FF >> 4)) {
				programCounter += 2;
			}
			programCounter += 2;
			break;
		case 0x4000: // Skips the next instruction if VX doesn't equal NN.
			if (!(v[opcode & 0x0F00 >> 8] == (opcode & 0x00FF >> 4))) {
				programCounter += 2;
			}
			programCounter += 2;
			break;
		case 0x5000: // Skips the next instruction if VX equals VY
			if (v[opcode & 0x0F00 >> 8] == (v[opcode & 0x00F0 >> 4])) {
				programCounter += 2;
			}
			programCounter += 2;
			break;
		case 0x6000:
			v[opcode & 0x0F00 >> 8] = opcode & 0x0FF;
			programCounter = +2;
			break;
		case 0x7000:
			v[opcode & 0x0F00 >> 8] += opcode & 0x0FF;
			programCounter = +2;
			break;
		case 0x8000:
			aluOperation(opcode);
			break;
		case 0x9000: //SNE Skips the next instruction if VX equals VY
			if (v[opcode & 0x0F00 >> 8] != (v[opcode & 0x00F0 >> 4])) {
				programCounter += 2;
			}
			programCounter += 2;
			break;
		case 0xB000:
			programCounter = (opcode & 0x0FFF) + v[0];
			break;
		case 0xC000:
			temp = (opcode & 0x00FF >> 4) & (unsigned char)(rand() % 255);
			v[opcode & 0x0F00 >> 8] = temp;
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
					break;
				case 0x00A1: //Skips instruction if Vx isn't pressed
					if (key[v[(opcode & 0x0F00) >> 8]] == 0) {
						programCounter += 2;
					}
					break;
				default:
					std::cout << "Unknown E-type opcode: 0x" + opcode << std::endl;
					ret = 99;
			}
			programCounter += 2;
			break;
		case 0xF000:
			finstruction(opcode);
			programCounter += 2;
			break;
		default:
			std::cout << "Unknown opcode: 0x" + opcode << std::endl;
			ret = 99;
	}


	//decrement phase
	if (delay_timer > 0) {
		--delay_timer;
	}
	if (sound_timer > 0){
		if (sound_timer == 1)
			ret = 1;
		--sound_timer;
	}
	//return phase
	return ret;

}
void chip8::finstruction(unsigned short opcode) {
	switch (opcode & 0x00FF) {
		case 0x0007:
			v[(opcode & 0x0F00) >> 8] = delay_timer;
			break;
		case 0x000A:
			bool keyPress = false;
			for (int i = 0; i < 16; i++) {
				if (key[i] != 0) {
					v[opcode & 0x0F00 >> 8] = i;
					keyPress = true;
				}
			}
			if (!keyPress) {
				return;
			}
			break;
		case 0x0015:
			delay_timer = v[(opcode & 0x0F00) >> 8];
			break;
		case 0x0018:
			sound_timer = v[(opcode & 0x0F00) >> 8];
			break;
		case 0x001E:
			indexReg += v[(opcode & 0x0F00) >> 8];
			break;
		case 0x0033:
			memory[indexReg] = v[(opcode & 0x0F00) >> 8] / 100;
			memory[indexReg] = (v[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[indexReg] = (v[(opcode & 0x0F00) >> 8] % 100) % 10;
			break;
		case 0x0055:
			int indexOld = indexReg;
			for (int i = 0; i < (opcode & 0x0F00) >> 8; i++) {
				memory[indexReg++] = v[i];
			}
			indexReg = indexOld;
			break;
		case 0x0065:
			int indexOld = indexReg;
			for (int i = 0; i < (opcode & 0x0F00) >> 8; i++) {
				v[i] = memory[indexReg++];
			}
			indexReg = indexOld;
			break;
		default:
			std::cout << "Unknown opcode: 0x" + opcode << std::endl;
	}
}
void chip8::draw(unsigned short opcode) {
	unsigned char yReg = opcode & 0x00F0 >> 4;
	unsigned char xReg = opcode & 0x0F00 >> 8;
	unsigned char xCor = v[xReg];
	unsigned char yCor = v[yReg];
	unsigned short nBytes = opcode & 0x000F;
	unsigned short oldIndex = indexReg;
	unsigned short pixel;
	v[0xF] = 0;
	for (int yline = 0; yline < nBytes; yline++){
		pixel = memory[indexReg + yline];
		for (int xline = 0; xline < 8; xline++){
			if ((pixel & (0x80 >> xline)) != 0){
				if (gfx[(xCor + xline + ((yCor + yline) * 64))] == 1)
					v[0xF] = 1;
				gfx[xCor + xline + ((yCor + yline) * 64)] ^= 1;
			}
		}
	}
	indexReg = oldIndex;
}

void chip8::aluOperation(unsigned short opcode) {
	switch(opcode & 0x000F) {
		case 0x0000: // 8XY0
			v[opcode & 0x0F00 >> 8] = v[opcode & 0x00F0 >> 4];
			break;
		case 0x0001: // X = X or Y
			v[opcode & 0x0F00 >> 8] = v[opcode & 0x00F0 >> 4] | v[opcode & 0x0F00 >> 8];
			break;
		case 0x0002: // and
			v[opcode & 0x0F00 >> 8] = v[opcode & 0x00F0 >> 4] & v[opcode & 0x0F00 >> 8];
			break;
		case 0x0003: // xor
			v[opcode & 0x0F00 >> 8] = v[opcode & 0x00F0 >> 4] ^ v[opcode & 0x0F00 >> 8];
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
				v[0xF] = 1;
			else 
				v[0xF] = 0;			
			v[(opcode & 0x0F00) >> 8] -= v[(opcode & 0x00F0) >> 4];
			break;
		case 0x0006: // shift VX right by one. VF is set to least signifigant bit before shift
			v[0xF] = (v[(opcode & 0x0F00) >> 8] & 0x0F);
			v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x0F00) >> 8] >> 1;
			break;
		case 0x0007: // sub VX = VY - VX; VF is set to fals eon borrow
			if (v[(opcode & 0x00F0) >> 4] > v[(opcode & 0x0F00) >> 8])
				v[0xF] = 1;
			else
				v[0xF] = 0;
			v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x00F0) >> 4] - v[(opcode & 0x0F00) >> 8];
			break;
		case 0x000E: // shift VX left by 1. VF = most signifigant bit
			v[0xF] = (v[(opcode & 0x0F00) >> 8] & 0xF0) >> 4;
			v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x0F00) >> 8] << 1;
			break;
	}
	programCounter += 2;
}
int chip8::processZero(unsigned short opcode) {
	switch (opcode & 0x000F) { // assume 0x00EX
		case 0x0000: //CLS | 0x00E0
			programCounter += 2;
			return 2;// clears screen
		case 0x000E: //RET | 0x00EE
			programCounter = stack[sp--];
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
	//move the fontset into memory
	for(int i = 0; i < 80; i++) {
		 memory[i] = fontset[i];
	}
	//
}
bool chip8::drawFlag() {
	return true;
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
	free(memory);
	free(v);
	free(gfx);
	free(key);
	free(fontset);

}
