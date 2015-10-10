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

void print_schedule(struct schedule *sch)
{
	printf("Printing schedule (makespan %d)\n", sch->makespan);
	int i, j, k, o, l;
	printf("+---+---+");
	for (k = 0; k < sch->makespan; ++k) {
		printf("-");
	}
	printf("+\n");
	printf("| t | m |");
	for (k = 0; k < sch->makespan - 1; k += 4) {
		if (k < sch->makespan - 3) {
			printf("%-2d  ", k);
		} else {
			printf("%-2d", k);
		}
	}
	for (k = 0; k < sch->makespan % 2; ++k) {
		printf(" ");
	}
	printf("|\n+---+---+");
	for (k = 0; k < sch->makespan; ++k) {
		printf("-");
	}
	printf("+\n");
	for (i = 0; i < sch->num_types; ++i) {
		for (j = 0; j < sch->types[i].num_machines; ++j) {
			printf("| %1d | %1d |", i, j);
			for (k = 0; k < sch->makespan;) {
				int flag = 0;
				for (o = 0; o < sch->inst->num_ops; ++o) {
					if (sch->types[i].machines[j].op_start_times[o] == k) {
						for (l = 0; l < sch->inst->ops[o].proc_time; ++l) {
							printf("=");
							++k;
						}
						flag++;
						break;
					}
				}
				if (!flag) {
					++k;
					printf(" ");
				}
			}
			printf("|\n");
		}
	}
	printf("+---+---+");
	for (k = 0; k < sch->makespan; ++k) {
		printf("-");
	}
	printf("+\n");
}

void print_schedule_labeled(struct schedule *sch, int start, int len)
{
	printf("Printing schedule (makespan %d)\n", sch->makespan);
	int end = start + len;
	if (len == 0) {
		end = sch->makespan;
	}
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
						for (l = 1; l < sch->inst->ops[o].proc_time; ++l) {
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
