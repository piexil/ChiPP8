#pragma once
class chip8
{
public:
	chip8();
	~chip8();
	void stepCycle();
	void init();

private:
	unsigned short opcode;
	unsigned char memory[4096];
	unsigned char v[16];
	unsigned short indexReg;
	unsigned short programCounter;
	unsigned char gfx[2048]; // 64x32 pixel screen
	unsigned char delay_timer;
	unsigned char sound_timer; 
	unsigned char key[16]; // the keypad
};

