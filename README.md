# 高性能多线程并发服务器

该项目是基于[muduo](https://github.com/chenshuo/muduo)项目实现的高性能多线程并发服务器，做这个项目的主要动机是在学习了[The Linux Programming Interface](https://en.wikipedia.org/wiki/The_Linux_Programming_Interface)这本书之后，想基于学习内容进行一定实操，恰好被推荐了陈硕的[《Linux多线程服务端编程——使用muduoC++网络库》](Linux多线程服务端编程——使用muduoC++网络库)，之前有一定的C++基础，于是着手从muduo开始构建自己的网络库

# 特点

该项目使用C++11标准，在muduo基础上进行了一定简化，muduo为了适应mac等系统增加了一部分额外代码，在编译工具上也有多种选择（Cmake、Bazel），本项目仅仅基于Cmake进行构建，去掉了大实现复杂编译自动化的部分Cmake代码，实现更为简单明了，除此之外，本项目加入了大量注释，便于初学者理解各部分功能

# 实现类说明

项目基础部分的类，也就是base部分主要是线程同步、线程、锁、、条件变量、日志、时间、原子操作等的C++类封装，该部分实现较为简单，其中互斥锁的实现利用C++的RAII机制实现了可随对象析构进行解锁的mutex，该项目核心类库是net部分，其中核心类实现讲解如下：
### EventLoop
核心类，该对象负责管理所有的IO事件，创建了该对象的线程就是一个IO线程，在同一个线程中只允许存在一个这样的线程，其核心函数是loop()，在这个函数调用中做了三件事：调用poller成员的poll函数获取活跃的channel，对每个活跃channel执行负责的回调函数，执行其他必须在IO线程中完成的任务
### Channel
负责管理某个文件描述符的回调函数，存储这个文件描述关心的事件类型以及其对应的回调函数，并借助其所属的EventLoop对象调用其Poller来更新channel状态
### Poller
是epoll和poll实现的基类，主要负责对channel的状态更新，负责poll功能调用，本项目实现了poll以及epoll系统调用的LT模式
### TcpConnection
负责管理一个Tcp连接，一个tcp连接由一对IP端口确定，分别是local端以及peer端，同时还有一个socket，管理这个socket的回调需要一个channel对象，该channel创建时就绑定到了一个EventLoop上

# 线程池机制
在主线程维护一个EventLoop的线程池，创建多个IO线程，每个IO线程中都创建一个EventLoop对象，以Round-Robin的方式取用每个Eventloop对象
# 构建模式
main函数中，创建一个EventLoop对象作为base_loop，server类中创建一个EventLoop线程池对象，若线程数设置为0，则线程池中并未创建新的EventLoop，若线程数设置为大于0，则创建了一个线程池，其中每个线程存在一个EventLoop对象，在base_loop中进行listen，通过acceptor的listen获得新的连接，每次获得新连接，就从EventLoop线程池以round-robin方式取一个loop出来，创建一个TcpConnection由这个loop进行管理，每个TcpConnection保存了处理连接的回调函数和处理消息的回调函数，在每个loop线程中进行处理
# http解析类
为了做性能测试，实现了一个简单的http解析类，解析http请求头，使用HttpRequest类管理这些信息，并构建HttpResponse类，存放响应报文数据
# 代码量分析
![代码量分析](https://static.code-david.cn/blog/web_server代码分析.png)
# 性能测试
使用webbench进行测试，测试结果如下：
- 不使用线程池：
![单线程测试](https://static.code-david.cn/blog/webbench_test.png)
- 线程池=4
![使用线程池](https://static.code-david.cn/blog/webbench_线程池.png)