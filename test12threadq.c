#include <stdio.h>
#include <uv.h>

#define FIB_UNTIL 10

uv_loop_t *loop;
uv_work_t reqs[FIB_UNTIL];

long Fibonacci(int n)
{
   if ( n == 0 )
      return 0;
   else if ( n == 1 )
      return 1;
   else
      return ( Fibonacci(n-1) + Fibonacci(n-2) );
} 

void fib(uv_work_t *req) {
    int n = *(int *) req->data;
    if (random() % 2)
        sleep(1);
    else
        sleep(3);
    long fib = Fibonacci(n);
    fprintf(stderr, "%dth fibonacci is %lu\n", n, fib);
}

void after_fib(uv_work_t *req, int status) {
  if (status == UV_ECANCELED)
    fprintf(stderr, "Calculation of %d cancelled.\n", *(int *) req->data);
  else
    fprintf(stderr, "Done calculating %dth fibonacci\n", *(int *) req->data);
}

void signal_handler(uv_signal_t *req, int signum)
{
    printf("Signal received!\n");
    int i;
    for (i = 0; i < FIB_UNTIL; i++) {
        uv_cancel((uv_req_t*) &reqs[i]);
    }
    uv_signal_stop(req);
}

int main() {
    loop = uv_default_loop();

    int data[FIB_UNTIL];
    int i;
    for (i = 0; i < FIB_UNTIL; i++) {
        data[i] = i;
        reqs[i].data = (void *) &data[i];
        uv_queue_work(loop, &reqs[i], fib, after_fib);
    }

    uv_signal_t sig;
    uv_signal_init(loop, &sig);
    uv_signal_start(&sig, signal_handler, SIGINT);
  
    return uv_run(loop, UV_RUN_DEFAULT);
}
