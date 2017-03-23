#include "apue.h"
#include "locker.h"
#include "process.h"

int main()
{
    int ret = 0;
    sem sem1;
    locker locker1;
    cond cond1;
    err_msg("sizof sig = [%d]" ,sizeof(SIGTERM));

    char *logVal = NULL;
    logVal = getenv("UNIONLOG");
    printf("varPtr=[%X] \n",logVal);
    printf("var=[%s] \n",logVal);
    ret = access(logVal,R_OK|W_OK|X_OK);
    if(ret<0)
    {
        printf("access err ret=[%d] errno=[%d]  strerror=[%s]\n",ret,errno ,strerror(errno));
        return -1;
    }

    return 0;
}