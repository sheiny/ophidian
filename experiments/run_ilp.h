#ifndef EXPERIMENTS_RUN_ILP
#define EXPERIMENTS_RUN_ILP

#include <ophidian/design/Design.h>

void run_mcf_multithreading(ophidian::design::Design & design);

void run_ilp_for_circuit(ophidian::design::Design & design, std::string circuit_name, bool initial_routing = true);

void run_circuit(ophidian::design::Design & design, std::string circuit_name);

#endif
