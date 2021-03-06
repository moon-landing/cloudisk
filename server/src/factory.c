#include "../include/factory.h"

int factory_init(pFactory_t p,int threadNum,int queCapacity, MYSQL* pmysql, FILE* fp_log)
{
    bzero(p,sizeof(Factory_t));
    que_init(&p->que,queCapacity);//队列初始化
    pthread_cond_init(&p->cond,NULL);
    p->pthid=(pthread_t*)calloc(threadNum,sizeof(pthread_t));
    p->threadNum=threadNum;
    p->pmysql = pmysql;
    p->fp_log = fp_log;
    return 0;
}
//子线程的函数
void* threadFunc(void *p)
{
    pFactory_t pFac=(pFactory_t)p;
    pQue_t pq=&pFac->que;
    pNode_t pTask;
    while(1)
    {
        pthread_mutex_lock(&pq->queMutex);
        if(!pq->queSize)
        {
            pthread_cond_wait(&pFac->cond,&pq->queMutex);
        }
        que_get(pq,&pTask);//从队列里拿任务
        pthread_mutex_unlock(&pq->queMutex);
        //先接收客户端发送的任务
        recvCmd(pTask->newFd, pFac->pmysql, pFac->fp_log);
        //trans_file(pTask->newFd);
        close(pTask->newFd);
        free(pTask);
    }
}
//创建子线程，并把主要的数据结构传给每一个子线程
int factory_start(pFactory_t p)
{
    int i;
    if(!p->startFlag)
    {
        for(i=0;i<p->threadNum;i++)
        {
            pthread_create(p->pthid+i,NULL,threadFunc,p);
        }
        p->startFlag=1;
    }
    return 0;
}
