#include "apue.h"
#include "locker.h"
#include "process.h"

int main()
{
    sem sem1;
    locker locker1;
    cond cond1;
    err_msg("sizof sig = [%d]" ,sizeof(SIGTERM));

    return 0;
}