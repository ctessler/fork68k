#include <iostream>
#include "getopt.h"
#include "step68k.h"

/* Globals */
struct {
	int opt_verb;
	string opt_ctxfile;
	string opt_insfile;
} OPTS;

void enable_verbose();
void usage();
void vout(string s);

int main(int argc, char** argv)
{
	static struct option loptions[] = {
		{ "verbose", no_argument, 0, 'v'},
		{ "serial", no_argument, 0, 's' },
		{ NULL, 0, 0, 0 }
	};

	bool done = false;
	while (!done) {
		int opt_idx;
		int c = getopt_long(argc, argv, "vsc:", loptions, &opt_idx);

		switch (c) {
		case 0:
			/* long option */
			cout << "long option" << endl;
			break;
		case -1:
			done = true;
			break;
		case 'c':
			OPTS.opt_ctxfile = (string) optarg;
			vout("Context file: " + OPTS.opt_ctxfile + "\n");
			break;
		case 'v':
			enable_verbose();
			break;
		default:
			usage();
			return -1;
		}
	}

	if (optind >= argc) {
		usage();
		return -1;
	}
	OPTS.opt_insfile = argv[optind];

	vout("Instruction file: " + OPTS.opt_insfile + "\n");

	
	Step68k step68k;
	step68k.setObjectFile(OPTS.opt_insfile);
	uint16_t word;
	step68k.readWord(word);
	cout << "Read: " << word << endl;
	
	return 0;
}

void enable_verbose() {
	OPTS.opt_verb = 1;
	vout("Verbose output enabled.\n");
}

void vout(string s) {
	if (!OPTS.opt_verb) {
		return;
	}
	cout << "verbose: " << s;
}

void usage() {
	cout << "Usage: ctsim [options] -c <context file> <instruction file>" << endl
	     << "OPTIONS:" << endl
	     << "\t" << "--verbose|-v"
	     << "\t\t" << "Enable verbose output" << endl
	     << "\t" << "--serial|-s"
	     << "\t\t" << "Disable pipeline analysis" << endl
	     << "\t" << "-c <context>"
	     << "\t\t" << "Memory context file" << endl; 
		
}



// Local Variables:
// c-basic-offset: 8
// fill-column: 80
// End:
