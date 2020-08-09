#ifndef EXIT_STATUS_H
#define EXIT_STATUS_H

typedef enum exit_status {
    SUCCESS,
    CONFIG_FAIL,
    LOG_FAIL,
    SOCKET_BIND_FAILED,
    SOCKET_INIT_FAILED,
    LISTEN_FAILED,
    CONNECTION_FAIL

} exit_status;

#endif