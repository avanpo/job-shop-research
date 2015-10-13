#ifndef INSTANCE_H
#define INSTANCE_H

#define MAX_OPERATIONS 5000

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
	int num_ops;
	int *ops;
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

int min_job_makespan(struct instance *inst);

void print_inst(struct instance *inst);
void print_inst_info(struct instance *inst);

#endif
