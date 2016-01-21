#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instance.h"
#include "graph.h"
#include "schedule.h"

static struct graph *graph;

static int serialize_node(struct graph *graph, struct node *node);
static void serialize_cycle(struct graph *graph, struct node **ops, int n);
static int is_cycle_serializable(struct node **ops, struct graph *graph, int *not_serializable, int m);
static int is_serializable(struct node *node);
static void deserialize_schedule_node(struct schedule *sch, struct node *node);
static void deserialize_graph(struct graph *graph);

struct graph *construct_graph(struct instance *inst, int blocking)
{
	struct graph *g = calloc(1, sizeof(struct graph));
	struct node_type *t = calloc(inst->num_types, sizeof(struct node_type));
	struct node *n = calloc(inst->num_ops, sizeof(struct node));

	graph = g;

	g->inst = inst;
	g->blocking = blocking;

	g->num_types = inst->num_types;
	g->types = t;
	g->num_nodes = inst->num_ops;
	g->nodes = n;

	g->schedule = construct_schedule(inst);

	int *backup = calloc(inst->num_ops, sizeof(int));

	int i;
	for (i = 0; i < inst->num_types; ++i, ++t) {
		t->id = i;
		t->num_machines = inst->types[i].num_machines;
		t->num_ops = inst->types[i].num_ops;
		t->ops_order = calloc(inst->types[i].num_ops, sizeof(int));
		t->ops_order_backup = backup;

		t->blocked = calloc(inst->types[i].num_machines, sizeof(int));
		t->end_times = calloc(inst->types[i].num_machines, sizeof(int));
		t->end_ops = calloc(inst->types[i].num_machines, sizeof(int));
	}

	for (i = 0; i < inst->num_ops; ++i, ++n) {
		n->id = i;
		n->op = inst->ops + i;
		n->type = &g->types[n->op->type];
		if (n->op == n->op->job->ops) {
			n->prev = NULL;
		} else {
			n->prev = n - 1;
		}
		if (n->op - n->op->job->ops == n->op->job->num_ops - 1) {
			n->next = NULL;
		} else {
			n->next = n + 1;
		}
		n->prev_in_path = NULL;
	}

	return g;
}

void destroy_graph(struct graph *graph)
{
	int i;
	for (i = 0; i < graph->num_types; ++i) {
		free(graph->types[i].blocked);
		free(graph->types[i].end_times);
		free(graph->types[i].end_ops);
		free(graph->types[i].ops_order);
	}
	free(graph->types[0].ops_order_backup);
	free(graph->types);
	free(graph->nodes);
	destroy_schedule(graph->schedule);
	free(graph);
}

void init_graph(struct graph *graph)
{
	int t, o, i;
	for (t = 0; t < graph->num_types; ++t) {
		for (o = 0, i = 0; o < graph->num_nodes; ++o) {
			if (graph->inst->types[t].ops[o]) {
				graph->types[t].ops_order[i] = o;
				++i;
			}
		}
	}
	serialize_graph(graph);
}

int serialize_graph(struct graph *graph)
{
	deserialize_graph(graph);
	struct node *n, *last = NULL;

	int *progress = calloc(graph->num_types, sizeof(int));
	int *serialized = calloc(graph->num_nodes, sizeof(int));
	int *not_serializable = calloc(graph->num_nodes, sizeof(int));

	int i, j, l, m = 0, k, loop_guard = 0, makespan = 0;
	for (i = graph->num_nodes, j = 0; i; j %= graph->num_types) {
		if (loop_guard > graph->num_types * 2) {
			free(progress);
			free(serialized);
			free(not_serializable);
			return 0;
		}
		if (progress[j] == graph->types[j].num_ops) {
			++j;
			++loop_guard;
			continue;
		}
		n = graph->nodes + graph->types[j].ops_order[progress[j]];
		int available = (n->prev == NULL || serialized[n->prev->id]);
		if (available && is_serializable(n)) {
			l = serialize_node(graph, n);
			++serialized[n->id];

			if (l > makespan) {
				makespan = l;
				last = n;
			}

			++progress[j];
			--i;
			loop_guard = 0;
			memset(not_serializable, 0, graph->num_nodes * sizeof(int));
			m = 0;
		} else if (available && graph->blocking) {
			int seen = 0;
			for (k = 0; k < m; ++k) {
				if (not_serializable[k] == n->id) {
					++seen;
					break;
				}
			}
			if (!seen) {
				not_serializable[m++] = n->id;
			}

			struct node **ops = calloc(m, sizeof(struct node *));

			int o = 0;
			if (m > 1 && loop_guard > graph->num_types) {
				o = is_cycle_serializable(ops, graph, not_serializable, m);
			}

			if (o) {
				serialize_cycle(graph, ops, o);
				for (k = 0; k < o; ++k) {
					++serialized[ops[k]->id];

					l = ops[k]->start_time + ops[k]->op->proc_time + ops[k]->op->idle_time;
					if (l > makespan) {
						makespan = l;
						last = ops[k];
					}

					++progress[ops[k]->type->id];
					--i;
				}
				loop_guard = 0;
				memset(ops, 0, m * sizeof(struct node *));
				o = 0;
			} else {
				++j;
				++loop_guard;
			}
			free(ops);
		} else {
			++j;
			++loop_guard;
		}
	}
	graph->last = last;
	graph->schedule->makespan = makespan;

	free(progress);
	free(serialized);
	free(not_serializable);
	return 1;
}

/* swap operation order while maintaining job operation precedence
 * within machine ordering
 */
void swap_operations(struct node *n1, struct node *n2)
{
	if (n1->type != n2->type) {
		fprintf(stderr, "Cannot swap operations %d and %d: machine types do not match.\n", n1->id, n2->id);
		exit(EXIT_FAILURE);
	} else if (n1->id == n2->id) {
		fprintf(stderr, "Cannot swap operation %d with itself.\n", n1->id);
		exit(EXIT_FAILURE);
	}

	struct node_type *t = n1->type;
	int i, i1, i2;
	for (i = 0, i1 = -1, i2 = -1; i < t->num_ops; ++i) {
		if (t->ops_order[i] == n1->id) {
			i1 = i;
		} else if (t->ops_order[i] == n2->id) {
			i2 = i;
			break;
		}
	}

	if (i1 < 0 || i2 < 0) {
		fprintf(stderr, "Cannot find operations %d and %d in machine type %d.\n", n1->id, n2->id, t->id);
		exit(EXIT_FAILURE);
	}

	// backup the order, in case the swap needs to be reversed
	for (i = 0; i < t->num_ops; ++i) {
		t->ops_order_backup[i] = t->ops_order[i];
	}

	// incrementally move n2 forward, then if the operations haven't
	// been switched yet, incrementally move n1 backward
	struct node *nodes = n1 - n1->id;
	struct node *node;
	int done = 0;
	for (i = i2 - 1; i >= i1; --i) {
		node = nodes + t->ops_order[i];
		if (node->op->job->id != n2->op->job->id) {
			t->ops_order[i2] = node->id;
			t->ops_order[i] = n2->id;
			i2 = i;
			if (node == n1) {
				++done;
			}
		} else {
			break;
		}
	}
	for (i = i1 + 1; i <= i2 && !done; ++i) {
		node = nodes + t->ops_order[i];
		if (node->op->job->id != n1->op->job->id) {
			t->ops_order[i1] = node->id;
			t->ops_order[i] = n1->id;
			i1 = i;
			if (node == n2) {
				++done;
			}
		} else {
			break;
		}
	}

	if (done) {
		return;
	} else {
		fprintf(stderr, "Something went wrong swapping operations %d and %d in machine type %d.\n", n1->id, n2->id, t->id);
		exit(EXIT_FAILURE);
	}
}

void reverse_swap(struct node_type *type)
{
	int i;
	for (i = 0; i < type->num_ops; ++i) {
		type->ops_order[i] = type->ops_order_backup[i];
	}
}

int get_longest_path(struct graph *graph, int *path)
{
	int *reverse_path = calloc(graph->num_nodes, sizeof(int));
	struct node *n;
	int i, j;
	for (i = 0, n = graph->last; n; ++i, n = n->prev_in_path) {
		reverse_path[i] = n->id;
	}
	for (--i, j = 0; i >= 0; --i, ++j) {
		path[j] = reverse_path[i];
	}
	free(reverse_path);
	return j;
}

void print_graph(struct graph *graph)
{
	printf("Printing graph representation\n");
	int i, j;
	for (i = 0; i < graph->num_types; ++i) {
		printf("type %d: %d machines\n", i, graph->types[i].num_machines);
		printf("  operation priority:");
		for (j = 0; j < graph->types[i].num_ops; ++j) {
			printf(" %d", graph->types[i].ops_order[j]);
		}
		printf("\n");
	}
}

void print_type(struct node_type *t, int backup)
{
	printf("Printing type %d[%d] order%s\n", t->id, t->num_machines, backup ? " (backup)" : "");
	int i;
	for (i = 0; i < t->num_ops; ++i) {
		if (!backup) {
			printf(" %d", t->ops_order[i]);
		} else {
			printf(" %d", t->ops_order_backup[i]);
		}
	}
	printf("\n");
}

void print_longest_path(struct graph *graph)
{
	printf("Printing longest path (makespan %d)\n", graph->last->start_time + graph->last->op->proc_time);
	printf("time |  id | job | tp m | proc idle\n");
	printf("-----+-----+-----+------+----------\n");
	int *path = calloc(graph->num_nodes, sizeof(int));
	int path_len = get_longest_path(graph, path);
	struct node *n;
	int i;
	for (i = 0; i < path_len; ++i) {
		n = graph->nodes + path[i];
		printf("%4d |  %02d | %3d |  %1d %1d | %4d %4d\n", n->start_time, n->id, n->op->job->id, n->type->id,
				n->machine, n->op->proc_time, n->op->idle_time);
	}
	printf("-----+-----+-----+------+----------\n");
	free(path);
}

static int serialize_node(struct graph *graph, struct node *node)
{
	// calculate time at which earlier job operations
	// are finished
	int job_release = 0;
	if (node->prev != NULL) {
		job_release = node->prev->start_time + node->prev->op->proc_time +
			node->prev->op->idle_time;
	}

	// calculate time at which at least 1 machine is
	// available
	// check: !blocked || blocked by prev job op
	int machine = -1;
	int machine_release = INT_MAX;
	int m;
	for (m = 0; m < node->type->num_machines; ++m) {
		if ((!node->type->blocked[m] ||	(node->prev && node->type->end_ops[m] == node->prev->id)) && node->type->end_times[m] < machine_release) {
			machine_release = node->type->end_times[m];
			machine = m;
		}
	}

	// do not schedule operation until start time of previous
	// operation to preserve ordering
	if (machine_release < node->type->prev_start_time) {
		machine_release = node->type->prev_start_time;
	}

	// calculate operation start time and previous in path
	int start_time;
	if (job_release >= machine_release) {
		start_time = job_release;
		node->prev_in_path = node->prev;
	} else {
		start_time = machine_release;
		if (machine_release == node->type->prev_start_time && machine_release != 0) {
			node->prev_in_path = graph->nodes + node->type->prev_start_op;
		} else {
			node->prev_in_path = graph->nodes + node->type->end_ops[machine];
		}
	}

	// blocking resolution
	if (graph->blocking) {
		if (node->prev) {
			node->prev->type->end_times[node->prev->machine] = start_time;
			node->prev->type->blocked[node->prev->machine] = 0;
		}
		if (node->next) {
			node->type->blocked[machine] = 1;
		}
	}

	int finish_time = start_time + node->op->proc_time;

	deserialize_schedule_node(graph->schedule, node);
	graph->schedule->types[node->type->id].machines[machine].op_start_times[node->id] = start_time;
	node->start_time = start_time;
	node->machine = machine;
	node->type->prev_start_op = start_time > node->type->prev_start_time ? node->id : node->type->prev_start_op;
	node->type->prev_start_time = start_time;
	node->type->end_times[machine] = finish_time;
	node->type->end_ops[machine] = node->id;

	return finish_time;
}

static void serialize_cycle(struct graph *graph, struct node **ops, int n)
{
	int i;
	int start_time = 0;
	for (i = 0; i < n; ++i) {
		int t =	ops[i]->prev->start_time + ops[i]->prev->op->proc_time + ops[i]->prev->op->idle_time;
		if (t > start_time) {
			start_time = t;
		}
	}

	for (i = 0; i < n; ++i) {
		struct node *node = ops[i];
		int finish_time = start_time + node->op->proc_time;
		int machine = ops[i == 0 ? n - 1 : i - 1]->prev->machine;

		if (graph->blocking && !node->next) {
			node->type->blocked[machine] = 0;
		}

		deserialize_schedule_node(graph->schedule, node);
		graph->schedule->types[node->type->id].machines[machine].op_start_times[node->id] = start_time;
		node->start_time = start_time;
		node->machine = machine;
		node->type->prev_start_op = start_time > node->type->prev_start_time ? node->id : node->type->prev_start_op;
		node->type->prev_start_time = start_time > node->type->prev_start_time ? start_time : node->type->prev_start_time;
		node->type->end_times[machine] = finish_time;
		node->type->end_ops[machine] = node->id;
	}
}

// return number of nodes inside serializable blocking cycle
static int is_cycle_serializable(struct node **ops, struct graph *graph, int *not_serializable, int m)
{
	// this could definitely be optimized and may not find
	// all possible blocking cycles
	int *used = calloc(graph->num_nodes, sizeof(int));

	int found_loop = 0, o = 0;
	int x, y, z;
	for (x = 0; x < m && !found_loop; ++x) {
		if (!(graph->nodes[not_serializable[x]].prev)) {
			continue;
		}
		ops[o++] = graph->nodes + not_serializable[x];
		++used[not_serializable[x]];
		for (y = x + 1, z = 0; z < m * m; y = (y + 1) % m, ++z) {
			struct node *curr = graph->nodes + not_serializable[y];
			if (!used[curr->id] && curr->prev && curr->type == ops[o - 1]->prev->type) {
				ops[o++] = curr;
				++used[curr->id];
				if (curr->prev->type == ops[0]->type) {
					++found_loop;
					break;
				}
			}
		}
		if (!found_loop) {
			memset(ops, 0, m * sizeof(struct node *));
			o = 0;
		}
		memset(used, 0, graph->num_nodes * sizeof(int));
	}
	
	free(used);
	return o;
}

static int is_serializable(struct node *node)
{
	int m;
	// check: !blocked || blocked by prev job op
	for (m = 0; m < node->type->num_machines; ++m) {
		if (!node->type->blocked[m] || (node->prev && node->type->end_ops[m] == node->prev->id)) {
			return 1;
		}
	}
	return 0;
}

static void deserialize_schedule_node(struct schedule *sch, struct node *node)
{
	int i;
	for (i = 0; i < sch->types[node->type->id].num_machines; ++i) {
		sch->types[node->type->id].machines[i].op_start_times[node->id] = -1;
	}
}

static void deserialize_graph(struct graph *graph)
{
	int i;
	for (i = 0; i < graph->num_types; ++i) {
		graph->types[i].prev_start_time = 0;
		graph->types[i].prev_start_op = 0;

		int blob_size = graph->types[i].num_machines * sizeof(int);
		memset(graph->types[i].blocked, 0, blob_size);
		memset(graph->types[i].end_times, 0, blob_size);
		memset(graph->types[i].end_ops, 0, blob_size);
	}
}
