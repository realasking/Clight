#include "../inc/utils.h"

/*
 * pointers to poll callback functions;
 * MODULES_NUM - 1 as dpms does not generate poll i/o
 */
static void (*poll_cb[MODULES_NUM - 1])(void);

/**
 * Create timer and returns its fd to
 * the main struct pollfd
 */
int start_timer(int clockid, int initial_timeout) {
    int timerfd = timerfd_create(clockid, 0);
    if (timerfd == -1) {
        ERROR("could not start timer: %s\n", strerror(errno));
        state.quit = 1;
    } else {
        set_timeout(initial_timeout, 0, timerfd, 0);
    }
    return timerfd;
}

/**
 * Helper to set a new trigger on timerfd in $start seconds
 */
void set_timeout(int sec, int nsec, int fd, int flag) {
    struct itimerspec timerValue = {{0}};

    timerValue.it_value.tv_sec = sec;
    timerValue.it_value.tv_nsec = nsec;
    timerValue.it_interval.tv_sec = 0;
    timerValue.it_interval.tv_nsec = 0;
    int r = timerfd_settime(fd, flag, &timerValue, NULL);
    if (r == -1) {
        ERROR("%s\n", strerror(errno));
        state.quit = 1;
    }
}

void set_pollfd(int fd, enum modules module, void (*cb)(void)) {
    if (fd == -1) {
        state.quit = 1;
        return;
    }

    main_p[module].fd = fd;
    poll_cb[module] = cb;
}

void run_callback(enum modules module) {
    if (poll_cb[module]) {
        poll_cb[module]();
    }
}