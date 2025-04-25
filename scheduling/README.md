# Task Done: Weighted Round-Robin Scheduling

## Changes Made
Implemented weighted round-robin scheduling with the following time quanta distribution:
- **Group A**: 1 quantum
- **Group B**: 2 quanta
- **Group C**: 3 quanta

### Observations
- Each process receives an additional quantum, resulting in an effective distribution of **2/3/4** instead of **1/2/3**.

### Test File
- [Test File](minix_usr/src/soi/test.c)

### Columns Description
- **`-user`**: Displays the time quanta granted to each process.
- **`g`**: Displays the group of the process.

### Example
![Example Screenshot](ss/ss.png)

In this example:
- **Group A**: 2 processes
- **Group B**: 1 process
- **Group C**: 1 process

During a cycle:
- Group A processes share 2 quanta.
- Group B processes receive 3 quanta.
- Group C processes receive 4 quanta.

---

## Code Changes

### System Calls
Added the following system calls:
```c
do_getGroup, /* 78 = getgroups */
do_setGroup, /* 79 = setgroups */
do_getQuants, /* 80 = getquants */
do_setQuants, /* 81 = setquants */
```

### Prototypes
```c
_PROTOTYPE( int do_getGroup, (void) );
_PROTOTYPE( int do_setGroup, (void) );
_PROTOTYPE( int do_getQuants, (void) );
_PROTOTYPE( int do_setQuants, (void) );
```

### System Call Implementations
```c
PUBLIC int do_getGroup() {
    int r;
    r = _taskcall(SYSTASK, SYS_GETGROUP, &mm_in);
    mp->mp_reply = mm_in;
    return r;
}

PUBLIC int do_setGroup() {
    return _taskcall(SYSTASK, SYS_SETGROUP, &mm_in);
}

PUBLIC int do_getQuants() {
    int r;
    r = _taskcall(SYSTASK, SYS_GETQUANTS, &mm_in);
    mp->mp_reply = mm_in;
    return r;
}

PUBLIC int do_setQuants() {
    return _taskcall(SYSTASK, SYS_SETQUANTS, &mm_in);
}
```

### Scheduler Logic
Modified the `sched()` function to prioritize processes based on group weights:
```c
PRIVATE void sched() {
    struct proc *rp, *prev;
    int current_group, next_group, search_group, i;

    if (rdy_head[USER_Q] == NIL_PROC) return;

    current_group = rdy_head[USER_Q]->group;

    rdy_tail[USER_Q]->p_nextready = rdy_head[USER_Q];
    rdy_tail[USER_Q] = rdy_head[USER_Q];
    rdy_head[USER_Q] = rdy_head[USER_Q]->p_nextready;
    rdy_tail[USER_Q]->p_nextready = NIL_PROC;

    next_group = (current_group + 1) % 3;
    for (i = 0; i < 3; i++) {
        search_group = (next_group + i) % 3;
        rp = rdy_head[USER_Q];
        prev = NULL;

        while (rp != NIL_PROC) {
            if (rp->group == search_group) {
                if (prev != NULL) {
                    prev->p_nextready = rp->p_nextready;
                } else {
                    break;
                }

                rp->p_nextready = rdy_head[USER_Q];
                rdy_head[USER_Q] = rp;

                if (rp == rdy_tail[USER_Q]) {
                    rdy_tail[USER_Q] = prev;
                }
                break;
            }
            prev = rp;
            rp = rp->p_nextready;
        }

        if (rp != NIL_PROC && rp->group == search_group) break;
    }

    pick_proc();
}
```

### Helper Functions
#### Find Process Descriptor
```c
struct proc * findProcDescriptor(int pid) {
    struct proc *rp;
    for (rp = BEG_PROC_ADDR; rp < END_PROC_ADDR; rp++) {
        if (rp->p_pid != 0 && rp->p_pid == pid) return rp;
    }
    return NULL;
}
```

#### Get/Set Group
```c
PRIVATE int do_getGroup(register message *m_ptr) {
    struct proc *rp = findProcDescriptor(m_ptr->m1_i1);
    if (rp == NULL) return EFAULT;
    m_ptr->m1_i1 = rp->group;
    return OK;
}

PRIVATE int do_setGroup(register message *m_ptr) {
    struct proc *rp;
    if (m_ptr->m1_i2 < 0 || m_ptr->m1_i2 > 2) return EFAULT;
    rp = findProcDescriptor(m_ptr->m1_i1);
    if (rp == NULL) return EFAULT;
    rp->group = m_ptr->m1_i2;
    return OK;
}
```

#### Get/Set Quanta
```c
PRIVATE int do_getQuants(register message *m_ptr) {
    if (m_ptr->m1_i1 < 0 || m_ptr->m1_i1 > 2) return EFAULT;
    m_ptr->m1_i1 = quants[m_ptr->m1_i1];
    return OK;
}

PRIVATE int do_setQuants(register message *m_ptr) {
    if (m_ptr->m1_i1 < 0 || m_ptr->m1_i1 > 2) return EFAULT;
    if (m_ptr->m1_i2 < 1 || m_ptr->m1_i2 > 100) return EFAULT;
    quants[m_ptr->m1_i1] = m_ptr->m1_i2;
    return OK;
}
```

### Added attributes
```c
int group; // Added to proc descriptor
int quants[3] = {1, 2, 3}; // static int
```
