#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "logger.h"
#include "procfs.h"
#include "util.h"

int pfs_hostname(char *proc_dir, char *hostname_buf, size_t buf_sz)
{
    LOGP("Getting the hostname\n");
    //opens path
    int fd = open_path(proc_dir, "sys/kernel/hostname");
    //error checking
    if (fd == -1) {
        perror("open_path");
        return -1;
    }
    //
    ssize_t read_sz = lineread(fd, hostname_buf, buf_sz);
    if (read_sz == -1) {
        return -1;
    }
    close(fd);

    hostname_buf[read_sz - 1] = '\0';
    return 0;
}

int pfs_kernel_version(char *proc_dir, char *version_buf, size_t buf_sz)
{
    LOGP("Getting the kernel version\n");

    int fd = open_path(proc_dir, "sys/kernel/osrelease");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }    
    
    ssize_t read_sz = lineread(fd, version_buf, buf_sz);
    if (read_sz == -1) {
        return -1;
    }
    close(fd);
    //iterate until "-"
    size_t dash_loc = strcspn(version_buf, "-");
    version_buf[dash_loc] = '\0';
    return 0;
}

int pfs_cpu_model(char *proc_dir, char *model_buf, size_t buf_sz)
{
    LOGP("Getting the cpu model\n");

    int fd = open_path(proc_dir, "cpuinfo");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }
    //model buf is 128 chars in display.c, so we allocate space for that plus allowance
    char line[256] = { 0 };

    ssize_t read_sz = 0;    
    while ((read_sz = lineread(fd, line, 256)) > 0) {
        if (strncmp(line, "model name", 10) == 0) {
            size_t model_loc = strcspn(line, ":") + 2;
            size_t newline_loc = strcspn(&line[model_loc], "\n");
            strncpy(model_buf, &line[model_loc], newline_loc);
        }
    }   
    close(fd);

    return 0;
}

int pfs_cpu_units(char *proc_dir)
{
    LOGP("Getting the cpu units\n");

    int fd = open_path(proc_dir, "cpuinfo");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }    
    char line[256] = { 0 };
    int counter = 0;
    ssize_t read_sz = 0;    
    while ((read_sz = lineread(fd, line, 256)) > 0) {
        if (strncmp(line, "processor", 9) == 0) {
            counter++;
        }
    }   
    close(fd);

    return counter;
}

double pfs_uptime(char *proc_dir)
{
    LOGP("Getting the uptime\n");

    int fd = open_path(proc_dir, "uptime");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }
    char line[256] = { 0 };
    char *ptr;
    double time;
    // char temp[256] = { 0 };
    lineread(fd, line, 256);
    // size_t uptime_loc = strcspn(line, " ");
    // strncpy(temp, line, uptime_loc);
    // convert string to double
    time = strtod(line, &ptr);
    // double *time = (double *)temp;

    close(fd);

    return time;
}

int pfs_format_uptime(double time, char *uptime_buf)
{
    LOGP("Getting the uptime format\n");
    
    int years = (time / 31556952);
    int years_remainder = (int)time % 31556952;

    int days = (years_remainder/(24 * 3600));
    int days_remainder = (int)years_remainder % (24 * 3600);

    int hours = (days_remainder / 3600);
    int hours_remainder = (int)days_remainder % 3600;

    int minutes = (hours_remainder / 60);
    int minutes_remainder = (int)hours_remainder % 60;

    int seconds = minutes_remainder; 
    
    if (years == 0) {
        sprintf(uptime_buf, "%d days, %d hours, %d minutes, %d seconds", days, hours, minutes, seconds);
    }
    else if (days == 0) {
        sprintf(uptime_buf, "%d hours, %d minutes, %d seconds", hours, minutes, seconds);
    }
    else if (hours == 0) {
        sprintf(uptime_buf, "%d minutes, %d seconds", minutes, seconds);
    }
    else if (minutes == 0) {
        sprintf(uptime_buf, "%d minutes, %d seconds", 0, seconds);
    }
    else {
        sprintf(uptime_buf, "%d years, %d days, %d hours, %d minutes, %d seconds", years, days, hours, minutes, seconds);
    }

    return 0;
}

struct load_avg pfs_load_avg(char *proc_dir)
{  
    LOGP("Getting the load average\n");

    struct load_avg lavg = { 0 };

    int fd = open_path(proc_dir, "loadavg");
    if (fd == -1) {
        perror("open_path");
        return lavg;
    }
    char line[50] = { 0 };
    // struct load_avg lavg = { 0 };
    one_lineread(fd, line, 50, "\n");
    // size_t loadavg_loc = strcspn(line, " ") + 10;
    // strncpy(struct load_avg lavg, &line, loadavg_loc);
    char *next_tok = line;
    char *curr_tok;
    char *end_ptr;
    curr_tok = next_token(&next_tok, " ");
    lavg.one = strtod(curr_tok, &end_ptr);
    curr_tok = next_token(&next_tok, " ");
    lavg.five = strtod(curr_tok, &end_ptr);
    curr_tok = next_token(&next_tok, " ");
    lavg.fifteen = strtod(curr_tok, &end_ptr);

    close(fd);
    return lavg;
}

double pfs_cpu_usage(char *proc_dir, struct cpu_stats *prev, struct cpu_stats *curr)
{
    LOGP("Getting the cpu usage\n");

    double idle1, idle2, total1, total2 = 0.0;
    idle1 = prev->idle;
    total1 = prev->total;
    //opens file
    int fd = open_path(proc_dir, "stat");
    //error checking
    if (fd == -1) {
        perror("open_path");
        return -1;
    }
    size_t read_sz = 0;
    char line[256] = { 0 };
    while ((read_sz = lineread(fd, line, 256)) > 0) {
        //check for "cpu " in line
        if (strstr(line, "cpu ")) {
            char *next_tok = line;
            char *curr_tok;
            char *end_ptr;
            int counter = 0;
            //Iterate and tokenize line
            while ((curr_tok = next_token(&next_tok, " ")) != NULL) {
                    //checks if not in "cpu"
                    if (counter != 0) {
                        //add value to total2
                        total2 += strtod(curr_tok, &end_ptr);
                        //checks when in idle
                        if (counter == 4) {
                           //stores idle value to idle2 
                           idle2 = strtod(curr_tok, &end_ptr);
                        }
                    }
                    counter++;
            }
            curr->idle = idle2;
            curr->total = total2; 
        }
    }
    if ((idle2 - idle1 < 0) || (total2 - total1 < 0)) {
        return 0.0;
    }
    double cpu_usage = 1 - ((idle2-idle1) / (total2-total1));
    if (isnan(cpu_usage)) {
        return 0.0;
    }

    close(fd);
    return cpu_usage;
}

struct mem_stats pfs_mem_usage(char *proc_dir)
{
    // int fd = open_path(proc_dir, "meminfo");
    // if (fd == -1) {
    //     perror("open_path");
    //     struct mem_stats mstats = { 0 };
    // }

    // char line[256] = { 0 };

    // ssize_t read_sz = 0;    
    // while ((read_sz = lineread(fd, line, 256)) > 0) {
    //     if (strncmp(line, "MemTotal", 8) == 0) {
    //         size_t memTotal_loc = strcspn(line, ":");
    //         size_t newline_loc = strcspn(&line[memTotal_loc], "\n");
    //         // strncpy(model_buf, &line[model_loc], newline_loc);
    //     }
    //     if (strncmp(line, "MemAvailable", 12) == 0) {

    //     }
    // }   
    // close(fd);
    struct mem_stats mstats = { 0 };
    return mstats;
}

struct task_stats *pfs_create_tstats()
{
    return NULL;
}

void pfs_destroy_tstats(struct task_stats *tstats)
{

}

int pfs_tasks(char *proc_dir, struct task_stats *tstats)
{
    return -1;
}

