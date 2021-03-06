#include "life.h"
#include <pthread.h>
#include <vector>
#include <stdio.h>


struct Position
{
    int x, y;
};
struct DataRange 
{
    std::vector<Position> indeces;
};
struct LifeArgs
{
    int *steps;
    LifeBoard *evenState, *oddState;
    std::vector<Position> dr;
    pthread_barrier_t *barrier;
    int id;
};

void *RunLife(void *args) {
    struct LifeArgs *myArgs;
    myArgs = (struct LifeArgs *) args;
    int steps = *myArgs->steps;
    LifeBoard *evenState = myArgs->evenState;
    LifeBoard *oddState = myArgs->oddState;
    std::vector<Position> dr = myArgs->dr;
    pthread_barrier_t *barrier = myArgs->barrier;
    // int id = myArgs->id;

    // printf("starting RunLife for thread %d\n", id);

    for(int step = 0; step < steps; step++) {
        int x, y;
        bool state_is_even = (step % 2 == 0);
        struct LifeBoard *state;
        struct LifeBoard *next_state;
        if(state_is_even) {
            state = evenState;
            next_state = oddState;
        } else {
            state = oddState;
            next_state = evenState;
        }
        for(Position p : dr) {
            int live_in_window = 0;
            x = p.x;
            y = p.y;
            /* For each cell, examine a 3x3 "window" of cells around it,
             * and count the number of live (true) cells in the window. */
            for (int y_offset = -1; y_offset <= 1; ++y_offset) {
                for (int x_offset = -1; x_offset <= 1; ++x_offset) {
                    if (state->at(x + x_offset, y + y_offset)) {
                        ++live_in_window;
                    }
                }
            }
            /* Cells with 3 live neighbors remain or become live.
                Live cells with 2 live neighbors remain live. */
            next_state->at(x, y) = (
                live_in_window == 3 /* dead cell with 3 neighbors or live cell with 2 */ ||
                (live_in_window == 4 && state->at(x, y)) /* live cell with 3 neighbors */
            );

        }
        // printf("thread %d started waiting on barrier\n", id);
        pthread_barrier_wait(barrier);
        // printf("thread %d finished waiting on barrier\n", id);
    }
    // printf("finishedRunLife for thread %d\n", id);
    return ((void*) NULL);
}

std::vector<DataRange> divideBoard(int threads, LifeBoard &state) {
    int h, w;
    h = state.height() - 2;
    w = state.width() - 2;
    int total_cells = h * w;
    // handle the case in which there are more threads than cells
    if (total_cells < threads) threads = total_cells;
    int cells_per_thread = total_cells/threads;
    int cells_left_over = total_cells%threads;

    int count = 0;
    int cells_for_cur_thread = cells_per_thread;
    if(cells_left_over != 0){
        cells_for_cur_thread++;
        cells_left_over--;
    }
    std::vector<DataRange> threadRanges;
    struct DataRange curRange;
    for (int y = 1; y < state.height() - 1; ++y) {
        for (int x = 1; x < state.width() - 1; ++x) {
            struct Position p = {x, y};
            // If we need to start making a new thread range
            if(count >= cells_for_cur_thread) {
                count = 0;

                cells_for_cur_thread = cells_per_thread;
                if(cells_left_over != 0){
                    cells_for_cur_thread++;
                    cells_left_over--;
                }
                threadRanges.push_back(curRange);

                struct DataRange newRange;
                curRange = newRange;
            }

            curRange.indeces.push_back(p);

            count++;
        }
    }
    threadRanges.push_back(curRange);
    if((int)threadRanges.size() != threads) {
        printf("threadRange size=%ld, threads=%d", threadRanges.size(), threads);
    }
    return threadRanges;
}


void simulate_life_parallel(int threads, LifeBoard &state, int steps) {
    /* YOUR CODE HERE */
    LifeBoard oddState{state.width(), state.height()};

    int rc, t;

    // Make the Barrier
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, threads);

    // Divide the array into chunks for each thread to work on
    std::vector<DataRange> drs = divideBoard(threads, state);
    // Account for case where there are more threads than cells
    threads = drs.size();

    // Make a vector of args so args do not go out of scope
    // Make a vector of threads for the same reason
    std::vector<LifeArgs> argList;
    std::vector<pthread_t> all_threads;
    for(t = 0; t < threads; ++t) {
        DataRange dr = drs[t];
        struct LifeArgs args = {&steps, &state, &oddState, dr.indeces, &barrier, t};
        argList.push_back(args);
        pthread_t thread = 0;
        all_threads.push_back(thread);
        // printf("thread %d has been assigned %ld cells\n", t, dr.indeces.size());
    }

    for(t = 0; t < threads; ++t) {

        rc = pthread_create(&all_threads.at(t), NULL, RunLife, &argList.at(t));
        if (rc){
          printf("ERROR; return code from pthread_create() is %d\n", rc);
          exit(-1);
        }
        // printf("made it through the for loop %d times\n", t);

    }
    // Join all of the threads that have been created
    for (pthread_t thread : all_threads) {
        // printf("started joining %p \n", (void*)&thread);
        rc = pthread_join(thread, NULL);
        // printf("finished joining %p \n", (void*)&thread);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }
    // Destroy the barrier
    pthread_barrier_destroy(&barrier);

    // Determine if evenState or oddState should be returned
    bool steps_is_odd = (steps % 2 == 1);
    if(steps_is_odd){
        swap(state, oddState);
    }
}
