# Part 1: Multithreaded Sorting Application

<font color = #34a853>注意事项：</font>

  在Makefile编写中，pthread 库不是 Linux 系统默认的库，连接时需要使用静态库 libpthread.a，所以在使用pthread_create()创建线程时，在编译中要加 -lpthread参数。

即要将：

  <center> $(CC) -o $@ $^</center>
  
改为：

  <center> $(CC) -o $@ $^ <font color = #ea4335> -lpthread </font> </center>
