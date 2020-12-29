/**
 * 15-213 Proclab Student's Solution File
 * @author <your name here>
 * @andrewid <andrew ID here>
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

extern char **environ; /* Defined by libc */

void sleep_millisecond(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1e3;
    ts.tv_nsec = (milliseconds % (int)1e3) * 1e6;
    nanosleep(&ts, NULL);
}

/* The type for signal handler functions
 * You can use this. */
typedef void (*sighandler_t)(int);

int is_timeout = 0;

/*****************************************************************************\
*******************************************************************************
**  Timer Puzzle
*******************************************************************************
\*****************************************************************************/
/**
 *  This function reads a line from stdin, waiting indefinitely until a newline
 *  character is encountered.
 *  You are expected to call this to actually read a line from stdin
 *
 *  @param buf Buffer to hold the line
 *
 *  @param buf_size Size of the buffer
 *
 *
 *  @return -1 if interrupted by a signal or if there is an error
 *          otherwise the number of characters read, including the newline
 *          character (if present), but excluding the null-terminator.
 *
 *  @remarks  This function will read everything from STDIN until the newline
 *            character is encountered.
 *            The buffer will contain the newline character 
 *            and null-terminator.
 *            However, if there is insufficient space, then 
 *            the buffer will store a truncated version of the line read,
 *            and the newline character will *not* be present.
 *            However, in any case, the buffer will still be null-terminated.
 *                   
 *  */
ssize_t mygets(char *buf, int buf_size);

void alarm_sig_hander(int sig) {
    int olderrno = errno;

    if (sig == SIGALRM) {
        is_timeout = 1;
    }

    errno = olderrno;
}

/**
 * Use this to set the alarm signal handler.
 * @param alarm_handler  Address of the Signal handler for SIGALRM
 * @return SIG_ERR on error; otherwise the address of the previous handler
 */
sighandler_t set_alarm_handler(sighandler_t alarm_handler) {
    struct sigaction action, old_action;

    action.sa_handler = alarm_handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */

    /* Make sure you do not specify SA_RESTART,
    SA_RESTART will cause IO functions to be restarted when
    they are interrupted by signals. */
    action.sa_flags = 0;

    if (sigaction(SIGALRM, &action, &old_action) < 0) {
        return SIG_ERR;
    }

    return old_action.sa_handler;
}

/**
 * Reads a line from STDIN within a timeout period.
 *
 * @param buf Pointer to buffer to hold the line read from STDIN,
 *        including the newline character.
 *
 * @param buf_size Size of the buffer, including the space for holding the
 *        newline and null-terminator character. If there is insufficient 
 *        space, then buffer will store a truncated version of the line. 
 *        In whatever case, buffer must be null-terminated.
 *
 * @param timeout_secs Number of seconds before timing out;
 *        Can be zero if no timeout is desired, in which case this
 *        function should wait indefinitely  for a line of input.
 *
 * @return -1 if interrupted by a signal or if there is an error.
 *         Number of characters read (including the newline char if present
 *            but excluding the null-terminator) if a line is read 
 *             before timing out.
 *         0 if timeout happened before finishing reading a line.
 *
 *  @remarks Note that reading a line means read everything from STDIN until
 *           the newline character is encountered. 
 *           So if 'timeout_secs' seconds has passed *before* a newline char is
 *           encountered, then this function must return 0.
 *           If a newline char is encountered within 'timeout_secs' seconds,
 *           then this function must return the number of characters read. 
 *
 *           IMPORTANT!!!: This function should restore the original alarm 
 *           signal handler before returning!
 *
 * */
ssize_t tgets(char *buf, int buf_size, int timeout_secs) {
    ssize_t num_read = 0;

    /* TODO: set up your alarm handler here */
    sighandler_t old_hander = set_alarm_handler(alarm_sig_hander);

    /* Use this to read a line */
    alarm(timeout_secs);
    num_read = mygets(buf, buf_size);

    /* TODO: restore alarm handler */
    set_alarm_handler(old_hander);

    /* TODO: check the return value of mygets */
    if (num_read == -1 && errno == EINTR) {
        return is_timeout ? 0 : -1;
    }

    /* TODO: change the return value  */
    return num_read;
}

/*****************************************************************************\
*******************************************************************************
**  Racer Puzzle
*******************************************************************************
\*****************************************************************************/

/* helper functions */
void printAletter();

/* @param pid - the pid of the child process*/
void racer1(int pid) {
    sleep_millisecond(10);
    /*do not remove this line*/
    printAletter();
}

void setup_race2() {
    sigset_t mask_one, prev_one;

    sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD);

    // block SIGCHLD before fork
    sigprocmask(SIG_BLOCK, &mask_one, &prev_one);
}

/* @param pid - the pid of the child process*/
void racer2(int pid) {
    /*do not remove this line*/
    printAletter();
}

/*****************************************************************************\
*******************************************************************************
**  Decipher Puzzle
*******************************************************************************
\*****************************************************************************/

void decipher(const char *encrypted_words[]) {
    char *word;
    pid_t pid;
    char *argv[3];
    argv[0] = "./decipher";
    argv[2] = NULL;
    for (int i = 0; (word = (char *)encrypted_words[i]); i++) {
        argv[1] = word;

        sigset_t mask_one, prev_one;

        sigemptyset(&mask_one);
        sigaddset(&mask_one, SIGCHLD);

        // block SIGCHLD before fork to avoid race condition
        sigprocmask(SIG_BLOCK, &mask_one, &prev_one);

        if ((pid = fork()) == 0) { /* Child runs user job */
            // unblock SIGCHLD in child process
            sigprocmask(SIG_SETMASK, &prev_one, NULL);
            // change to child process's own process group
            setpgid(0, 0);
            if (execve(argv[0], argv, environ) < 0) {
                printf("%s: Command not found\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }

        int status;
        if (waitpid(pid, &status, 0) < 0) {
            printf("waitpid error\n");
            exit(EXIT_FAILURE);
        }
    }
}

/*****************************************************************************\
*******************************************************************************
**  Counter Puzzle
*******************************************************************************
\*****************************************************************************/
void counter(int num_direct_children) {
    if (num_direct_children == 0) {
        exit(1);
    }

    sigset_t mask_all, prev_all;
    sigfillset(&mask_all);

    pid_t pid;
    int status;
    int n = 1;
    while ((pid = waitpid(-1, &status, 0)) > 0) {
        // block signals to protect global variable
        sigprocmask(SIG_BLOCK, &mask_all, &prev_all);

        if (WIFEXITED(status)) {
            n += WEXITSTATUS(status);
        }

        sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }

    exit(n);
}

/*****************************************************************************\
*******************************************************************************
**  Reaper Puzzle
*******************************************************************************
\*****************************************************************************/

/*
 * safe_printf - async-signal-safe wrapper for printf
 */
void safe_printf(const char *format, ...);

/*note: this is a handler for SIGCHLD*/
void reaper(int sig) {
    sigset_t mask_all, prev_all;
    pid_t pid;
    int status;

    sigfillset(&mask_all);

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        // block signals to protect global variable
        sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
        // normal exit
        if (WIFEXITED(status)) {
            safe_printf("Job (%d) exited with status %d\n", pid, WEXITSTATUS(status));
        }
        // terminated by signal
        else if (WIFSIGNALED(status)) {
            safe_printf("Job (%d) terminated by signal %d\n", pid, WTERMSIG(status));
        }
        // stopped by signal
        else if (WIFSTOPPED(status)) {
            safe_printf("Job (%d) stopped by signal %d\n", pid, WSTOPSIG(status));
        }
        sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }
}

/*****************************************************************************\
*******************************************************************************
**  Shower Puzzle
*******************************************************************************
\*****************************************************************************/

/** You need to call this in each of your signal handler */
void signal_received(int signum);

typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0) {
        fprintf(stderr, "Signal error: %s\n", strerror(errno));
        exit(0);
    }
    return (old_action.sa_handler);
}

int alrm_hit = 0, usr1_hit = 0, usr2_hit = 0, cont_hit = 0, chld_hit = 0;

void sigalrm_handler(int sig) {
    int olderrno = errno;
    if (alrm_hit == 0) {
        signal_received(sig);
        alrm_hit = 1;
    }
    errno = olderrno;
}

void sigusr1_handler(int sig) {
    int olderrno = errno;
    if (usr1_hit == 0) {
        signal_received(sig);
        usr1_hit = 1;
    }
    errno = olderrno;
}

void sigusr2_handler(int sig) {
    int olderrno = errno;
    if (usr2_hit == 0) {
        signal_received(sig);
        usr2_hit = 1;
    }
    errno = olderrno;
}

void sigcont_handler(int sig) {
    int olderrno = errno;
    if (cont_hit == 0) {
        signal_received(sig);
        cont_hit = 1;
    }
    errno = olderrno;
}

void sigchld_handler(int sig) {
    int olderrno = errno;
    if (chld_hit == 0) {
        signal_received(sig);
        chld_hit = 1;
    }
    errno = olderrno;
}

/** 
 * Setup your signal handlers here.
 *
 */
void shower_setup(void) {
    sigset_t mask_all;
    sigfillset(&mask_all);
    sigprocmask(SIG_BLOCK, &mask_all, NULL);

    Signal(SIGALRM, sigalrm_handler);
    Signal(SIGUSR1, sigusr1_handler);
    Signal(SIGUSR2, sigusr2_handler);
    Signal(SIGCONT, sigcont_handler);
    Signal(SIGCHLD, sigchld_handler);
}

/**
 * Block off the appropriate signals
 */
void shower_run(void) {
    sigset_t mask_all;

    sigfillset(&mask_all);
    sigdelset(&mask_all, SIGALRM);
    sigprocmask(SIG_SETMASK, &mask_all, NULL);
    while (alrm_hit == 0) {
        sleep_millisecond(10);
    }

    sigfillset(&mask_all);
    sigdelset(&mask_all, SIGUSR1);
    sigprocmask(SIG_SETMASK, &mask_all, NULL);
    while (usr1_hit == 0) {
        sleep_millisecond(10);
    }

    sigfillset(&mask_all);
    sigdelset(&mask_all, SIGUSR2);
    sigprocmask(SIG_SETMASK, &mask_all, NULL);
    while (usr2_hit == 0) {
        sleep_millisecond(10);
    }

    sigfillset(&mask_all);
    sigdelset(&mask_all, SIGCONT);
    sigprocmask(SIG_SETMASK, &mask_all, NULL);
    while (cont_hit == 0) {
        sleep_millisecond(10);
    }

    sigfillset(&mask_all);
    sigdelset(&mask_all, SIGCHLD);
    sigprocmask(SIG_SETMASK, &mask_all, NULL);
    while (chld_hit == 0) {
        sleep_millisecond(10);
    }
}
