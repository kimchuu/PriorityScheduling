# 1. 시스템 콜 구현

### 1. user.h에 아래 함수 헤더 넣어주기
```
set_proc_priority(int, int);
get_proc_priority(int);
int cps(void);
```

### 2. usys.S에 system call define 해주기
```
SYSCALL(set_proc_priority) 
SYSCALL(get_proc_priority) 
SYSCALL(cps)
```

### 3. syscall.h에 system call number define 해주기
```
#define SYS_set_proc_priority 23
#define SYS_get_proc_priority 24
#define SYS_cps 25
```

### 4. syscall.c
trap에서 syscall()를 부르면 syscall-table에서 index에 해당하는 값(함수)를 호출하게 됩니다.

- systemcalls 배열에 위에서 만든 system call number index에 호출할 함수 넣어주기
```
static int (*syscalls[])(void) = {
...
[SYS_set_proc_priority] sys_set_proc_priority,
[SYS_get_proc_priority] sys_get_proc_priority,
[SYS_cps] sys_cps,
};
```

- 호출할 함수 extern 변수로 선언해주기
```
extern int sys_set_proc_priority(void);
extern int sys_get_proc_priority(void);
extern int sys_cps(void);
```

### 5. sysproc.c
system call에 의해 불린 함수를 구현해놓은 파일입니다. 각각의 함수에서 kernel mode에 있는 함수를 호출하게 됩니다.
argument가 필요한 함수의 경우 argint을 이용하여 register에 저장해놓은 argument 값을 받아옵니다. 
예를 들어 sys_set_proc_priority(void)는 set_proc_priority(int, int)를 호출해야하므로 argint(0, &pid) 와 argint(1, &priority)를 사용하여 두 개의 argument를 각각 pid와 priority에 받아옵니다.

```
int sys_cps(void)는 cps()를 return하고,
int sys_set_proc_priority(void)는 set_proc_priority(pid, priority)를 return하고,
int sys_get_proc_priority(void)는 get_proc_priority(pid)를 return합니다.
```

### 6. defs.h에 3개의 함수 헤더 적기
커널에서 돌아가는 함수의 헤더를 적어놓은 파일입니다.
주석 proc.c 아래에 다음과 같이 추가해주면 됩니다.

```
void set_proc_priority(int, int);
int get_proc_priority(int);
int cps(void);
```

### 7. proc.c에서 3개의 함수 구현
set_proc_priority는 pid값을 받아 해당 process의 priority를 바꿔주는 함수이고,
get_proc_priority는 pid값을 받아 process의 priority를 return해주는 함수고,
cps는 ptable의 proc 배열을 돌면서 배열에 있는 각 프로세스의 상태 및 priority, exec_num(실행횟수) 등을 print 해주는 함수입니다.

### 8. nice.c, foo.c, ps.c 세 개의 main 함수 생성
- nice.c : set_proc_priority 시스템 콜을 호출하는 실행파일입니다.
- foo.c : fork()를 통해 부모&자식 process를 만들고, priority 또한 지정해줄 수 있는 실행파일입니다.
- ps.c : cps를 호출하여 현재 process의 상태를 출력해주는 실행파일입니다.

위 3개는 shell에서 명령어로 입력하면 실행 가능합니다.

### 9. Makefile
명령어로 위에서 만든 파일을 실행시켜 주긴 위해서는 Makefile에 추가하여 complie하고 실행파일로 만드는 작업이 필요합니다.
UPROGS에 _foo\ _ps\ _nice\을 추가해주고 EXTRA에 nice.c ps.c foo.c를 추가해줍니다. 


# 2. Scheduling

### 1. proc.h의 proc 구조체에 uint priority 변수 추가
아래의 함수 및 구조체는 전부 proc.c 안에 들어있습니다.

### 2. ptable 구조체에 int exec_num[NPROC+1] 변수 추가
NPROC+1로 배열 크기를 설정한 이유는 인덱스를 ptable의 proc 배열과 맞춰주기 위함입니다.  

### 3. allocproc(void) 함수에 p->priority = 5 추가
allocproc에서 process가 할당이 되므로 process가 할당이 될 때 priority = 5로 세팅되게 만들어주었습니다.

### 4. fork(void) 함수에 np->priority = curproc -> priority 추가
자식 프로세스는 부모 프로세스와 같은 priority를 갖는다는 요구사항이 있었으므로 위와 같은 코드를 추가해주었습니다.

### 5. scheduler(void)

- exec_num 배열에서 사용할 인덱스를 구하기 위해 idx, pidx, highPidx 변수를 선언했습니다.
idx는 p(기존 차례의 process)에 해당하는 인덱스 변수이고,
pidx와 highPidx는 highP(priority가 높은 process)의 인덱스 값을 찾기 위한 변수입니다.

- ptable_exec_num[idx] 값을 통한 priority 변경
해당 값이 200보다 크면 실행이 많이 되어 양보할 차례가 되었다는 의미이므로 priority 값을 감소시켜주었고,
해당 값이 -200보다 작으면 실행이 너무 안되어 참을 수 없는 상태가 되었다는 의미이므로 priority를 증가시켜주었습니다.
또한, priority 조정 작업 후에 ptable_exec_num[idx] = 0으로 초기화하여 다시 실행횟수를 세도록 하였습니다.

- highP를 찾는 작업
proc *p1 변수를 선언해 proc 배열을 처음부터 끝까지 돌며 높은 priority를 갖는 process(highP)를 찾도록 하였습니다.
highP와 함께 highPidx의 값도 저장해주어 후에 실행횟수를 늘이는 데 사용하게 했습니다.

- highP와 p 비교
highP == p -> 자기 차례에 자기가 실행된 것 
highP != p -> 자기 차례에 더 priority 높은 process가 실행된 것
따라서 다를 경우 highP의 실행횟수는 높여주고 p의 실행횟수는 낮춰주는 작업을 여기서 했습니다.
다만, priority가 5일 경우 후에 round robin으로 실행될 것이므로 실행횟수 변경 작업을 하지 못하게 했습니다.
