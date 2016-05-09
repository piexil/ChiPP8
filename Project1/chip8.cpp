#include "chip8.h"



chip8::chip8()
{
}

void chip8::stepCycle() {
	//fetch block
	opcode = memory[programCounter] << 8 | memory[programCounter + 1]; // opcode is a short so it's made up of two byutes from memory.
	//decode & execution block
	switch (opcode & 0xF000) {

	}


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
