#ifndef PIPELINE
#define PIPELINE

#include <cstdint>

enum PIPE_STAGE {
	PS_FETCH,
	PS_DECODE,
	PS_EXECUTE,
	PS_WRITE,
	PS_STAGE_MAX
};

enum REG_SET {
	REG_A,
	REG_D,
	REG_MAX
};


class instruction {
public:
	instruction() {
		reset();
	}
	bool flushes = false; /* When true, the instruction flushes the pipeline */

	void setOpCode(uint16_t op_code) {
		ins_opcode = op_code;
		unset = false;
	}
	uint16_t getOpCode() {
		return ins_opcode;
	}

	/**
	 * Sets the number of cycles required for a stage of the
	 * pipeline
	 * @param[in] stage the stage 
	 * @param[in] cycles the number of cycles required in that
	 * stage
	 */
	void setCycles(PIPE_STAGE stage, uint16_t c) {
		cycles[stage] = c;
	}
	/**
	 * Gets the remaining cycles required for a stage of the
	 * pipeline
	 *
	 * @param[in] stage the stage 
	 * @return the number of remaining cycles
	 */
	uint16_t getCycles(PIPE_STAGE stage) {
		return cycles[stage];
	}

	/**
	 * Reducs the remaining execution cycles required for the stage
	 *
	 * @param[in] stage the stage
	 * @param[in] cycles the number of cycles being 'executed'
	 *
	 * @return the number of remaining cycles
	 */
	uint16_t execCycles(PIPE_STAGE stage, uint16_t c) {
		if (cycles[stage] < c) {
			cycles[stage] = 0;
		} else {
			cycles[stage] -= c;
		}
		return cycles[stage];
	}

	/**
	 * Indicates which registers are read during the decode phase.
	 *
	 * @param[in] reg either reg_a, or reg_b
	 * @param[in] mask the registers set
	 */
	void setRead(REG_SET reg, uint8_t mask) {
		read[reg] = mask;
	}
	uint8_t getRead(REG_SET reg) {
		return read[reg];
	}

	/**
	 * Indicates which registers are written during the write
	 * phase
	 * @param[in] reg either reg_a, or reg_b
	 * @param[in] mask the registers set
	 */
	void setWrite(REG_SET reg, uint8_t mask) {
		write[reg] = mask;
	}
	uint8_t getWrite(REG_SET reg) {
		return write[reg];
	}


	/**
	 * Clears the instruction state
	 */
	void reset() {
		ins_opcode = 0;
		for (int i = 0; i < PS_STAGE_MAX; i++) {
			cycles[i] = 0;
		}
		for (int i = 0; i < REG_MAX; i++) {
			read[i] = 0;
			write[i] = 0;
		}
		unset = true;
		flushes = false;
	}

	bool isSet() {
		return !unset;
	}
private:
	bool unset = true;
	uint16_t ins_opcode;
	uint16_t cycles[PS_STAGE_MAX];

	uint8_t read[REG_MAX];
	uint8_t write[REG_MAX];

};

class pipeline {
public:
	/**
	 * Returns true if the pipeline can accept another instruction
	 */
	bool canAccept() {
		return (!pipe[PS_FETCH].isSet());
	}
	bool addInstruction(instruction &a);

	void advanceToAdd(instruction &a);

	/*
	 * Returns the number of cycles required to complete
	 * everything in the pipeline.
	 */
	unsigned long int getCycleCount() { return p_cycles; }
	unsigned long int getPipelessCount() { return raw_cycles; }	

	/**
	 * Advances the pipeline by the smallest amount, returning the
	 * number of cycles consumed.
	 */
	unsigned int advance();
	bool willStall(instruction &a, instruction &b);
	void copy(instruction &src, instruction &dst);
	unsigned long int remaining();
	bool isEmpty();
protected:
	instruction pipe[PS_STAGE_MAX];
	unsigned long int p_cycles = 0;
	unsigned long int raw_cycles = 0;
};


	
#endif /* PIPELINE */
