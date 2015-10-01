#include <stdio.h>
#include <stdlib.h>

#include "instance.h"

struct instance *read_inst(char fname[])
{
	FILE *fp = fopen(fname, "r");
	if (fp == NULL) {
		fprintf(stderr, "File \"%s\" failed to open.\n", fname);
		exit(EXIT_FAILURE);
	}

	struct instance *inst = calloc(1, sizeof(struct instance));
	struct operation *ops = calloc(MAX_OPERATIONS, sizeof(struct operation));

	int t, j, o, jo;
	int num_types, num_jobs, num_ops;

	// read types
	fscanf(fp, " types %d", &num_types);
	struct type *types = calloc(num_types, sizeof(struct type));
	struct type *type = types;
	for (t = 0; t < num_types; ++t, ++type) {
		type->id = t;
		fscanf(fp, "%d", &(type->num_machines));
	}

	// read jobs with contained operations
	fscanf(fp, " jobs %d", &num_jobs);
	struct job *jobs = calloc(num_jobs, sizeof(struct job));
	struct job *job = jobs;
	struct operation *op = ops;
	for (j = 0, o = 0; j < num_jobs; ++j, ++job) {
		fscanf(fp, " ops %d", &num_ops);

		job->id = j;
		job->num_ops = num_ops;
		job->ops = op;

		for (jo = 0; jo < num_ops; ++jo, ++o, ++op) {
			op->id = o;
			op->job = job;
			op->order = jo;
			fscanf(fp, "%d %d %d", &(op->type), &(op->proc_time), &(op->idle_time));
		}
	}

	fclose(fp);

	inst->num_types = num_types;
	inst->num_jobs = num_jobs;
	inst->num_ops = o;

	inst->types = types;
	inst->jobs = jobs;
	inst->ops = ops;

	// bind types to their operations
	for (t = 0, type = inst->types; t < num_types; ++t, ++type) {
		for (o = 0, op = inst->ops; o < inst->num_ops; ++o, ++op) {
			if (op->type == t) {
				type->ops[o] = 1;
			}
		}
	}

	return inst;
}

void destroy_inst(struct instance *inst)
{
	free(inst->types);
	free(inst->jobs);
	free(inst->ops);
	free(inst);
}

void print_inst(struct instance *inst)
{
	int t, j, o, to;
	printf("types: %d\n", inst->num_types);
	for (t = 0; t < inst->num_types; ++t) {
		printf("  type %d: %d (", t, inst->types[t].num_machines);
		for (to = 0; to < inst->num_ops; ++to) {
			printf(" %d", inst->types[t].ops[to]);
		}
		printf(" )\n");
	}
	printf("jobs: %d (ops: %d)\n", inst->num_jobs, inst->num_ops);
	for (j = 0; j < inst->num_jobs; ++j) {
		printf("  job %d: %d\n", j, inst->jobs[j].num_ops);
		for (o = 0; o < inst->jobs[j].num_ops; ++o) {
			struct operation *op = inst->jobs[j].ops + o;
			printf("    op %d: order %d, type %d, proc_time %d, idle_time %d\n",
				op->id, op->order, op->type, op->proc_time, op->idle_time);
		}
	}
}