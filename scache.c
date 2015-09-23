#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/time.h>

char* keyfile = 0;
int timeout = 0;

void bail(char*);

int delete_keyfile() {
  if (!keyfile) bail("Could not delete keyfile");
  umask(077);
  FILE* f = fopen(keyfile, "w");
  fclose(f);
  unlink(keyfile);
  return 0;
}

void bail(char* msg) {
  fprintf(stderr, "%s\n", msg);
  exit(-1);
}

int now() {
  struct timeval tp;
  struct timezone tz;
  tz.tz_minuteswest = 0;
  tz.tz_dsttime = 0;
  if (gettimeofday(&tp, &tz)) bail("Could not get time of day");
  return tp.tv_sec;
}

int getpppid() {
  char pscmd[128];
  
  if (snprintf(pscmd, 100, "/bin/ps -oppid= %d", getppid())>80) {
    bail("Failed to get PPPID.  This should never happen.");
  }
  
  char* line = 0;
  size_t linecap;

  FILE* f = popen(pscmd, "r");
  if (getline(&line, &linecap, f) < 0) {
    bail("Failed to get PPPID.  This should never happen.");
  }
  
  return atoi(line);
}

int update_keyfile(char* pw) {
  umask(077);
  FILE* f = fopen(keyfile, "w");
  fprintf(f, "%d\n", now() + timeout);
  fprintf(f, "%d\n", getpppid());
  fprintf(f, "%s\n", ttyname(0));
  fprintf(f, "%d\n", getuid());
  fprintf(f, "%s\n", pw);
  fclose(f);
  return 0;
}

int init_keyfile() {
  char *pw = getpass("Enter pass phrase: ");
  if (strlen(pw)<=0) bail("Cannot cache an empty pass phrase.");
  return update_keyfile(pw);
}

int main(int argc, char* argv[]) {

  // Assemble the key file name
  char* homedir = getenv("HOME");
  int len = strlen(homedir) + 128;
  keyfile = malloc(len);
  snprintf(keyfile, len, "%s/.scache_%d", homedir, getuid());

  timeout = (argc>1) ? atoi(argv[1]) : 300;
  if (timeout<=0) return delete_keyfile();
  if (timeout>300) timeout = 300;

  if (geteuid()) bail("This program must be run as root.");
  if (!isatty(0)) bail("ERROR: STDIN is not a TTY");
  if (isatty(1)) bail("ERROR: stdout is a TTY");

  struct stat statbuf;

 loop:
  
  if (stat(keyfile, &statbuf)<0) {
    init_keyfile();
    if (stat(keyfile, &statbuf)<0) bail("Failed to create keyfile");
  }
  if (statbuf.st_uid) bail("ERROR: Key file is not owned by root");
  if (statbuf.st_mode != 0100600) bail("ERROR: Key file has incorrect mode");
  FILE* f = fopen(keyfile, "r");
  if (!f) bail("Could not open keyfile.");
  
  int cnt;
  char* line = 0;
  size_t linecap;

  // Check the timeout
  if ((cnt = getline(&line, &linecap, f)) <= 0) bail("Error reading keyfile");
  if (now() > atoi(line)) {
    fclose(f);
    delete_keyfile();
    fprintf(stderr, "Cached pass phrase has expired.\n");
    goto loop;
  }

  // Check the PPPID
  if ((cnt = getline(&line, &linecap, f)) <= 0) bail("Error reading keyfile");
  if (getpppid() != atoi(line)) bail("PPPID mismatch");

  // Check the TTY name
  if ((cnt = getline(&line, &linecap, f)) <= 0) bail("Error reading keyfile");
  line[strlen(line)-1]=0;
  char* tty = ttyname(0);
  if (strcmp(tty, line)) bail("TTY mismatch");

  // Check the UID
  if ((cnt = getline(&line, &linecap, f)) <= 0) bail("Error reading keyfile");
  if (getuid() != atoi(line)) bail("UID mismatch");

  // Everthing checks out, get the secret
  if ((cnt = getline(&line, &linecap, f)) <= 0) bail("Error reading keyfile");
  fclose(f);
  line[cnt-1] = 0;
  // Update the time stamp
  update_keyfile(line);
  printf("%s\n", line);
}
