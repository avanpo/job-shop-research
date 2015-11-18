#include <stdio.h>
#include <stdlib.h>

#include "schedule.h"
#include "instance.h"

struct schedule *construct_schedule(struct instance *inst)
{
	struct schedule *schedule = calloc(1, sizeof(struct schedule));
	schedule->inst = inst;
	schedule->num_types = inst->num_types;

	struct machine_type *types = calloc(inst->num_types, sizeof(struct machine_type));
	schedule->types = types;

	int i, j, k;
	for (i = 0; i < inst->num_types; ++i, ++types) {
		types->num_machines = inst->types[i].num_machines;
		struct machine *machines = calloc(types->num_machines, sizeof(struct machine));
		types->machines = machines;
		for (j = 0; j < types->num_machines; ++j, ++machines) {
			machines->op_start_times = calloc(inst->num_ops, sizeof(int));
			for (k = 0; k < inst->num_ops; ++k) {
				machines->op_start_times[k] = -1;
			}
		}
	}
	return schedule;
}

struct schedule *copy_schedule(struct schedule *sch)
{
	struct schedule *copy = calloc(1, sizeof(struct schedule));
	copy->inst = sch->inst;
	copy->num_types = sch->num_types;

	struct machine_type *types = calloc(sch->num_types, sizeof(struct machine_type));
	copy->types = types;

	int i, j, k;
	for (i = 0; i < sch->num_types; ++i, ++types) {
		types->num_machines = sch->types[i].num_machines;
		struct machine *machines = calloc(types->num_machines, sizeof(struct machine));
		types->machines = machines;
		for (j = 0; j < types->num_machines; ++j, ++machines) {
			machines->op_start_times = calloc(sch->inst->num_ops, sizeof(int));
			for (k = 0; k < sch->inst->num_ops; ++k) {
				machines->op_start_times[k] = sch->types[i].machines[j].op_start_times[k];
			}
		}
	}
	copy->makespan = sch->makespan;
	return copy;
}

void destroy_schedule(struct schedule *sch)
{
	int i, j;
	for (i = 0; i < sch->num_types; ++i) {
		for (j = 0; j < sch->types[i].num_machines; ++j) {
			free(sch->types[i].machines[j].op_start_times);
		}
		free(sch->types[i].machines);
	}
	free(sch->types);
	free(sch);
}

int validate_schedule(struct schedule *sch, int verbose)
{
	int invalid = 0;

	int num_ops = sch->inst->num_ops;
	int *op_scheduled = calloc(num_ops, sizeof(int));
	int *op_started = calloc(num_ops, sizeof(int));
	int *op_machine = calloc(num_ops, sizeof(int));

	int i, j, k, l;
	for (i = 0; i < sch->num_types; ++i) {
		for (j = 0; j < sch->types[i].num_machines; ++j) {
			int *machine_in_use = calloc(sch->makespan, sizeof(int));
			for (k = 0; k < num_ops; ++k) {
				int start_time = sch->types[i].machines[j].op_start_times[k];
				if (start_time >= 0) {
					// check op completes before makespan
					if (start_time + sch->inst->ops[k].proc_time > sch->makespan) {
						++invalid;
						if (verbose) {
							printf("Op %d completed late.\n", k);
						}
					}
					// check op scheduled only once
					if (op_scheduled[k]) {
						++invalid;
						if (verbose) {
							printf("Op %d scheduled >1 times.\n", k);
						}
					}

					++op_scheduled[k];
					op_started[k] = start_time;
					op_machine[k] = j;

					for (l = 0; l < sch->inst->ops[k].proc_time; ++l) {
						++machine_in_use[start_time + l];
					}
				}
			}
			// check machine used only once at all times
			for (l = 0; l < sch->makespan; ++l) {
				if (machine_in_use[l] > 1) {
					++invalid;
					if (verbose) {
						printf("Machine %d of type %d overloaded.\n", j, i);
					}
				}
			}
			free(machine_in_use);
		}
	}

	for (k = 0; k < num_ops; ++k) {
		// check all ops scheduled
		if (!op_scheduled[k]) {
			++invalid;
			if (verbose) {
				printf("Op %d not scheduled.\n", k);
			}
		}
		if (sch->inst->ops[k].order > 0) {
			int d = op_started[k] - op_started[k - 1] -
				sch->inst->ops[k - 1].proc_time -
				sch->inst->ops[k - 1].idle_time;
			// check job precedence constraints
			if (d < 0) {
				++invalid;
				if (verbose) {
					printf("Op %d violates job precedence constraint.\n", k);
				}
			}
		}
	}

	free(op_scheduled);
	free(op_started);
	free(op_machine);

	return invalid == 0;
}

void print_schedule(struct schedule *sch, int start, int len)
{
	int end = start + len;
	if (len == 0 || end > sch->makespan) {
		end = sch->makespan;
	}
	printf("Printing schedule (makespan %d) from time %d to %d.\n", sch->makespan, start, end);
	int i, j, k, o, l;
	printf("+---+---+");
	for (k = start; k < end; ++k) {
		printf("--");
	}
	printf("+\n");
	printf("| t | m |");
	for (k = start; k < end; k += 2) {
		if (k < end - 1) {
			printf("%-2d  ", k);
		} else {
			printf("%-2d", k);
		}
	}
	printf("|\n+---+---+");
	for (k = start; k < end; ++k) {
		printf("--");
	}
	printf("+\n");
	for (i = 0; i < sch->num_types; ++i) {
		for (j = 0; j < sch->types[i].num_machines; ++j) {
			printf("| %1d | %1d |", i, j);
			for (k = start; k < end; ++k) {
				int flag = 0;
				for (o = 0; o < sch->inst->num_ops; ++o) {
					if (sch->types[i].machines[j].op_start_times[o] == k) {
						printf("%02d", o);
						for (l = 1; l < sch->inst->ops[o].proc_time && k + 1 < end; ++l) {
							printf("==");
							++k;
						}
						flag++;
						break;
					}
				}
				if (!flag) {
					printf("  ");
				}
			}
			printf("|\n");
		}
	} 
	printf("+---+---+");
	for (k = start; k < end; ++k) {
		printf("--");
	}
	printf("+\n");
}
