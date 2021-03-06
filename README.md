# Project 1: System Inspector

See: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-1.html 

**About This Program:**

The System Inspector Program implements a Unix utility that inspects the system it runs on and displays a summary of useful information of the system to the user. It behaves somewhat similar to the `top` command found in many Unix-like operating systems which is used to show the Linux processes. Similarly, the two provide a dynamic real-time view of the running system.

**How It Works:**

The program displays a summary of useful information of the system by reading the contents of `\proc`.

**How It's Built:**

Here is a list of methods that reads the different contents of `\proc`:

* Load average every one, five, and fifteen minutes: 
`struct load_avg {
    double one;
    double five;
    double fifteen;
};`

* CPU information to check cpu usage: 
`struct cpu_stats {
    long idle;
    long total;
};`

* Memory information to check memory usage: 
`struct mem_stats {
    double used;
    double total;
};`

* Task statistics: 
`struct task_stats {
    unsigned int total;
    unsigned int running;
    unsigned int waiting;
    unsigned int sleeping;
    unsigned int stopped;
    unsigned int zombie;
    struct task_info *active_tasks;
};`

* Task information: 
`struct task_info {
    pid_t pid;
    uid_t uid;
    char name[26];
    char state[13];
};`

* Retrieves hostname of the machine: `int pfs_hostname(char *proc_dir, char *hostname_buf, size_t buf_sz);`

* Retrieves kernel version of the machine: `int pfs_kernel_version(char *proc_dir, char *version_buf, size_t buf_sz);`

* Retrieves cpu model name of the machine: `int pfs_cpu_model(char *proc_dir, char *model_buf, size_t buf_sz);`

* Sums up the number of processing units of the machine: `int pfs_cpu_units(char *proc_dir);`

* Retrieves uptime(in seconds) of the machine: `double pfs_uptime(char *proc_dir);`

* Formats uptime(in seconds) to days, hours, minutes, and seconds: `int pfs_format_uptime(double time, char *uptime_buf);`

* Retrieves load average of the machine: `struct load_avg pfs_load_avg(char *proc_dir);`

* Retrieves cpu usage of the machine: `double pfs_cpu_usage(char *procfs_dir, struct cpu_stats *prev, struct cpu_stats *curr);`

* Retrieves memory usage of the machine: `struct mem_stats pfs_mem_usage(char *procfs_dir);`

* Creates memory for task_stats structs: `struct task_stats *pfs_create_tstats();`

* Frees memory for task_stats structs: `void pfs_destroy_tstats(struct task_stats *tstats);`

* Retrieves tasks info of the machine: `int pfs_tasks(char *proc_dir, struct task_stats *tstats);`

Here is a list of helper methods:

* Opens specified path: `int open_path(char *proc_dir, char *path);`

* Reads one line of file: `ssize_t one_lineread(int fd, char *buf, size_t sz, char *delim);`

* Reads a file line by line: `ssize_t lineread(int fd, char *buf, size_t sz);`

* Tokenize string: `char *next_token(char **str_ptr, const char *delim);`

* Draws percentage bar: `void draw_percbar(char *buf, double frac);`

* Maps ID numbers (UID) associated with running processes to corresponding username: `void uid_to_uname(char *name_buf, uid_t uid);`

*More information about these methods such as input parameters and return values in header files

To compile and run:

```bash
make
./inspector
```
**Sample Output:**

<img width="653" alt="Screen Shot 2021-02-23 at 2 08 09 PM" src="https://user-images.githubusercontent.com/61255802/108840687-b2219900-7611-11eb-9f63-f0c9fa3b3650.png">


## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'
```
