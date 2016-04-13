#include "pipeline.h"


/**
 * Determines if instruction a will cause b to stall.
 *
 * @param[in] a instruction that my write a value
 * @param[in] b instruction that may read a value
 *
 * @return true if a will write a value that b will read,
 * necessitating that b will be stalled in the decode phase.
 */
bool pipeline::willStall(instruction &a, instruction &b) {
	uint8_t A, B;
	A = a.getWrite(REG_A);
	B = b.getRead(REG_A);
	if (A & B) {
		return true;
	}
	A = a.getWrite(REG_D);
	B = b.getRead(REG_D);
	if (A & B) {
		return true;
	}
	return false;
}

void pipeline::copy(instruction &src, instruction &dst) {
	if (!src.isSet()) {
		dst.reset();
		return;
	}
	dst.setOpCode(src.getOpCode());
	for (int i = 0; i < PS_STAGE_MAX; i++) {
		PIPE_STAGE s = (PIPE_STAGE) i;
		dst.setCycles(s, src.getCycles(s));
	}
	for (int i = REG_A; i < REG_MAX; i++) {
		REG_SET r = (REG_SET) i;
		dst.setRead(r, src.getRead(r));
		dst.setWrite(r, src.getWrite(r));
	}
	dst.flushes = src.flushes;
}

/**
 * Advances the pipeline by the smallest increment
 */
unsigned int pipeline::advance() {
	uint16_t cycles;

	/* Last instruction flushes the pipe */
	if (pipe[PS_WRITE].flushes) {
		cycles = pipe[PS_WRITE].getCycles(PS_WRITE);
		pipe[PS_WRITE].execCycles(PS_WRITE, cycles);
		for (int i = 0; i < PS_STAGE_MAX; i++) {
			pipe[i].reset();
		}
		p_cycles += cycles;
		return cycles;
	}

	/* Last instruction stalls the pipe */
	if (willStall(pipe[PS_WRITE], pipe[PS_DECODE])) {
		uint16_t wc = pipe[PS_WRITE].getCycles(PS_WRITE);
		uint16_t ec = pipe[PS_EXECUTE].getCycles(PS_EXECUTE);
		cycles = wc;
		if (ec < cycles) {
			cycles = ec;
		}
		pipe[PS_FETCH].execCycles(PS_FETCH, cycles);
		pipe[PS_EXECUTE].execCycles(PS_EXECUTE, cycles);
		pipe[PS_WRITE].execCycles(PS_WRITE, cycles);
		goto shift;
	}

	/* No stalls, no flushes, normal operation */
	cycles = UINT16_MAX;
	for (int i = 0; i < PS_STAGE_MAX; i++) {
		if (!pipe[i].isSet()) {
			continue;
		}
		PIPE_STAGE s = (PIPE_STAGE) i;
		uint16_t t = pipe[i].getCycles(s);
		if (t > 0 && t < cycles) {
			cycles = t;
		}
	}
	if (cycles == UINT16_MAX) {
		cycles = 0;
	}
	for (int i = 0; i < PS_STAGE_MAX; i++) {
		pipe[i].execCycles((PIPE_STAGE)i, cycles);
	}

shift:
	for (int i = PS_WRITE; i >= PS_FETCH; i--) {
		if (pipe[i].getCycles((PIPE_STAGE)i) != 0) {
			/* More cycles to execute in this phase */
			continue;
		}
		if (i == PS_WRITE) {
			/* Nothing beyond, all done. */
			pipe[i].reset();
			continue;
		}
		/* Need to check next position */
		if (!pipe[i+1].isSet()) {
			/* Move this instruction to the next position */
			copy(pipe[i], pipe[i+1]);
			pipe[i].reset();
		}
		
	}
	p_cycles += cycles;
	return cycles;
}

/**
 * Returns the remaing cycles for the instructions in the pipeline.
 */
unsigned long int pipeline::remaining() {
	pipeline duplicate;

	for (int i = 0 ; i < PS_STAGE_MAX; i++) {
		copy(pipe[i], duplicate.pipe[i]);
	}

	unsigned long int remaining = 0;
	unsigned int r;
	do {
		r = duplicate.advance();
		remaining += r;
	} while (!duplicate.isEmpty());

	return remaining;
}

bool pipeline::addInstruction(instruction &a) {
	if (!canAccept()) {
		return false;
	}

	copy(a, pipe[PS_FETCH]);
	if (pipe[PS_FETCH].flushes) {
		while (!isEmpty()) {
			advance();
		}
	}

	for (int i = 0; i < PS_STAGE_MAX; i++) {
		raw_cycles += a.getCycles((PIPE_STAGE) i);
	}
	
	return true;
}

void pipeline::advanceToAdd(instruction &a) {
	while (!canAccept()) {
		advance();
	}
	addInstruction(a);
}

bool pipeline::isEmpty() {
	bool value = false;
	for (int i = 0; i < PS_STAGE_MAX; i++) {
		value |= pipe[i].isSet();
	}
	return !value;
}
	
