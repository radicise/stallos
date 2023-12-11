#include "kernel/types.h"
extern ssize_t write(int, const void*, size_t);
void _start() {
	write(1, "Stall v0.0.2.0\n", 15);
	while (1) {
	}
}
