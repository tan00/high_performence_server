#include "apue.h"
#include "locker.h"
#include "process.h"

int log_to_stderr = 1;

int main()
{
    sem sem1;
    locker locker1;
    cond cond1;
    err_msg("sizof sig = [%d]" ,sizeof(SIGTERM));
    log_msg("test log_msg");

    
    return 0;
}