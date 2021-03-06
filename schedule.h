#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "instance.h"

struct schedule {
	struct instance *inst;
	int num_types;
	struct machine_type *types;
	int makespan;
};

struct machine_type {
	int num_machines;
	struct machine *machines;
};

struct machine {
	int *op_start_times; // -1 if not scheduled
};

struct schedule *construct_schedule(struct instance *inst);
struct schedule *copy_schedule(struct schedule *sch);
void destroy_schedule(struct schedule *sch);
int validate_schedule(struct schedule *sch, int verbose);

void print_schedule(struct schedule *sch);
void write_schedule(struct schedule *sch);
/* Set len to 0 to print the entire schedule.
 */
void draw_schedule(struct schedule *sch, int start, int len);

#endif
