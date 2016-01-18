#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <poll.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <errno.h>

#define TT_DIR "/home/pi/.totweet/totweet"
#define TT_FDIR TT_DIR "/../failed"

static void handle_event(const int fd);

static int fd, wd;
static nfds_t nfds;
static struct pollfd fds[0];
static pid_t pid = 1;

int main()
{
	int ret;

	fd = inotify_init1(IN_NONBLOCK);
	if (fd == -1) {
		fprintf(stderr, "%s: %d: inotify_init1: %s\n", __FILE__, __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	ret = mkdir(TT_DIR, S_IRWXU);
	if (ret == -1) {
		if (errno != EEXIST) {
			fprintf(stderr, "%s: %d: mkdir: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	ret = mkdir(TT_FDIR, S_IRWXU);
	if (ret == -1) {
		if (errno != EEXIST) {
			fprintf(stderr, "%s: %d: mkdir: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	wd = inotify_add_watch(fd, TT_DIR, IN_CREATE);
	if (wd == -1) {
		fprintf(stderr, "%s: %d: inotify_add_watch: %s: %s\n", __FILE__, __LINE__, TT_DIR, strerror(errno));
		exit(EXIT_FAILURE);
	}

	nfds = 1;
	fds[0].fd = fd;
	fds[0].events = POLLIN;

	for (; ; ) {
		int pn = poll(fds, nfds, -1);
		if (pn == -1) {
			fprintf(stderr, "%s: %d: poll: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (pn == 0) {
			fprintf(stderr, "%s: %d: poll returned 0 though the timeout is not set\n", __FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}

		if (fds[0].revents & POLLIN)
			handle_event(fd);
	}

	ret = inotify_rm_watch(fd, wd);
	if (ret == -1) {
		fprintf(stderr, "%s: %d: inotify_rm_watch: %s\n", __FILE__, __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	ret = close(fd);
	if (ret == -1) {
		fprintf(stderr, "%s: %d: close: %s\n", __FILE__, __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

static void handle_event(const int fd)
{
	char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
	const struct inotify_event *event;
	ssize_t len;
	char *ptr;
	int ret;
	int status;

	for (; ; ) {
		len = read(fd, buf, sizeof(buf));
		if (len == -1 && errno != EAGAIN) {
			fprintf(stderr, "%s: %d: read: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}
		if (len <= 0)
			break;
		for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
			event = (const struct inotify_event*) ptr;

			if ((event->mask & IN_IGNORED) || (event->mask & IN_Q_OVERFLOW) || (event->mask & IN_UNMOUNT)) {
				exit(EXIT_FAILURE);
			}

			if ((event->len > 0) && (event->mask & IN_CREATE)) {
				char str[0x1000];
				pid = fork();
				if (pid == 0) {
					char *args[] = {
						"/bin/bash",
						"-c",
						NULL,
						NULL
					};
					char *envs[] = {
						NULL
					};

					args[2] = str;

					ret = snprintf(str, 0x1000, ". ~/.profile; HOME=~ ttytter -keyf=coma_ararat -status=\"$(cat %s/%s)\"", TT_DIR, event->name);
					if (ret < 0) {
						fprintf(stderr, "%s: %d: error in snprintf\n", __FILE__, __LINE__);
						exit(EXIT_FAILURE);
					}

					ret = execve(args[0], args, envs);
					if (ret == -1) {
						fprintf(stderr, "%s: %d: execve: %s\n", __FILE__, __LINE__, strerror(errno));
						_exit(EXIT_FAILURE);
					}
				}

				ret = waitpid(pid, &status, 0);
				if (ret == -1) {
					fprintf(stderr, "%s: %d: waitpid: %s\n", __FILE__, __LINE__, strerror(errno));
					exit(EXIT_FAILURE);
				}

				if (
					(WIFEXITED(status) && (WEXITSTATUS(status) != 0))
					|| WIFSIGNALED(status)
#ifdef WCOREDUMP
					|| WCOREDUMP(status)
#endif /* WCOREDUMP */
				) {
					char str2[0x1000];
					time_t t;
					struct tm *tmp;
					size_t rets;

					t = time(NULL);
					if (t == ((time_t) -1)) {
						fprintf(stderr, "%s: %d: time: %s\n", __FILE__, __LINE__, strerror(errno));
						exit(EXIT_FAILURE);
					}

					tmp = localtime(&t);
					if (tmp == NULL) {
						fprintf(stderr, "%s: %d: localtime returned NULL\n", __FILE__, __LINE__);
						exit(EXIT_FAILURE);
					}

					rets = strftime(str2, 0x1000, "%Y-%m-%d-%H:%M:%S", tmp);
					if (rets == 0) {
						fprintf(stderr, "%s: %d: strftime returned 0\n", __FILE__, __LINE__);
						exit(EXIT_FAILURE);
					}

					snprintf(str, 0x1000, "mv %s/%s %s/%s-%s", TT_DIR, event->name, TT_FDIR, str2, event->name);
					system(str);
				} else {
					snprintf(str, 0x1000, "%s/%s", TT_DIR, event->name);

					ret = unlink(str);
					if (ret == -1) {
						fprintf(stderr, "%s: %d: unlink: %s\n", __FILE__, __LINE__, strerror(errno));
						exit(EXIT_FAILURE);
					}
				}
			}
		}
	}
}
