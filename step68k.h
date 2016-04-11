#ifndef STEP68K_H
#define STEP68K_H

#include <cstdint>
#include <fstream>
#include "results.h"
#include "68k/68k.h"
#include "memoryblock.h"

class Results;

class Step68k : public Core_68k {
 private:
	ifstream insFile;
	bool debug = true;
 protected:
	MemoryBlock memoryblock;

	int cycleCounter;
	unsigned adrCounter;

	void sync(u8 cycles) { cycleCounter += cycles; }
	unsigned getUserVector( u8 level ) { return 0 ; }
	void cpuHalted() {}
	void sampleIrq();
	void op_illegal(u16 opcode);
	void setPrivilegeException();
	unsigned irqSampleCycle;
	void group0exception(u8 type);
	void trapException(u8 vector);
	bool illegalOpcode;
	bool privilege;
	bool addressError;
	bool busError;
	bool group2exception;
	unsigned errorCounter;
	unsigned testCounter;

	u8 memRead(u32 addr);
	void memWrite(u32 addr, u8 data);

	u16 memWordRead(u32 addr);
	void memWordWrite(u32 addr, u16 data);

	void setUp();
	void check(string ident);
	Results* sampleResult();
	unsigned getEA(ADM _adm, unsigned reg = 0);

	void addWord(u16 _word) {
		memoryblock.write(adrCounter++, _word >> 8);
		memoryblock.write(adrCounter++, _word & 0xff);
	}
	void logInstruction(u16 word, bool isNext);
 public:
	Step68k();
	void setObjectFile(string file);
	bool readWord(uint16_t &word);
	void runSim();
	void printErrorCounter();
};

#endif /* STEP68K_H */
