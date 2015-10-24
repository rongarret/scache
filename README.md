# S-Cache (Secure Cache)

S-Cache is a program that securely caches a pass phrase or other secret data.

S-Cache is an SUID-root executable that works by storing the secret in
a file that is owned by root and only readable by root.  Its security is
thus comparable to the security of shadow passwords.

In addition, S-Cache takes a number of precautions to prevent its being
hijacked by malware running as the user:

1.  It stores the name of the controlling TTY and the grandparent process
ID (PPPID) along with the secret, and only reveals it if the calling
process has the same values as the one which stored the secret in the
first place.  This is comparable to the security of SUDO with TTY_TICKETS
enabled.

2.  It will not reveal the secret if STDOUT is a TTY.  This is easily
subverted by piping the output to CAT, but provides a measure of protection
against displaying the secret on the terminal by accident.

3.  The cache will time out after a user-settable period of time.
