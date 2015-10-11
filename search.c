#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "graph.h"
#include "schedule.h"

#include "search.h"

static void replace_best_schedule(struct sa_state *sa);
static int handle_epoch(struct sa_state *sa);
static void print_sa_search_start(struct sa_state *sa);
static void print_sa_search_end(struct sa_state *sa);
static void print_sa_epoch_stats(struct sa_state *sa);
static int perform_swap(struct graph *graph, double temp);
static struct node *get_swap_possibility(struct graph *graph);
static int is_accepted(double temp, int old_makespan, int new_makespan);

struct sa_state *construct_sa_search(struct instance *inst)
{
	srand(1);
	struct sa_state *sa = calloc(1, sizeof(struct sa_state));

	sa->epoch_length = (inst->num_ops / inst->num_jobs) * 8;
	sa->initial_temp = 15;
	sa->alpha = 0.95;
	
	struct graph *graph = construct_graph(inst);
	init_graph(graph);
	sa->graph = graph;

	sa->best = copy_schedule(graph->schedule);

	return sa;
}

void destroy_sa_search(struct sa_state *sa)
{
	destroy_graph(sa->graph);
	destroy_schedule(sa->best);
	free(sa);
}

void start_sa_search(struct sa_state *sa)
{
	print_sa_search_start(sa);
	sa->temp = sa->initial_temp;

	struct graph *g = sa->graph;

	int i;
	int done = 0;
	for (sa->k = 0; !done; ++sa->k) {
		for (i = 0; i < sa->epoch_length; ++i) {
			sa->successes += perform_swap(g, sa->temp);
			if (sa->k > 0 && sa->graph->schedule->makespan < sa->best->makespan) {
				replace_best_schedule(sa);
			}
		}
		done = handle_epoch(sa);
	}
	print_sa_search_end(sa);
}

static void replace_best_schedule(struct sa_state *sa)
{
	free(sa->best);
	sa->best = copy_schedule(sa->graph->schedule);
}

static int handle_epoch(struct sa_state *sa)
{
	print_sa_epoch_stats(sa);

	sa->temp *= sa->alpha;

	int done = 0;
	if (sa->temp < 1.5 || sa->graph->schedule->makespan == sa->prev_makespan) {
		++done;
	}

	sa->successes = 0;
	sa->prev_makespan = sa->graph->schedule->makespan;

	return done;
}

static void print_sa_search_start(struct sa_state *sa)
{
	int min = min_job_makespan(sa->graph->inst);

	printf("\n");
	printf("Starting simulated annealing.\n");
	printf("Chosen parameters: L = %d, T_0 = %.0f, alpha = %.2f\n", sa->epoch_length, sa->initial_temp, sa->alpha);
	printf("  Makespan: \033[1m%d\033[0m (initial ordering)\n", sa->graph->schedule->makespan);
	printf("  Min job makespan: %d\n", min);
}

static void print_sa_search_end(struct sa_state *sa)
{
	//print_schedule(sa->best);
	printf("\n");
}

static void print_sa_epoch_stats(struct sa_state *sa)
{
	printf("Epoch: %d (T = %.1f)\n", sa->k, sa->temp);
	printf("  Makespan: \033[1m%d\033[0m\n", sa->graph->schedule->makespan);
	printf("  Success rate: %.1f%% (out of %d swaps)\n", 100 * (double)sa->successes / (double)sa->epoch_length, sa->epoch_length);
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
	int p = (int)floor(100 * exp(diff / temp));

	return (rand() % 100) < p;
}

/* performs randomized swap between consecutive operations
 * on the longest path on the same machine
 */
static int perform_swap(struct graph *graph, double temp)
{
	int old_makespan = graph->schedule->makespan;

	struct node *n2 = get_swap_possibility(graph);
	if (n2 == NULL) {
		return 0;
	}
	struct node *n1 = n2->prev_in_path;

	swap_operations(n1, n2);
	serialize_graph(graph);

	int new_makespan = graph->schedule->makespan;
	if (is_accepted(temp, old_makespan, new_makespan)) {
		return 1;
	} else {
		reverse_swap(n1->type);
		serialize_graph(graph);
		return 0;
	}
}
