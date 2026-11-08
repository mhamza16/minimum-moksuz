#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/wait.h>	/* wait() for Part 3 */
#include <pthread.h>	/* pthread_create/join for Part 4 */

#include "scull.h"

#define CDEV_NAME "/dev/scull"

/* Quantum command line option */
static int g_quantum;

static void usage(const char *cmd)
{
	printf("Usage: %s <command>\n"
	       "Commands:\n"
	       "  R          Reset quantum\n"
	       "  S <int>    Set quantum\n"
	       "  T <int>    Tell quantum\n"
	       "  G          Get quantum\n"
	       "  Q          Query quantum\n"
	       "  X <int>    Exchange quantum\n"
	       "  H <int>    Shift quantum\n"
	       "  i          Print task info\n"
	       "  p          Run 4 processes, each prints task info twice\n"
	       "  t          Run 4 threads, each prints task info twice\n"
	       "  h          Print this message\n",
	       cmd);
}

typedef int cmd_t;

static cmd_t parse_arguments(int argc, const char **argv)
{
	cmd_t cmd;

	if (argc < 2) {
		fprintf(stderr, "%s: Invalid number of arguments\n", argv[0]);
		cmd = -1;
		goto ret;
	}

	/* Parse command and optional int argument */
	cmd = argv[1][0];
	switch (cmd) {
	case 'S':
	case 'T':
	case 'H':
	case 'X':
		if (argc < 3) {
			fprintf(stderr, "%s: Missing quantum\n", argv[0]);
			cmd = -1;
			break;
		}
		g_quantum = atoi(argv[2]);
		break;
	case 'R':
	case 'G':
	case 'Q':
	case 'h':
	case 'i':
	case 'p':
	case 't':
		break;
	default:
		fprintf(stderr, "%s: Invalid command\n", argv[0]);
		cmd = -1;
	}

ret:
	if (cmd < 0 || cmd == 'h') {
		usage(argv[0]);
		exit((cmd == 'h')? EXIT_SUCCESS : EXIT_FAILURE);
	}
	return cmd;
}

static void *thread_fn(void *arg);	/* Part 4 thread body, defined below do_op */

static int do_op(int fd, cmd_t cmd)
{
	int ret, q, i, err, started = 0;
	pid_t pid;
	pthread_t tid[4];
	struct task_info ti;	/* filled by the driver via SCULL_IOCIQUANTUM */

	switch (cmd) {
	case 'i': /* Info: the driver copies a whole struct task_info into ti */
		ret = ioctl(fd, SCULL_IOCIQUANTUM, &ti);
		if (ret == 0)
			printf("state %u, cpu %u, prio %d, pid %d, tgid %d, nv %lu, niv %lu\n",
			       ti.__state, ti.cpu, ti.prio, ti.pid, ti.tgid,
			       ti.nvcsw, ti.nivcsw);
		break;
	case 'p': /* 4 child processes, each does 'i' twice */
		/* Flush first: unflushed text would be copied into every child */
		fflush(stdout);
		ret = 0;
		/* Start ALL children before waiting, so they run concurrently */
		for (i = 0; i < 4; i++) {
			pid = fork();
			if (pid < 0) {
				perror("fork");
				ret = -1;
				break;
			}
			if (pid == 0) {
				/*
				 * Child: uses the fd inherited from the parent
				 * (same open file, no open/close here), then
				 * exits so it never returns into this loop.
				 */
				do_op(fd, 'i');
				do_op(fd, 'i');
				exit(EXIT_SUCCESS);
			}
			started++;
		}
		/* Parent: reap exactly the children that were started */
		for (i = 0; i < started; i++)
			wait(NULL);
		break;
	case 't': /* 4 threads in this process, each does 'i' twice */
		ret = 0;
		/*
		 * Create ALL threads before joining any, so they run
		 * concurrently. &fd is safe to share: do_op only returns
		 * after every thread is joined, so fd outlives them.
		 */
		for (i = 0; i < 4; i++) {
			err = pthread_create(&tid[i], NULL, thread_fn, &fd);
			if (err != 0) {
				/* returns an error number, does not set errno */
				fprintf(stderr, "pthread_create: %d\n", err);
				ret = -1;
				break;
			}
			started++;
		}
		/* Wait for exactly the threads that were started */
		for (i = 0; i < started; i++)
			pthread_join(tid[i], NULL);
		break;
	case 'R':
		ret = ioctl(fd, SCULL_IOCRESET);
		if (ret == 0)
			printf("Quantum reset\n");
		break; case 'Q':
		q = ioctl(fd, SCULL_IOCQQUANTUM);
		printf("Quantum: %d\n", q);
		ret = 0;
		break;
	case 'G':
		ret = ioctl(fd, SCULL_IOCGQUANTUM, &q);
		if (ret == 0)
			printf("Quantum: %d\n", q);
		break;
	case 'T':
		ret = ioctl(fd, SCULL_IOCTQUANTUM, g_quantum);
		if (ret == 0)
			printf("Quantum set\n");
		break;
	case 'S':
		q = g_quantum;
		ret = ioctl(fd, SCULL_IOCSQUANTUM, &q);
		if (ret == 0)
			printf("Quantum set\n");
		break;
	case 'X':
		q = g_quantum;
		ret = ioctl(fd, SCULL_IOCXQUANTUM, &q);
		if (ret == 0)
			printf("Quantum exchanged, old quantum: %d\n", q);
		break;
	case 'H':
		q = ioctl(fd, SCULL_IOCHQUANTUM, g_quantum);
		printf("Quantum shifted, old quantum: %d\n", q);
		ret = 0;
		break;
	default:
		/* Should never occur */
		abort();
		ret = -1; /* Keep the compiler happy */
	}

	if (ret != 0)
		perror("ioctl");
	return ret;
}

/*
 * Part 4 thread body: arg is &fd from the main thread.
 */
static void *thread_fn(void *arg)
{
	int fd = *(int *)arg;	/* same fd (same open file) as main */

	/* Each thread is its own task: own PID, shared TGID */
	do_op(fd, 'i');
	do_op(fd, 'i');
	return NULL;
}

int main(int argc, const char **argv)
{
	int fd, ret;
	cmd_t cmd;

	cmd = parse_arguments(argc, argv);

	fd = open(CDEV_NAME, O_RDONLY);
	if (fd < 0) {
		perror("cdev open");
		return EXIT_FAILURE;
	}

	printf("Device (%s) opened\n", CDEV_NAME);

	ret = do_op(fd, cmd);

	if (close(fd) != 0) {
		perror("cdev close");
		return EXIT_FAILURE;
	}

	printf("Device (%s) closed\n", CDEV_NAME);

	return (ret != 0)? EXIT_FAILURE : EXIT_SUCCESS;
}
