#ifndef INSTANCE_H
#define INSTANCE_H

#define MAX_OPERATIONS         500

struct instance {
	int num_types;
	struct type *types;
	int num_jobs;
	struct job *jobs;
	int num_ops;
	struct operation *ops;
};

struct type {
	int id;
	int num_machines;
	int ops[MAX_OPERATIONS];
};

struct job {
	int id;
	int num_ops;
	struct operation *ops;
};

struct operation {
	int id;
	struct job *job;
	int order;
	int type;
	int proc_time;
	int idle_time;
};

struct instance *read_inst(char *fname);
void destroy_inst(struct instance *inst);

void print_inst(struct instance *inst);

#endif
