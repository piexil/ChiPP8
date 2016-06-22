#pragma once
#include <iostream>
class chip8
{
public:
	chip8();
	chip8(FILE* rom);
	~chip8();
	int stepCycle();
	void init();
	bool drawFlag;
	unsigned char* getgfx();
	void setMem(int, unsigned char[]);
	void debugRender(void);
private:
	
	int ret;
	void finstruction(unsigned short);
	void load(FILE* rom);
	void aluOperation(unsigned short);
	int processZero(unsigned short);
	void draw(unsigned short);
	bool keyPress = false;
	unsigned int indexOld = 0;
	unsigned short opcode;
	unsigned char temp;
	unsigned char memory[4096];
	unsigned char v[16];
	unsigned short indexReg;
	unsigned short programCounter;
	unsigned char gfx[2048]; // 64x32 pixel screen
	unsigned char delay_timer;
	unsigned char sound_timer; 
	unsigned short stack[16];
	unsigned short sp;
	unsigned char key[16]; // the keypad
 //defualt fontset is hardcoded into the machine
};

