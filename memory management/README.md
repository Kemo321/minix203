# Added Option to Change Memory Assignment Algorithm Between First Fit and Worst Fit

This document describes the changes made to the memory management system to allow switching between the First Fit and Worst Fit memory allocation algorithms.

---

## Changed Files:

```
src/
├── fs/
│   └── table.c
├── mm/
│   ├── alloc.c  - syscalls (458 485) alloc_mem (50)
│   ├── proto.h
│   └── table.c
include/
├── minix/
│   └── callnr.h
```

---

## Testing the Solution With:
-   `t.c`
-   `w.c`
-   `x.c`
-   `script.sh`

---

## Result in [result.txt](result.txt):
![Result Image](image.png)

---

## Code Changes:

### 1. Definitions for New System Calls

The following constants and prototypes were added to define the new system calls:

```c
#define HOLE_MAP      78
#define WORST_FIT     79

do_hole_map,
do_worst_fit,

_PROTOTYPE( int do_hole_map, (void) );
_PROTOTYPE( int do_worst_fit, (void) );
```

- **HOLE_MAP** and **WORST_FIT**: Constants representing the new system calls.
- **do_hole_map** and **do_worst_fit**: Function declarations for the new system calls.

---

### 2. Global Variable for Algorithm Selection

A global variable was introduced to track the current memory allocation algorithm:

```c
int use_worst_fit = 0;
```

- **use_worst_fit**: A flag to determine whether to use the Worst Fit algorithm (`1`) or the First Fit algorithm (`0`).

---

### 3. Modified `alloc_mem` Function

The `alloc_mem` function was updated to support both First Fit and Worst Fit algorithms:

```c
PUBLIC phys_clicks alloc_mem(clicks)
phys_clicks clicks; /* amount of memory requested */
{
    register struct hole *hp, *prev_ptr;
    phys_clicks old_base;

    phys_clicks max_size = 0;
    struct hole *biggest_found = NULL;
    struct hole *worst_prev = NULL;

    if (use_worst_fit == 0) {
        do {
            hp = hole_head;
            while (hp != NIL_HOLE && hp->h_base < swap_base) {
                if (hp->h_len >= clicks) {
                    old_base = hp->h_base;
                    hp->h_base += clicks;
                    hp->h_len -= clicks;

                    if (hp->h_len == 0) del_slot(prev_ptr, hp);

                    return (old_base);
                }
                prev_ptr = hp;
                hp = hp->h_next;
            }
        } while (swap_out());
        return (NO_MEM);
    } else {
        hp = hole_head;

        while (hp != NIL_HOLE && hp->h_base < swap_base) {
            if (hp->h_len >= clicks && hp->h_len > max_size) {
                max_size = hp->h_len;
                biggest_found = hp;
                worst_prev = prev_ptr;
            }
            prev_ptr = hp;
            hp = hp->h_next;
        }

        if (max_size > 0) {
            old_base = biggest_found->h_base;
            biggest_found->h_base += clicks;
            biggest_found->h_len -= clicks;

            if (biggest_found->h_len == 0) del_slot(worst_prev, biggest_found);

            return (old_base);
        } else {
            return (NO_MEM);
        }
    }
}
```

- **First Fit**: Allocates the first hole that fits the requested size.
- **Worst Fit**: Allocates the largest available hole that fits the requested size.

---

### 4. Implementation of `do_hole_map`

The `do_hole_map` function provides a system call to retrieve the current memory hole map:

```c
PUBLIC int do_hole_map() {
    register struct hole *hp = hole_head;
    unsigned int buffer[2];
    int i = 0;

    while (hp != NIL_HOLE && hp->h_base < swap_base && i < (mm_in.m1_i1 - 1)) {
        buffer[0] = hp->h_len;
        buffer[1] = hp->h_base;
        hp = hp->h_next;
        sys_copy(MM_PROC_NR, D, (phys_bytes)buffer, who, D, (phys_bytes)(mm_in.m1_p1 + (8 * i)), (phys_bytes)8);
        i++;
    }

    buffer[0] = 0;

    sys_copy(MM_PROC_NR, D, (phys_bytes)buffer, who, D, (phys_bytes)(mm_in.m1_p1 + (8 * i)), (phys_bytes)4);

    return i;
}
```

### 5. Implementation of `do_worst_fit`

The `do_worst_fit` function allows enabling or disabling the Worst Fit algorithm:

```c
PUBLIC int do_worst_fit() {
    int enable = mm_in.m1_i1;
    use_worst_fit = (enable != 0);
    return OK;
}
```

- **Purpose**: Toggles the `use_worst_fit` flag based on the input parameter.

---
