#include "step68k.h"

Step68k::Step68k()
    : Core_68k()
{
    errorCounter = 0;
    testCounter = 0;
}

void Step68k::logInstruction(u16 word, bool isNext) {
	if (debug) {
		cout << "Logging: 0x" << std::hex << word << " is next: "
		     << isNext << endl;
	}
}

void Step68k::setObjectFile(string file) {
	insFile.open(file, ios::in | ios::binary);		
};


bool Step68k::readWord(uint16_t &word) {
	if (!insFile.is_open()) {
	 	return false;
	}
	/* Streams are worthless */
	char cWord[2];
	
	while (insFile.read(cWord, 2)) {
		uint8_t low = cWord[1];
		uint8_t high = cWord[0];
		word = low | (high << 8);
		if (debug) {
			cout << "Read low: 0x" << std::hex << low
			     << " high: 0x" << std::hex << high;
			cout << " Converted to word: 0x" << std::hex << word << endl;
		}
		cWord[0] = '\0';
		cWord[1] = '\0';		
		return true;
	}
	return false;
}

void Step68k::runSim() {
	setUp();
	uint16_t word;
	while (readWord(word)) {
		addWord(word);
	}
	power();

	while(getPC() != 0 && getOP() != 0) {
		process();
	}

}


void Step68k::printErrorCounter() {
	cout << endl << "Errors: " + Helper::convertIntToString(errorCounter)
	     << endl << endl;
}

void Step68k::setUp() {
	irqSampleCycle = 0;
	illegalOpcode = false;
	privilege = false;
	addressError = false;
	busError = false;
	group2exception = false;
	memoryblock.init();
	cycleCounter = -40;
	adrCounter = 8;
	memoryblock.write(7, 8);
}

u16 Step68k::memWordRead(u32 addr) {
	u16 res = memRead(addr) << 8;
	res |= memRead(addr + 1);
	return res;
}

void Step68k::memWordWrite(u32 addr, u16 data) {
	memWrite(addr, data >> 8);
	memWrite(addr + 1, data & 0xff);
}

u8 Step68k::memRead(u32 addr) {
	if (memoryblock.isBusError(addr)) {
		raiseBusError(addr);
	}
	return memoryblock.read(addr);
}

void Step68k::memWrite(u32 addr, u8 data) {
    if (memoryblock.isBusError(addr)) {
        raiseBusError(addr);
    }
    memoryblock.write(addr, data);
}

unsigned Step68k::getEA(ADM _adm, unsigned reg) {
	if ((_adm >> 3) == 7) {
		return _adm;
	} else {
		return _adm | reg;
	}
}

void Step68k::check(string ident) {
	Results* oSampled = Results::getResult(ident);

	if (oSampled == 0) {
		throw Exception(" ident not found: " + ident);
	}

	Results* oCalced = sampleResult();

	if (!Results::compareContent(oSampled, oCalced)) {
		cout << ident + " -> error: expected " + oSampled->getError()
			+ oCalced->getError(true) << endl;
		errorCounter++;
	} else {
		cout << ident + " -> success " << endl;
	}

	if (testCounter++ > 100) {
		testCounter = 0;
		cout << endl;
		string nothing;
		cin >> nothing;
	}
}

Results* Step68k::sampleResult() {
	Results* oCalced = new Results("", false);
	for (unsigned i=0; i < 8; i++) {
		oCalced->setRegA(i, getRegA(i));
		oCalced->setRegD(i, getRegD(i));
	}
	oCalced->setRegS(getSR());
	oCalced->setRegIRD(getRegIrd());
	oCalced->setCycleCount(cycleCounter);
	oCalced->setIrqSampleCyclePos(irqSampleCycle);
	oCalced->expectIllegal(illegalOpcode);
	oCalced->expectPrivilege(privilege);
	oCalced->expectAddressError(addressError);
	oCalced->expectBusError(busError);
	oCalced->expectGroup2Exception(group2exception);
	oCalced->setCodeBlock(&memoryblock);
	return oCalced;
}

void Step68k::sampleIrq() {
	irqSampleCycle = cycleCounter;
	Core_68k::sampleIrq();
}

void Step68k::op_illegal(u16 opcode) {
	illegalOpcode = true;
	Core_68k::op_illegal(opcode);
}

void Step68k::setPrivilegeException() {
	privilege = true;
	Core_68k::setPrivilegeException();
}

void Step68k::group0exception(u8 type) {
	if (type == ADDRESS_ERROR) {
		addressError = true;
	} else {
		busError = true;
	}
	Core_68k::group0exception(type);
}

void Step68k::trapException(u8 vector) {
	group2exception = true;
	Core_68k::trapException(vector);
}
