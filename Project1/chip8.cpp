#include "chip8.h"
#include <iostream>


chip8::chip8()
{
}

/*
	this function will return what the system will need to process. 
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
			break;
		case 0xA000:	//ANNN
			indexReg = opcode & 0x0FFF;
			programCounter += 2;
			break;


		default:
			std::cout << "Unknown opcode: 0x" + opcode << std::endl;
			ret = 99;
	}


	//decrement phase
	if (delay_timer > 0)
		--delay_timer;

	if (sound_timer > 0)
	{
		if (sound_timer == 1)
			ret = 1;
		--sound_timer;
	}

	//return phase
	return ret;


}
int chip8::processZero(unsigned short opcode) {
	switch (opcode & 0x000F) {
		case 0x0000: //CLS | 0x00E0
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

}


chip8::~chip8()
{
}
