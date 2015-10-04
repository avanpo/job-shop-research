#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "graph.h"
#include "schedule.h"

#include "search.h"

static int perform_swap(struct graph *graph, double temp);
static struct node *get_swap_possibility(struct graph *graph);
static int is_accepted(double temp, int old_makespan, int new_makespan);

struct sa_state *construct_sa_search(struct instance *inst)
{
	srand(1);
	struct sa_state *sa = calloc(1, sizeof(struct sa_state));

	sa->epoch_length = (inst->num_ops / inst->num_jobs) * 10;
	sa->initial_temp = 15;
	sa->alpha = 0.95;
	
	struct graph *graph = construct_graph(inst);
	init_graph(graph);
	sa->graph = graph;
}

void destroy_sa_search(struct sa_state *sa)
{
	destroy_graph(sa->graph);
	free(sa);
}

void start_sa_search(struct sa_state *sa)
{
	print_sa_search_start(sa);
	sa->temp = sa->initial_temp;

	struct graph *g = sa->graph;

	int i, k;
	int s;
	for (k = 0, s = 0; sa->temp > 10.5; ++k, sa->temp *= sa->alpha) {
		for (i = 0, s = 0; i < sa->epoch_length; ++i) {
			s += perform_swap(g, sa->temp);
			print_schedule_labeled(sa->graph->schedule);
			print_longest_path(sa->graph);
		}
		print_sa_epoch_stats(sa, k, s);
		print_schedule_labeled(sa->graph->schedule);
	}
}

void print_sa_search_start(struct sa_state *sa)
{
	printf("Starting simulated annealing.\n");
	printf("Chosen parameters: L = %d, T_0 = %.0f, alpha = %.2f\n", sa->epoch_length, sa->initial_temp, sa->alpha);
	printf("  Makespan: \033[1m%d\033[0m\n", sa->graph->schedule->makespan);
}

void print_sa_epoch_stats(struct sa_state *sa, int k, int s)
{
	printf("Epoch: %d\n", k);
	printf("  Makespan: \033[1m%d\033[0m\n", sa->graph->schedule->makespan);
	printf("  Success rate: %.1f (out of %d swaps)\n", 100 * (double)s / (double)sa->epoch_length, sa->epoch_length);
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
		if (n1->machine == n2->machine && n1->op->job->id != n2->op->job->id) {
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
	printf("swapping operations %d and %d\n", n1->id, n2->id);

	swap_operations(n1, n2);
	print_graph(graph);
	serialize_graph(graph);

	int new_makespan = graph->schedule->makespan;
	if (is_accepted(temp, old_makespan, new_makespan)) {
		return 1;
	} else {
		swap_operations(n1, n2);
		serialize_graph(graph);
		return 0;
	}
}
