#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "graph.h"
#include "schedule.h"
#include "search.h"

static void replace_best_schedule(struct sa_state *sa);
static void update_stats(struct sa_state *sa, int result);
static int handle_restart(struct sa_state *sa);
static int handle_epoch(struct sa_state *sa);
static void print_sa_search_start(struct sa_state *sa);
static void print_sa_search_end(struct sa_state *sa);
static void print_sa_cycle_stats(struct sa_state *sa);
static void print_sa_epoch_stats(struct sa_state *sa);
static void validate_best_schedule(struct sa_state *sa);
static void print_elapsed_time(int start_time);
static int perform_swap(struct graph *graph, double temp);
static struct node *get_swap_possibility(struct graph *graph);
static int is_accepted(double temp, int old_makespan, int new_makespan);

struct sa_state *construct_sa_search(struct instance *inst, int verbose, int draw, int blocking, int neighborhood, int write)
{
	srand(time(NULL));
	struct sa_state *sa = calloc(1, sizeof(struct sa_state));

	sa->epoch_length = inst->num_ops * 8;
	sa->initial_temp = 25;
	sa->alpha = 0.95;
	
	struct graph *graph = construct_graph(inst, blocking, neighborhood);
	init_graph(graph);
	sa->graph = graph;

	sa->verbose = verbose;
	sa->draw = draw;
	sa->write = write;

	sa->best = copy_schedule(graph->schedule);

	return sa;
}

void destroy_sa_search(struct sa_state *sa)
{
	destroy_graph(sa->graph);
	destroy_schedule(sa->best);
	free(sa);
}

void start_sa_search(struct sa_state *sa, int restarts)
{
	print_sa_search_start(sa);

	sa->start_time = time(NULL);

	int i;
	int done = 0;
	int result;
	for (sa->c = 0; sa->c <= restarts && !done; ++(sa->c)) {
		sa->temp = sa->initial_temp;
		for (sa->k = 0; sa->temp > 0.5 && !done; ++sa->k) {
			for (i = 0; i < sa->epoch_length; ++i) {
				result = perform_swap(sa->graph, sa->temp);
				update_stats(sa, result);
				if (sa->graph->schedule->makespan < sa->best->makespan) {
					replace_best_schedule(sa);
				}
			}
			done = handle_epoch(sa);
		}
		done = handle_restart(sa);
	}

	print_sa_search_end(sa);
	validate_best_schedule(sa);

	if (sa->draw) {
		print_inst(sa->graph->inst);
		printf("\n");
		draw_schedule(sa->best, 0, 70);
		if (sa->best->makespan > 70) draw_schedule(sa->best, 60, 70);
	}
	if (sa->write) {
		write_schedule(sa->best);
	}
}

static void replace_best_schedule(struct sa_state *sa)
{
	destroy_schedule(sa->best);
	sa->best = copy_schedule(sa->graph->schedule);
}

static void update_stats(struct sa_state *sa, int result)
{
	switch(result) {
	case 0:
		++sa->c_successes;
		++sa->successes;
		break;
	case 1:
		++sa->c_cycles;
		break;
	case 2:
		++sa->c_rejected;
		break;
	case 3:
		++sa->c_empty;
		break;
	}
}

static int handle_restart(struct sa_state *sa)
{
	if (sa->verbose >= 1) {
		print_sa_cycle_stats(sa);
	}

	if (sa->best->makespan == sa->graph->inst->max_job_makespan ||
	   		(sa->c_successes == 0 && sa->c_rejected == 0)) {
		return 1;
	}

	sa->t_cycles += sa->c_cycles;

	sa->c_successes = 0;
	sa->c_cycles = 0;
	sa->c_rejected = 0;
	sa->c_empty = 0;

	return 0;
}

static int handle_epoch(struct sa_state *sa)
{
	if (sa->verbose >= 2) {
		print_sa_epoch_stats(sa);
	}

	sa->temp *= sa->alpha;

	sa->successes = 0;

	return sa->best->makespan == sa->graph->inst->max_job_makespan;
}

static void print_sa_search_start(struct sa_state *sa)
{
	printf("\n");
	print_inst_info(sa->graph->inst);
	printf("\n");
	printf("Problem definitions: Blocking = %s\n", sa->graph->blocking ? "yes" : "no");
	printf("Chosen search parameters: L = %d, T_0 = %.0f, alpha = %.2f\n", sa->epoch_length, sa->initial_temp, sa->alpha);
	printf("  Makespan: \033[1m%d\033[0m (initial ordering)\n", sa->graph->schedule->makespan);
	printf("\nStarting simulated annealing.\n");
}

static void print_sa_search_end(struct sa_state *sa)
{
	printf("\n");
	printf("Printing search statistics\n");
	printf("  Restarts: %d\n", sa->c - 1);
	printf("  Cycles encountered: %d\n", sa->t_cycles);
	print_elapsed_time(sa->start_time);
	printf("\n");
	if (sa->verbose) {
		print_inst_info(sa->graph->inst);
		printf("\n");
	}
	printf("Best solution found: \033[1m%d\033[0m\n", sa->best->makespan);
	printf("\n");
}

static void print_sa_epoch_stats(struct sa_state *sa)
{
	printf("Epoch: %d (T = %.1f)\n", sa->k, sa->temp);
	printf("  Makespan: \033[1m%d\033[0m\n", sa->graph->schedule->makespan);
	printf("  Success rate: %.1f%% (out of %d swaps)\n", 100 * (double)sa->successes / (double)sa->epoch_length, sa->epoch_length);
}

static void print_sa_cycle_stats(struct sa_state *sa)
{
	printf("Cycle: %d\n", sa->c);
	printf("  Makespan: \033[1m%d\033[0m\n", sa->graph->schedule->makespan);
	printf("  Success rate: %.1f%%\n", 100 * (double)sa->c_successes / ((double)sa->epoch_length * (double)sa->k));
	printf("  Success/Infeasible/Rejected/Empty: %d/%d/%d/%d\n", sa->c_successes, sa->c_cycles, sa->c_rejected, sa->c_empty);
}

static void validate_best_schedule(struct sa_state *sa)
{
	printf("Validating best solution\n");
	int valid = validate_schedule(sa->best, sa->verbose);

	if (valid) {
		printf("  Solution \033[1mvalid\033[0m\n");
	} else {
		printf("  Solution \033[1minvalid\033[0m\n");
	}
	printf("\n");
}

static void print_elapsed_time(int start_time)
{
	int hour, min, sec;
	int t = time(NULL) - start_time;

	hour = t / 3600;
	min = (t % 3600) / 60;
	sec = (t % 3600) % 60;

	printf("  Elapsed time: ");
	if (hour) printf("%d hour%s, ", hour, hour == 1 ? "" : "s");
	if (hour || min) printf("%d min, ", min);
	printf("%d second%s\n", sec, sec == 1 ? "" : "s");
}

/* takes serialized graph, selects random operation on the
 * longest path for which the previous operation on the
 * longest path is executed by the same machine, and not
 * in the same job
 */
static struct node *get_swap_possibility(struct graph *graph)
{
	int *path = calloc(graph->num_nodes, sizeof(int));
	int *possible = calloc(graph->num_nodes, sizeof(int));
	int path_len = get_longest_path(graph, path);
	struct node *n1, *n2;
	int i, j;
	for (i = 0, j = 0; i < path_len - 1; ++i) {
		n1 = graph->nodes + path[i];
		n2 = graph->nodes + path[i + 1];
		if (n1->type == n2->type && n1->op->job->id != n2->op->job->id) {
			possible[j] = path[i + 1];
			++j;
		}
	}
	free(path);
	
	if (j == 0) {
		free(possible);
		return NULL;
	}

	int r = rand() % j;
	int node_id = possible[r];

	free(possible);
	return graph->nodes + node_id;
}

/* takes serialized graph, selects random operation of random
 * type with the previously ordered operation of a different job
 */
static struct node *get_consecutive_swap_possibility(struct graph *graph)
{
	struct node *n1 = NULL, *n2 = NULL;
	int t = 0, o = 0;

	do {
		t = rand() % graph->num_types;
		o = rand() % (graph->types[t].num_ops - 1);
		n1 = graph->nodes + graph->types[t].ops_order[o];
		n2 = graph->nodes + graph->types[t].ops_order[o + 1];
	} while (n1->op->job == n2->op->job);

	n2->order_index = o + 1;

	return n2;
}

/* returns 0 or 1 depending on whether new solution accepted or not
 */
static int is_accepted(double temp, int old_makespan, int new_makespan)
{
	if (new_makespan < old_makespan) {
		return 1;
	} else if (temp <= 0.0) {
		return 0;
	}

	double diff = (double)(old_makespan - new_makespan);
	int p = (int)floor(100000 * exp(diff / temp));
	int r = rand() % 100000;

	return r < p;
}

/* finds a swap depending on neighborhood and performs
 * said swap
 */
static int find_and_swap(struct graph *graph)
{
	int ne = graph->neighborhood;
	if (ne == 2) {
		ne -= (1 + rand() % 2);
	}

	struct node *n2 = NULL;

	if (ne) {
		n2 = get_consecutive_swap_possibility(graph);
	} else {
		n2 = get_swap_possibility(graph);
	}

	if (n2 == NULL) {
		return 0;
	}

	graph->type_backup = n2->type->id;

	if (ne) {
		swap_consecutive_operations(n2);
	} else {
		struct node *n1 = n2->prev_in_path;
		swap_operations(n1, n2);
	}
	return 1;
}

/* performs randomized swap between consecutive operations
 * on the longest path on the same machine
 * returns: 0 on success
 *          1 on cyclic graph
 *          2 on sa rejected
 *          3 on empty neighborhood
 */
static int perform_swap(struct graph *graph, double temp)
{
	int old_makespan = graph->schedule->makespan;

	if (!find_and_swap(graph)) {
		return 3;
	}

	int serialized = serialize_graph(graph);

	int new_makespan = graph->schedule->makespan;
	if (serialized && is_accepted(temp, old_makespan, new_makespan)) {
		return 0;
	} else {
		reverse_swap(graph->types + graph->type_backup);
		serialize_graph(graph);
		if (!serialized) {
			return 1;
		}
		return 2;
	}
}
