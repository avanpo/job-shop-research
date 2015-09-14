#ifndef INSTANCE_H
#define INSTANCE_H

#define MAX_TYPES              10
#define MAX_MACHINES_PER_TYPE  10
#define MAX_JOBS               10
#define MAX_OPERATIONS         500
#define MAX_OPERATIONS_PER_JOB 50

struct instance {
	int num_types;
	struct type *types[MAX_TYPES];
	int num_jobs;
	struct job *jobs[MAX_JOBS];
	int num_ops;
	struct operation *ops[MAX_OPERATIONS];
};

struct type {
	int id;
	int num_machines;
};

struct job {
	int id;
	int num_ops;
	struct operation *ops[MAX_OPERATIONS_PER_JOB];
};

struct operation {
	int id;
	int job_id;
	int type_id;
	int proc_time;
	int idle_time;
};

#endif
