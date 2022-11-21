#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(int argc, char *argv[])
{
    int n, id;
    double z, x = 0.0;

    if (argc < 2)
    {
        n = 5;
    }
    else
    {
        n = atoi(argv[1]);
    }

    if (n < 1 || n > 10)
        n = 5;

    id = fork();

    if (id == 0)
    { // child
        printf(1, "Child %d created\n", getpid());
        set_proc_priority(getpid(), n);
        for (z = 0; z < 8000000.0; z += 0.001)
            x = x + 3.14 * 69.69; // Useless calculations to consume CPU time
    }
    else if (id > 0)
    { // parent
        printf(1, "Parent %d created child %d\n", getpid(), id);
        set_proc_priority(getpid(), n);
        wait();
    }
    else
    {
        printf(1, "%d failed in fork!\n", getpid());
    }

    exit();
}