# Project 1: System Inspector

See: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-1.html 

**About This Program:**

The System Inspector Program inspects the system it runs on and displays a summary of useful information of the system. It behaves somewhat similar to the `top` command found in many Unix-like operating systems which is used to show the Linux processes. Similarly, the two provide a dynamic real-time view of the running system.

**How It Works:**

The program displays a summary of useful information of the system by reading the contents of `\proc`.

**How It's Built:**

Here is a list of methods that reads the different contents of `\proc`:

To compile and run:

```bash
make
./inspector
```

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
