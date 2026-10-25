#include "errors.h"
#include "realloc.h"
#include "codexion.h"

#define OWNED_ERRORS_CHUNK 4096

#define RED "\033[0;31m"
#define NORMAL "\033[0m"
#define GREEN "\033[0;32m"

static void write_error(error_t error)
{
    if (error == SUCCESS)
        printf("%sSUCCESS%s", GREEN, NORMAL);
    else if (error >= MISSING_NB_CODERS && error <= UNKNOWENED_SHEDULER_TYPE)
        printf("%sParsing error...%s", RED, NORMAL);
    else if (error == HEAPQ_SIZE_TO_LOW)
        printf("%sHEAPQ error (max siz to low)%s", RED, NORMAL);
    else if (error == CODER_DIED)
        printf("%sStop error (a coder died)%s", RED, NORMAL);
    else if (error == GET_TIME_OF_DAY_ERROR)
        printf("%sgettimeofday() function failed%s", RED, NORMAL);
    else if (error == MALLOC_ERROR)
        printf("%smemory allocation's function failed%s", RED, NORMAL);
    else if (error == PTHREAD_ERROR)
        printf("%spthread lib failed%s", RED, NORMAL);
}

uint32_t print_log(uint32_t space, errors_t *errors, uint32_t log)
{
    uint32_t    i;
    uint32_t    y;
    uint32_t    len;

    i = 0;
    y = 0;
    if (log >= errors->logs_count)
        return 0;
    len = 0;
    while (errors->logs[log][y] && space > len)
    {
        if (errors->logs[log][y] == '%')
        {
            len += printf("%u", errors->logs_args[log][i++]);
            while (errors->logs[log][y] != ' ' && errors->logs[log][y])
                y++;
        }
        if (errors->logs[log][y] == '\n' && y++)
            continue;
        printf("%c", errors->logs[log][y++]);
        len++;
    }
    return len;
}

void error_table(errors_t *errors)
{
    uint32_t    line;
    uint32_t    colon;

    if (errors->owner_count == 0)
        return;
    line = 0;
    colon = 0;
    printf("\033[2J\033[H");
    while (line < SCREEN_SIZE_H)
    {
        colon = print_log(40, errors,
            (errors->logs_count > SCREEN_SIZE_H - 1) * (errors->logs_count - SCREEN_SIZE_H) + line);
        // colon += printf("  %u", errors->logs_count);
        while (colon++ < 40)
            printf(" ");
        // is on a sep
        if (line % (SCREEN_SIZE_H / errors->owner_count) == 0 && line != 0)
        {
            printf("+");
            while (colon++ < SCREEN_SIZE_W)
                printf("-");
        }
        else
        {
            printf("|");
            colon++;
            if ((line % (SCREEN_SIZE_H / errors->owner_count) == 1 || line == 0) && line / (SCREEN_SIZE_H / errors->owner_count - 1) < errors->owner_count)
                printf("%s", errors->owners[line / (SCREEN_SIZE_H / errors->owner_count - 1)]);
            else if (errors->owned_errors && errors->owned_errors_length
                && errors->owned_errors[line / (SCREEN_SIZE_H / errors->owner_count)]
                && line % (SCREEN_SIZE_H / errors->owner_count) > 1
                && errors->owned_errors_length[line / (SCREEN_SIZE_H / errors->owner_count)] >= line % (SCREEN_SIZE_H / errors->owner_count) - 1)
            {
                printf("call nb %u: ", errors->owned_errors_length[line / (SCREEN_SIZE_H / errors->owner_count)] - (line % (SCREEN_SIZE_H / errors->owner_count) - 1));
                write_error(errors->owned_errors[line / (SCREEN_SIZE_H / errors->owner_count)][
                    errors->owned_errors_length[line / (SCREEN_SIZE_H / errors->owner_count)] - (line % (SCREEN_SIZE_H / errors->owner_count) - 1)
                ]);
            }
        }
        printf("\n");
        line++;
    }
}

void *manager_runtime(void *arg)
{
    errors_t    *errors;

    errors = (errors_t *)arg;
    if (!errors)
        return NULL;
    while (1)
    {
        pthread_mutex_lock(&errors->manager_mutex);
        if (errors->logs_count > 0 || errors->owner_count > 0)
            error_table(errors);
        pthread_mutex_unlock(&errors->manager_mutex);
        usleep(10000);
    }
    return NULL;
}

static errors_t *init_manager(void)
{
    static errors_t    *manager;

    if (manager != NULL)
        return manager;
    manager = (errors_t *)malloc(sizeof(errors_t));
    if (!manager)
        return NULL;
    pthread_mutex_init(&manager->manager_mutex, NULL);
    manager->owners = NULL;
    manager->owner_count = 0;
    manager->owned_errors = NULL;
    manager->owned_errors_length = NULL;
    manager->logs_count = 0;
    manager->max_logs = 2;
    manager->logs = (char **)malloc(sizeof(char *) * manager->max_logs);
    manager->logs_args = (uint32_t **)malloc(sizeof(void *) * manager->max_logs);
    if (!manager->logs || !manager->logs_args)
        return NULL;
    pthread_create(&manager->thread, NULL, manager_runtime, manager);
    return manager;
}

void run(char *owner, error_t (* func)(void **), void **args)
// use args to insert manager with the __init__ owner
{
    static errors_t *errors;
    error_t         err;
    uint32_t        i;
    uint32_t        len_owner;
    uint32_t        j;

    if (!owner)
        return;
    if (strcmp("__init__", owner) == 0)
    {
        // errors = (errors_t *)args;
        errors = init_manager();
        return;
    }
    if (!errors)
    {
        write(2, "Error manager used not initialized\n", 35);
        errors = init_manager();
        if (!errors)
            return;
    }
    if (!func)
        return;

    err = func(args);
    pthread_mutex_lock(&errors->manager_mutex);
    i = 0;
    while (i < errors->owner_count)
    {
        if (strcmp(errors->owners[i], owner) == 0)
            break;
        i++;
    }
    if (i == errors->owner_count)
    {
        errors->owners = (char **)my_realloc(errors->owners,
            errors->owner_count * sizeof(char *),
            (errors->owner_count + 1) * sizeof(char *));
        errors->owned_errors = (error_t **)my_realloc(errors->owned_errors,
            errors->owner_count * sizeof(error_t *),
            (errors->owner_count + 1) * sizeof(error_t *));
        errors->owned_errors_length = (uint32_t *)my_realloc(errors->owned_errors_length,
            errors->owner_count * sizeof(uint32_t),
            (errors->owner_count + 1) * sizeof(uint32_t));
        if (!errors->owners || !errors->owned_errors || !errors->owned_errors_length)
        {
            pthread_mutex_unlock(&errors->manager_mutex);
            return;
        }
        len_owner = strlen(owner);
        errors->owners[i] = (char *)malloc(len_owner + 1);
        if (errors->owners[i])
        {
            j = 0;
            while (j < len_owner)
            {
                errors->owners[i][j] = owner[j];
                j++;
            }
            errors->owners[i][j] = '\0';
        }
        errors->owned_errors[i] = NULL;
        errors->owned_errors_length[i] = 0;
        errors->owner_count++;
    }

    if (errors->owned_errors_length[i] % OWNED_ERRORS_CHUNK == 0)
        errors->owned_errors[i] = (error_t *)my_realloc(errors->owned_errors[i],
            errors->owned_errors_length[i] * sizeof(error_t),
            (errors->owned_errors_length[i] + OWNED_ERRORS_CHUNK) * sizeof(error_t));
    if (errors->owned_errors[i])
    {
        errors->owned_errors[i][errors->owned_errors_length[i]] = err;
        errors->owned_errors_length[i]++;
    }
    pthread_mutex_unlock(&errors->manager_mutex);
}

void print(char *msg, uint32_t *args, uint32_t args_count)
// prints to stdint !
// message "__init__", defines the manager ptr and launches a new thread !
{
    static errors_t *manager;
    uint32_t        *args_cpy;

    if (!msg)
        return;
    // verifie manager is ready
    if (strcmp(msg, "__init__") == 0)
    {
        manager = init_manager();
        run("__init__", NULL, (void **)manager);
        return;
    }
    if (manager == NULL)
    {
        write(2, "Error manager used not initialized\n", 35);
        manager = init_manager();
        run("__init__", NULL, (void **)manager);
        if (!manager)
            return;
    }

    pthread_mutex_lock(&manager->manager_mutex);
    if (manager->logs_count >= manager->max_logs)
    {
        uint32_t new_max = manager->max_logs * 2;
        if (new_max == 0)
            new_max = 2;
        manager->logs = (char **)my_realloc(manager->logs,
            manager->logs_count * sizeof(char *),
            new_max * sizeof(char *));
        manager->logs_args = (uint32_t **)my_realloc(manager->logs_args,
            manager->logs_count * sizeof(void *),
            new_max * sizeof(void *));
        manager->max_logs = new_max;
    }
    if (manager->logs && manager->logs_args)
    {
        manager->logs[manager->logs_count] = msg;
        args_cpy = (uint32_t *)malloc(sizeof(uint32_t) * args_count);
        while (args_count--)
            args_cpy[args_count] = args[args_count];
        manager->logs_args[manager->logs_count] = (uint32_t *)args_cpy;
        manager->logs_count++;
    }
    pthread_mutex_unlock(&manager->manager_mutex);
}