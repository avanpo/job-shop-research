#ifndef SEARCH_H
#define SEARCH_H

#include "graph.h"

struct sa_state {
	int epoch_length;
	double initial_temp;
	double alpha;

	struct graph *graph;
	double temp;

	struct schedule *best;
};

struct sa_state *construct_sa_search(struct instance *inst);
void destroy_sa_search(struct sa_state *sa);

void start_sa_search(struct sa_state *sa);

void print_sa_search_start(struct sa_state *sa);
void print_sa_epoch_stats(struct sa_state *sa, int k, int s);

#endif
