#include <stdio.h>
#include <uv.h>

uv_loop_t *loop;
uv_process_t child_req;
uv_process_options_t options;

void on_exit(uv_process_t *req, int64_t exit_status, int term_signal) {
    fprintf(stderr, "Process exited with status %d, signal %d\n", exit_status, term_signal);
    uv_close((uv_handle_t*) req, NULL);
}

int main() {
    loop = uv_default_loop();

    char* args[3];
    args[0] = "./uvprocstreamtest";
    args[1] = NULL;
    args[2] = NULL;

    options.stdio_count = 3;
    uv_stdio_container_t child_stdio[3];
    child_stdio[0].flags = UV_IGNORE;
    child_stdio[1].flags = UV_IGNORE;
    child_stdio[2].flags = UV_INHERIT_FD;
    child_stdio[2].data.fd = 2;
  
    options.stdio = child_stdio;
    options.exit_cb = on_exit;
    options.file = args[0];
    options.args = args;

    int r;
    if ((r = uv_spawn(loop, &child_req, &options))) {
        fprintf(stderr, "%s\n", uv_strerror(r));
        return 1;
    } else {
        fprintf(stderr, "Launched process with ID %d\n", child_req.pid);
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}
