#include "process.h"
#include "apue.h"
#include "threadpool.h"


class worker
{
public:
    void process()
    {
        printf("111\n");
    }

};

int main()
{
    threadpool<worker>  thpool(16 ,1000);
    

    
}
