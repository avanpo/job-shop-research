#ifndef SEARCH_H
#define SEARCH_H

#include "graph.h"

struct sa_state {
	int epoch_length;
	double initial_temp;
	double alpha;

	int start_time;
	struct graph *graph;

	int c;
	int k;
	double temp;

	int c_successes;
	int c_cycles;
	int c_rejected;
	int c_empty;
	int successes;

	int t_cycles;

	int verbose;
	int draw;
	int write;

	struct schedule *best;
};

struct sa_state *construct_sa_search(struct instance *inst, int verbose, int draw, int blocking, int write);
void destroy_sa_search(struct sa_state *sa);

void start_sa_search(struct sa_state *sa, int restarts);

#endif
