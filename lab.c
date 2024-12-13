#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ############################################################
 * #         #             ###         #####       o          #
 * #i      ####         ###  ###      ####################    #
 * #                                                          #
 * #       #   ##          ######                ##############
 * ############################################################
 *
 * i - input
 * o - output
 * # - wall
 *   - free space
 */

int * lab; // maze
size_t W;
size_t H;
size_t cap;
size_t counter = 0;

#define CELL(x, y) lab[(y)*W+(x)]
#define VALID(dx, dy) ((dx)*(dx)+(dy)*(dy) == 1 || (dx)*(dx)+(dy)*(dy) == 2)
#define INBOUNDS(x, y) ((x) >= 0 && (x) < W && (y) >= 0 && (y) <H)
#define METRIC(dx, dy) ((dx)*(dx) + (dy)*(dy) == 1 ? 10 : 14)

typedef struct
{
	int x;
	int y;
} Point;

typedef struct _queue {
	Point pt;
	struct _queue * next;
} Queue;

int queueAdd (Queue** queue, Point pt) {
	Queue* n = malloc(sizeof(Queue));
	if (!*queue) {
		*n = (Queue){pt, n};
	}
	else {
		*n = (Queue){pt, (*queue)->next};
		(*queue)->next = n;
	}
	*queue = n;
	return 0;
}

int queuePop (Queue** q, Point * pt) {
	if(!*q) return -1;
	*pt = (*q)->next->pt;
	Queue * old = (*q)->next;
	if (old == (*q))
		*q = NULL;
	else
		(*q)->next = old->next;
	free(old);
	return 0;
}

Point start;
Point end;
int readLab(FILE * f) {
	H = 0;
	W = 0;
	char buf[1024];
	cap = 1024;
	lab = calloc(cap, sizeof(int));
	while (fgets(buf, sizeof(buf), f)) {
		if ( H == 0 ) W = strlen(buf);
		if ((H+1) * W >= cap) lab = realloc(lab, sizeof(int)*(cap*=2));
		for (int i = 0; i < W; i++) {
			switch(buf[i]) {
				case 'i':
					start = (Point){i, H};
					CELL(i, H) = 0;
					break;
				case 'o':
					end = (Point){i, H};
					CELL(i, H) = 0;
					break;
				case ' ':
					CELL(i, H) = 0;
					break;
				default:
					CELL(i, H) = -1;
					break;
				}
		}
		H++;
	}

	return 0;
}

int printLab() {
	for(int y = 0; y < H; y++) {
		for (int x = 0; x < W; x++) {
			if (x == start.x && y == start.y) {
				fputs("i ", stdout);
			}
			else if (x == end.x && y == end.y) {
				fputs("o ", stdout);
			}
			else {
				switch(CELL(x, y)) {
					case 0: fputs("  ", stdout); break;
					case -1: fputs("##", stdout); break;
					case -'w': printf("\033[32mw\033[0m "); break;
					default: printf("  "); break;
				}
			}
		}
		fputc('\n', stdout);
	}
}

int shortedPath(Queue ** q) {
	if (*q) {
		Point pt;
		queuePop(q, &pt);
		printf("IN x = %d, y = %d, num = %d\n", pt.x, pt.y, CELL(pt.x, pt.y));
		//printf("3\n");
		int dist = CELL(pt.x, pt.y);
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				Point n = {pt.x + dx, pt.y + dy};
				if (VALID(dx, dy) && INBOUNDS(n.x, n.y) ) {
					if (CELL(n.x, n.y) != -1 && (CELL(n.x, n.y) == 0 || CELL(n.x, n.y) > dist + METRIC(dx, dy))) {
						CELL(n.x, n.y) = dist + METRIC(dx, dy);
						printf("ADDED: x = %d, y = %d\n", n.x, n.y);
						counter++;
						queueAdd(q, n);
					}
				}
			}
		}
		return shortedPath(q);
	}
}

int markWay(int x, int y) {
	int min = -1;
	int minX = 0;
	int minY = 0;
	for (int dx = -1; dx <= 1; dx++) {
		for (int dy = -1; dy <= 1; dy++) {
	       		Point n = {x + dx, y + dy};
			if (VALID(dx, dy) && INBOUNDS(n.x, n.y)) {
				if (CELL(n.x, n.y) != -1 && CELL(n.x, n.y) != -'w' && (min == -1 || CELL(n.x, n.y) < min)) {
					minX = n.x;
					minY = n.y;
					min = CELL(n.x, n.y);
				}
			}
		}
	}

	if (min == -1) {
		printf("cannot print the way for x = %d, y = %d\n", x, y);
		return -1;
	}
	else if (min == 0) {
		return 0;
	}

	else {
		printf("Moved to x = %d, y = %d, min = %d\n", minX, minY, min);
		CELL(minX, minY) = -'w';
		return markWay(minX, minY);
	}

	return -1;
}



int main(int argc, const char * argv[]) {
	if (argc > 1) {
		FILE * f = fopen(argv[1], "r");
		if (!f) return -fprintf(stderr, "CANNOT FILE\n");
		if (!readLab(f)) {
			printLab();
			Queue * q = NULL;
			queueAdd(&q, start);
			shortedPath(&q);
			if (CELL(end.x, end.y) == 0)
				printf("CANNOT FIND THE WAY\n");
			else {
				printf("FOUND OPTIMAL WAY: lenght of way is %d\n", CELL(end.x, end.y));
				CELL(start.x, start.y) = 0;
				markWay(end.x, end.y);
			}
			printLab();
			printf("Total adds: %lu\n", counter);
			free(lab);
		}
	}

	return 0;
}


