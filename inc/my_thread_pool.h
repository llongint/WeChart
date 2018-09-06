#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_


#include <pthread.h>
#include <sys/epoll.h>

#define MAX_WAITING_TASKS	1000
#define MAX_ACTIVE_THREADS	20

#define BUFSIZE			100
#define PATHSIZE		100

#ifndef bool
#define bool char
#endif // !bool

#ifndef ture
#define ture 1
#endif // !true

#ifndef false
#define false 0
#endif // !false

struct task//任务结点
{
	int (*do_task)(int epfd,struct epoll_event *events); //函数指针，指向任务要执行的函数
	int epfd;  //一个指针，任务执行函数时作业函数的参数传入
    struct epoll_event *events;
	struct task *next;
};

typedef struct thread_pool  //线程池头结点
{
	pthread_mutex_t lock;   //互斥锁，用来保护这个"线程池"，就是保护这个链表的　
	pthread_cond_t  cond;   //　有任务的条件　

	bool shutdown;          //是否退出。

	struct task *task_list; //任务链表，即指向第一个任务结点

	pthread_t *tids;        //指向线程ID的数组，因为我可能会创建多个线程。

	unsigned max_waiting_tasks;//表示最大的执行的任务数
	unsigned waiting_tasks; //目前正在链表上的任务数，即待执行任务
	unsigned active_threads;//正在服役的线程数
}thread_pool;


bool init_pool(thread_pool *pool, unsigned int threads_number);
bool add_task(thread_pool *pool, int (*do_task)(int epfd,struct epoll_event *events),
                                 int epfd,struct epoll_event *events);
int  add_thread(thread_pool *pool, unsigned int additional_threads_number);
int  remove_thread(thread_pool *pool, unsigned int removing_threads_number);
bool destroy_pool(thread_pool *pool);
void *routine(void *arg);//任务执行函数
void handler(void *arg);

#endif
