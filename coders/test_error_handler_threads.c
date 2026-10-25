#include "errors.h"
#include <stdio.h>

#define TEST_THREAD_COUNT 4
#define TEST_ITERATION_COUNT 100

typedef struct test_thread_s {
    char        *owner;
    uint32_t    iterations;
} test_thread_t;

static pthread_mutex_t callback_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32_t        callback_count;

static error_t test_callback(void **args)
{
    error_t result;

    result = *(error_t *)args;
    pthread_mutex_lock(&callback_mutex);
    callback_count++;
    pthread_mutex_unlock(&callback_mutex);
    return result;
}

static void *test_worker(void *arg)
{
    test_thread_t   *test;
    uint32_t        iteration;
    error_t         result;

    test = (test_thread_t *)arg;
    iteration = 0;
    while (iteration < test->iterations)
    {
        print("threaded test log", NULL);
        result = SUCCESS;
        run(test->owner, test_callback, (void **)&result);
        iteration++;
    }
    return NULL;
}

int main(void)
{
    pthread_t       threads[TEST_THREAD_COUNT];
    test_thread_t   tests[TEST_THREAD_COUNT];
    char            *owners[TEST_THREAD_COUNT] = {
        "worker-0", "worker-1", "worker-2", "worker-3"
    };
    uint32_t        created;
    uint32_t        i;

    print("__init__", NULL);
    created = 0;
    while (created < TEST_THREAD_COUNT)
    {
        tests[created].owner = owners[created];
        tests[created].iterations = TEST_ITERATION_COUNT;
        if (pthread_create(&threads[created], NULL, test_worker,
                &tests[created]) != 0)
        {
            fprintf(stderr, "pthread_create failed at worker %u\n", created);
            return 1;
        }
        created++;
    }
    i = 0;
    while (i < created)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            fprintf(stderr, "pthread_join failed at worker %u\n", i);
            return 1;
        }
        i++;
    }
    if (callback_count != TEST_THREAD_COUNT * TEST_ITERATION_COUNT)
    {
        fprintf(stderr, "expected %u callbacks, got %u\n",
            TEST_THREAD_COUNT * TEST_ITERATION_COUNT, callback_count);
        return 1;
    }
    printf("PASS: %u threads, %u print() calls, %u run() callbacks\n",
        TEST_THREAD_COUNT, TEST_THREAD_COUNT * TEST_ITERATION_COUNT,
        callback_count);
    return 0;
}