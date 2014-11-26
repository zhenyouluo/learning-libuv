#include <stdio.h>
#include <fcntl.h>
#include <uv.h>

void on_read(uv_fs_t *req);

uv_fs_t open_req;
uv_fs_t read_req;
uv_fs_t write_req;

static char buffer[1024];

static uv_buf_t iov;

void on_write(uv_fs_t *req) {
    uv_fs_req_cleanup(req);
    if (req->result < 0) {
        fprintf(stderr, "Write error: %s\n", uv_strerror((int) req->result));
    } else {
        //printf("%s\n", iov.base);
        uv_fs_read(uv_default_loop(), &read_req, open_req.result, &iov, 1, -1, on_read);
    }
}

void on_read(uv_fs_t *req) {
    uv_fs_req_cleanup(req);
    if (req->result < 0) {
        if (req->result == UV_EOF) {
            uv_fs_t close_req;
            // synchronous
            uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
        } else {
            fprintf(stderr, "Read error: %s\n", uv_strerror((int) req->result));
        }
    } else if (req->result > 0) {
        iov.len = req->result;
        uv_fs_write(uv_default_loop(), &write_req, 1, &iov, 1, -1, on_write);
    }
}

void on_open(uv_fs_t *req) {
    // uv_fs_req_cleanup(req);
    if (req->result != -1) {
        iov = uv_buf_init(buffer, sizeof(buffer));
        uv_fs_read(uv_default_loop(), &read_req, req->result,
                &iov, 1, -1, on_read);
    } else {
        fprintf(stderr, "error opening file: %s\n", uv_strerror((int) req->result));
    }
    uv_fs_req_cleanup(req);
}

int main(int argc, char **argv) {
    uv_fs_open(uv_default_loop(), &open_req, argv[1], O_RDONLY, 0, on_open);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    return 0;
}
