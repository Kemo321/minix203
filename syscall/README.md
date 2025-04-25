

#   This project introduces a set of system calls to analyze process relationships in the MINIX operating system. The implemented system calls provide insights into process hierarchies, including identifying the process with the most children and finding the process with the longest chain of descendants.

## Implemented System Calls

### `do_maxChildren`
- **Purpose**: Determines the maximum number of children any process in the system has.
- **Prototype**: `_PROTOTYPE( int do_maxChildren, (void) );`
- **Description**: 
    - Iterates through all processes in the system.
    - Counts the number of children for each process using the helper function `childrenCount`.
    - Returns the maximum number of children found.

```c
PUBLIC int do_maxChildren()
{
    int children = -1;
    pid_t who = -1;
    maxChildren(&children, &who);
    return children;
}
```

### `do_whoMaxChildren`
- **Purpose**: Identifies the process with the maximum number of children.
- **Prototype**: `_PROTOTYPE( int do_whoMaxChildren, (void) );`
- **Description**: 
    - Uses the same logic as `do_maxChildren` but instead of returning the count, it returns the process ID (`pid`) of the process with the most children.

```c
PUBLIC int do_whoMaxChildren()
{
    int children = -1;
    pid_t who = -1;
    maxChildren(&children, &who);
    return who;
}
```

### `do_test`
- **Purpose**: Finds the process with the longest chain of descendants and the length of that chain.
- **Prototype**: `_PROTOTYPE( int do_test, (void) );`
- **Description**: 
    - Recursively traverses the process tree using the helper function `search_recursively`.
    - Skips a specific process if its `pid` is provided as input (`pid_to_skip`).
    - Tracks the longest chain of descendants and the corresponding process ID.
    - Combines the results into a single integer, where the higher bits represent the process ID and the lower bits represent the chain length.

```c
PUBLIC int do_test()
{
    int pid_to_skip = mm_in.m1_i1;
    int result = 0;
    int longest_path = 1;
    int longest_path_pid = 0;

    int i;
    for (i = 0; i < NR_PROCS; ++i)
    {
        if (mproc[i].mp_flags & IN_USE && mproc[i].mp_pid != pid_to_skip)
            search_recursively(&longest_path, &longest_path_pid, 1, i, mproc[i].mp_pid);
    }

    result = longest_path;
    longest_path_pid <<= 10;
    result += longest_path_pid;

    return result;
}
```

## Helper Functions

### `childrenCount`
- **Purpose**: Counts the number of immediate children for a given process.
- **Prototype**: `int childrenCount( int proc_nr );`
- **Description**: 
    - Iterates through all processes in the system.
    - Checks if a process is a child of the given process (`proc_nr`) by comparing the parent process ID.
    - Returns the count of children.

```c
int childrenCount(int proc_nr)
{
    int children = 0;
    int i = 0;
    for (i = 0; i < NR_PROCS; ++i)
        if ((mproc[i].mp_flags & IN_USE) && proc_nr != i && (mproc[i].mp_parent == proc_nr))
            ++children;
    return children;
}
```

### `maxChildren`
- **Purpose**: Finds the process with the maximum number of children and the count of those children.
- **Prototype**: `void maxChildren( int * children, pid_t * who );`
- **Description**: 
    - Iterates through all processes in the system.
    - Uses `childrenCount` to determine the number of children for each process.
    - Updates the maximum count and the corresponding process ID.

```c
void maxChildren(int *children, pid_t *who)
{
    int maxChildren = -1;
    pid_t found = -1;
    int count = 0;
    int proc_nr = 0;
    for (proc_nr = 0; proc_nr < NR_PROCS; ++proc_nr)
    {
        if (mproc[proc_nr].mp_flags & IN_USE)
        {
            count = childrenCount(proc_nr);
            if (count > maxChildren)
            {
                maxChildren = count;
                found = mproc[proc_nr].mp_pid;
            }
        }
    }
    *children = maxChildren;
    *who = found;
}
```

### `search_recursively`
- **Purpose**: Recursively traverses the process tree to find the longest chain of descendants.
- **Prototype**: `static void search_recursively(int *longest_path, int *longest_path_pid, int length, int proc_idx, pid_t root);`
- **Description**: 
    - Starts from a given process and recursively explores its descendants.
    - Updates the longest chain length and the corresponding root process ID.

```c
static void search_recursively(int *longest_path, int *longest_path_pid, int length, int proc_idx, pid_t root)
{
    int k = 0;
    if (length > *longest_path)
    {
        *longest_path = length;
        *longest_path_pid = root;
    }

    for (k = 0; k < NR_PROCS; ++k)
    {
        if (k == proc_idx) continue;

        if (mproc[k].mp_parent == proc_idx)
            search_recursively(longest_path, longest_path_pid, length + 1, k, root);
    }

    return;
}
```

## Constants

- `NR_PROCS`: Defines the maximum number of processes in the system (64).
- `MAXCHILDREN`: System call number for `do_maxChildren` (78).
- `WHOMAXCHILDREN`: System call number for `do_whoMaxChildren` (79).
- `TEST`: System call number for `do_test` (80).



## Notes

- The `do_test` system call combines the process ID and the chain length into a single integer. To extract these values:
    - Use bitwise operations to separate the higher bits (process ID) and lower bits (chain length).

## Screenshots
- ![1](ss/1.png)
- ![2](ss/2.png)
- ![3](ss/3.png)
