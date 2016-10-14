/** 
this program might leak memory; noticed by writer (https://nikhilm.github.io/uvbook)
test by telnet 127.0.0.1 13370
**/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/socket.h>
#include <uv.h>

#define DEFAULT_PORT  13370
#define DEFAULT_BACKLOG 1024

uv_loop_t *loop;

void alloc_buffer(uv_handle_t *h, size_t size, uv_buf_t *buf) {
    size = 1024;
    printf("alloc_buffer(%zd)\n", size);
    buf->base = malloc(size);
    buf->len = size;
}

void echo_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    printf("echo_read(stream=%p, nread=%zd)\n", stream, nread);
    if (nread == UV_EOF) {
        printf("<EOF>\n");
    }
    printf("| %s\n", buf->base);
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

int main() {
    loop = uv_default_loop();

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    struct sockaddr_in addr;
    
    uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);

    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
    int r = uv_listen((uv_stream_t*) &server, DEFAULT_BACKLOG, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
