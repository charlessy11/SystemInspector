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

    int fd = open_path(proc_dir, "sys/kernel/hostname");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }

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
    // time = int(time) % (24 * 3600);
    int days_remainder = (int)years_remainder % (24 * 3600);

    int hours = (days_remainder / 3600);
    int hours_remainder = (int)days_remainder % 3600;

    int minutes = (hours_remainder / 60);
    int minutes_remainder = (int)hours_remainder % 60;

    int seconds = minutes_remainder; 
    
    if (years == 0) {
        // strcpy(uptime_buf, days);
        // strcat(uptime_buf, hours);
        // strcat(uptime_buf, minutes);
        // strcat(uptime_buf, seconds); 
        sprintf(uptime_buf, "%d days, %d hours, %d minutes, %d seconds", days, hours, minutes, seconds);
    }
    else if (days == 0) {
        // strcpy(uptime_buf, hours);
        // strcat(uptime_buf, minutes);
        // strcat(uptime_buf, seconds);  
        sprintf(uptime_buf, "%d hours, %d minutes, %d seconds", hours, minutes, seconds);
    }
    else if (hours == 0) {
        // strcpy(uptime_buf, minutes);  
        // strcat(uptime_buf, seconds);
        sprintf(uptime_buf, "%d minutes, %d seconds", minutes, seconds);
    }
    else if (minutes == 0) {
        // strcpy(uptime_buf, "0");
        // strcat(uptime_buf, seconds);
        sprintf(uptime_buf, "%d minutes, %d seconds", 0, seconds);
    }
    else {
        // strcpy(uptime_buf, years);
        // strcat(uptime_buf, days);
        // strcat(uptime_buf, hours);
        // strcat(uptime_buf, minutes);
        // strcat(uptime_buf, seconds);
        sprintf(uptime_buf, "%d years, %d days, %d hours, %d minutes, %d seconds", years, days, hours, minutes, seconds);
    }

    return 0;
}

struct load_avg pfs_load_avg(char *proc_dir)
{  

    // LOGP("Getting the load average\n");

    // int fd = open_path(proc_dir, "loadavg");
    // if (fd == -1) {
    //     perror("open_path");
    //     return -1;
    // }
    // char line[256] = { 0 };
    struct load_avg lavg = { 0 };
    // // one_lineread(proc_dir, "loadavg");
    // lineread(fd, line, 256);
    // size_t loadavg_loc = strcspn(line, " ") + 10;
    // strncpy(lavg, line, loadavg_loc);

    return lavg;
}

double pfs_cpu_usage(char *proc_dir, struct cpu_stats *prev, struct cpu_stats *curr)
{
    // LOGP("Getting the cpu usage\n");

    // double idle1, idle2, total1, total2 = 0;
    // idle1 = prev->idle;
    // total1 = prev->total;

    // int fd = open_path(proc_dir, "stat");
    // if (fd == -1) {
    //     perror("open_path");
    //     return -1;
    // }

    // size_t read_sz = 0;
    // char line[256];
    // while ((read_sz = lineread(fd, line, 256)) > 0) {
    //     // LOG("line: %s\n", line);
    //     int counter = 0;
    //     //check for "cpu " in line
    //     if (strstr(line, "cpu ")) {
    //         char *next_tok = line;
    //         char *curr_tok;
    //         // LOG("Curr tok: %s\n", line);
    //         //Tokenize.
    //         while ((curr_tok = next_token(&next_tok, " ")) != NULL) {
    //         // LOG("Tokenize: %s\n", next_tok);
    //                 if (counter != 0) {
    //                     total2 += atol(curr_tok);
    //                     if (counter == 4) {
    //                        idle2 = atol(curr_tok);
    //                         // idle2 = counter - 1;
    //                     }
    //                 }
    //                 counter++;
    //         }
    //         curr->idle = idle2;
    //         curr->total = total2;
    //             // LOG("Idle: %s\n", idle2);  
    //         // }
    //         // counter++;
    //     }
    // }
    //  LOG("Total2 and Idle2: %ld %ld\n", total2, idle2);
    //  LOG("Total1 and Idle1: %ld %ld\n", total1, idle1);
    // double idle = idle2 - idle1;
    // double total = total2 - total1;
    // double cpu_usage = 1.0 - (idle / total);
    // if (isnan(cpu_usage)) {
    //     return 0.0;
    // }

    // close(fd);
    // // LOG("Total and Idle: %ld %ld\n", total, idle);
    // LOG("CPU_USAGE TOTAL: %ld", cpu_usage);
    // return cpu_usage;
    return 0.0;
}

struct mem_stats pfs_mem_usage(char *proc_dir)
{
    // int fd = open_path(proc_dir, "meminfo");
    // if (fd == -1) {
    //     perror("open_path");
    //     return -1;
    // }

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

