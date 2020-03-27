# Lab1: Utilities

这个 Lab 主要内容是：

* setup xv6/riscv env
* 读 xv6 文档，根据提供的一些编程接口完成 Lab1

这个 Lab Note 主要是希望能够介绍一下 Lab1，然后复习一下 Linux Programming Interface. xv6 是照着 Unix 系统写的，但是编程接口肯定是有一定差异的。

## Process/Memory

```c
int pid = fork(); 
if(pid > 0){
		printf("parent: child=%d\en", pid); 
  	pid = wait(0);
		printf("child %d is done\en", pid);
} else if(pid == 0){
		printf("child: exiting\en");
    exit(0);
} else {
		printf("fork error\en");
}
```

这一段涉及了 `fork`, `exit`, `wait`

> * Exit takes an integer status argument, conventionally 0 to indicate success and 1 to indicate failure. 
> * The wait system call returns the pid of an exited child of the current process and copies the exit status of the child to the address passed to wait; if none of the caller’s children has exited, wait waits for one to do so. If the parent doesn’t care about the exit status of a child, it can pass a 0 address to wait. 

fork 需要涉及 child 的 pid, 似乎是如此，所以 child 的 `fork` 拿到的应该是 0, parent 是 child 的 pid.

### 僵尸进程

这里需要考虑僵尸进程，一个子进程可以：

1. exit
2. return

一般认为0正常1不正常，但是无论如何，这个参数会被传给 os 。下面的问题是：父进程请求的时候需要拿到进程相关的转发台，没有拿到这些参数的进程会很自然的变成孤儿进程。

这个时候就需要 `wait` 或者 `waitpid`. 同时，也可以用信号处理的`SIGCHILD`, 这个信号捕获上述信息，便于用户处理

### fork and fd

![WechatIMG40](/Users/fuasahi/Downloads/WechatIMG40.jpeg)

这是 CSAPP 上面一张比较经典的图：

* 进程自己共享一个 descriptor table
  * 进程所有
  * 内容为 file table 的一个表项
* 各个进程分享一个 file table，记录多个进程的 refcnt, 如果你 fork 你会得到一个别的进程指向同一处，如果你 dup 你会得到当个进程的指向同一处。
  * 进程共享
  * 包括当前 refcnt 和 文件位置
  * 指向 v-node 中的指针
  * 内核不会删除它，直到 refcnt == 0
* v-node table
  * 进程共享
  * 文件类型
  * 指向锁列表的指针
  * 各种属性

那么我们讨论一些 case:

* 两个 fd open 了同一个 filename:
  * fd 不同
  * file table 的 record 不同
  * 同一个 v-node
* 父子进程 fork
  * fd 相同
  * 指向同一个 record
* 很明显的一个情况：不同地方 fd open 同一个 file, 有不同 refcnt 和同一个 v-node. unlink fd。