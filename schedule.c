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
	int i, j, k, o, l;
	printf("schedule has makespan of %d\n\n", sch->makespan);
	printf("type | mac |");
	for (k = 0; k < sch->makespan; k += 2) {
		printf("%-2d  ", k);
	}
	printf("\n-----+-----+");
	for (k = 0; k < sch->makespan; ++k) {
		printf("--");
	}
	printf("\n");
	for (i = 0; i < sch->num_types; ++i) {
		for (j = 0; j < sch->types[i].num_machines; ++j) {
			printf("%4d | %3d |", i, j);
			for (k = 0; k < sch->makespan; ++k) {
				int flag = 0;
				for (o = 0; o < sch->inst->num_ops; ++o) {
					if (sch->types[i].machines[j].op_start_times[o] == k) {
						printf("%02d", o);
						for (l = 2; l < sch->inst->ops[o].proc_time; ++l) {
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
			printf("\n");
		}
	}
}
