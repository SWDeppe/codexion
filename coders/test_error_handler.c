#include "errors.h"

int main(void)
{
    errors_t    manager = {0};
    char        *owners[] = {"test"};
    error_t     test_errors[] = {SUCCESS, CODER_DIED};
    error_t     *owned_errors[] = {test_errors};
    uint32_t    owned_errors_length[] = {2};
    char        *logs[] = {"test log: coder %u", "second test log"};
    uint32_t    first_log_args[] = {42};
    uint32_t    *logs_args[] = {first_log_args, NULL};

    manager.owners = owners;
    manager.owner_count = 1;
    manager.owned_errors = owned_errors;
    manager.owned_errors_length = owned_errors_length;
    manager.logs = logs;
    manager.logs_args = logs_args;
    manager.logs_count = 2;
    error_table(&manager);
    return 0;
}