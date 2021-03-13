#include "life.h"
#include <pthread.h>
#include <vector>
#include <stdio.h>

struct DataRange
{
    int cell_start, cell_stop;
};
struct LifeArgs
{
    int steps;
    LifeBoard &evenState, &oddState;
    DataRange dr;
    pthread_barrier_t *barrier;
};

void *RunLife(void *args) {
    printf("starting RunLife \n");
    struct LifeArgs *myArgs;
    myArgs = (struct LifeArgs *) args;
    int steps = myArgs->steps;
    LifeBoard evenState = myArgs->evenState;
    LifeBoard oddState = myArgs->oddState;
    DataRange dr = myArgs->dr;
    pthread_barrier_t *barrier = myArgs->barrier;

    int cs, cf;
    cs = dr.cell_start;
    cf = dr.cell_stop;
    printf("cs=%d, cf=%d, drIndex=%p\n", cs, cf, (void*)&dr);

    for(int step = 0; step < steps; ++step) {
        //wait for all other threads to finish writing.
        pthread_barrier_wait(barrier);
    }
    return NULL;
}

std::vector<DataRange> divideBoard(int threads, LifeBoard &state) {
    int h, w;
    // get the height and width of the 
    // values that we will be looking at
    h = state.height() - 2;
    w = state.width() - 2;
    int total_cells = h * w;
    int cells_per_thread = total_cells/threads;
    int cells_left_over = total_cells%threads;

    std::vector<DataRange> drs;
    // The starting cell should be the cell at position (1,1)
    int cur_cell = state.width() + 1;

    for(int i = 0; i < threads; i++) {
        DataRange dr;
        dr.cell_start = cur_cell;
        // avoid assigning edge cells to a thread
        int rows_crossed = total_cells/w;
        int new_cell = cur_cell + cells_per_thread + (2*rows_crossed);
        // take care of unevenly divided squares
        if(cells_left_over > 0) {
            ++new_cell;
            --cells_left_over;
        }
        dr.cell_stop = new_cell;
        drs.push_back(dr);

        cur_cell = new_cell + 1;

    }

    return drs;
}

void simulate_life_parallel(int threads, LifeBoard &state, int steps) {
    /* YOUR CODE HERE */
    LifeBoard oddState{state.width(), state.height()};

    std::vector<pthread_t> all_threads;
    int rc, t;

    // Make the Barrier
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, threads);

    std::vector<DataRange> drs = divideBoard(threads, state);
    for(t = 0; t < threads; t++) {
        DataRange dr = drs[t];
        LifeArgs args = {steps, state, oddState, dr, &barrier};
        pthread_t thread = 0;
        rc = pthread_create(&thread, NULL, RunLife, &args);
        if (rc){
          printf("ERROR; return code from pthread_create() is %d\n", rc);
          exit(-1);
        }
        all_threads.push_back(thread);
        // printf("made it through the for loop %d times\n", t);

    }
    pthread_barrier_destroy(&barrier);
    for (pthread_t thread : all_threads) {
        pthread_cancel(thread);
        pthread_join(thread, NULL);
    }
}
