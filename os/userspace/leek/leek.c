#include "leek.h"

extern void *brk(void*);

static unsigned char SMALL  = 128;
static unsigned char FILLED = 64;

static void *bm = NULL;

static char *slot0 = NULL;
static char *slot1 = NULL;
static char *slot2 = NULL;
static char *slot3 = NULL;
static char *slot4 = NULL;
static char *slot5 = NULL;
static char *slot6 = NULL;

static char *big = NULL;

void *malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    if (size > 64) {
        if (!big) create_new: {
            // case 1: no unfilled bins
            if (!bm) {
                bm = brk(NULL);
            }
            char *h1 = bm;
            bm = (char *)bm + (4-sizeof(h1)) - (uintptr)bm % 4; // align
            size_t *s1 = (size_t *)(h1 + 1);
            void *d = s1+1;
            size_t *s2 = (size_t *)((char *)d + size);
            char *h2 = (char *)(s2+1);
            
            bm = brk((char *)h1+2*sizeof(char)+2*sizeof(size_t)+size);
            *s1 = size;
            *s2 = size;
            *h1 = FILLED;
            *h2 = FILLED;
            return d;
        } else {
            // case 2: there is at least one unfilled bin, let's search for one that is correctly sized
            char *b = big;
            char *before = NULL;
            // find a bin that is large enough
            while (b && *(size_t *)(b+1) < size) {
                before = b;
                b = *(char **)(b+1+sizeof(size_t));
            }
            // if the bin is large enough, use it accordingly
            if (b) {
                // mark it as filled
                *b |= FILLED;
                *(b+1+2*sizeof(size_t)+size) |= FILLED;

                *(char **)(before+1+sizeof(size_t)) = *(char **)(b+1+sizeof(size_t));

                return b+1+sizeof(size_t);
            } else {
                // create a new bin
                goto create_new;
            }
        }
    }
    
    char rounded = 0;
    char align = 1;
    char **slot = NULL;
    switch (size) {
    case 1:
      {
        align = 0;
        rounded = 1;
        slot = &slot0;
        break;
      }
    case 2:
      {
        align = 0;
        rounded = 2;
        slot = &slot1;
        break;
      }
    case 3:
    case 4:
      {
        rounded = 4;
        slot = &slot2;
        break;
      }
    case 5:
    case 6:
    case 7:
    case 8:
      {
        rounded = 8;
        slot = &slot3;
        break;
      }
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      {
        rounded = 16;
        slot = &slot4;
        break;
      }
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
      {
        rounded = 32;
        slot = &slot5;
        break;
      }
    default:
      {
        rounded = 64;
        slot = &slot6;
        break;
      }
    }

    if (!*slot) {
        // case 1: no unfilled bins
        if (!bm) {
            bm = brk(NULL);
        }
        char *h1 = bm;
        if (align) {
            h1 += (4-sizeof(h1)) - (uintptr)bm % 4;
        }
        void *d = h1+1;
        char *h2 = h1+1+rounded;
        bm = brk(h2+1);

        *h1 |= SMALL | FILLED | rounded;
        *h2 = *h1;

        return d;
    } else {
        // case 2: there is an unfilled bin
        if (rounded >= sizeof(char *)) {
            *slot = *(char **)(*slot+1);
        }
        return *slot+1;
    }
}

#ifdef TESTING

int main(int argc, char **argv) {
    while (1) {
        puts("----------");
        printf("bm: %p\n", bm);
        char c;
        scanf("%c", &c);

        if (c == 'm') {
            int size;
            scanf("%d", &size);

            void *ptr = malloc(size);
            printf("==> %d\n", ptr);
        } else if (c == 'f') {
            void *ptr;
            scanf("%p", &ptr);
            /* free(ptr); */
        } else {
            puts("Try again...");
        }
    }
    return 0;
}

#endif
