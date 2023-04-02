#include <pthread.h>


//饿汉模式，类被加载就创建单体实例的对象
//只读，多线程安全
//即便没有使用单体实例也会被创建，导致资源浪费
class Singleton1{
private:
    Singleton1() {}
    Singleton1(const Singleton1 &rhs) {}
    Singleton1& operator=(const Singleton1 &rhs) {}

    static Singleton1* m_pInstance;

public:
    static Singleton1* GetInstance()
    {
        return m_pInstance;
    }
};

Singleton1* Singleton1::m_pInstance = new Singleton1();



//懒汉模式，调用时再实例化单体实例
//多线程不安全，如果一个线程检查了为空，被调度到另一个线程，检查也为空，那么会创建两个实例
//严格来说都不是单例了
class Singleton2{
private:
    Singleton2() {}
    Singleton2(const Singleton2 &rhs) {}
    Singleton2& operator=(const Singleton2 &rhs) {}

    static Singleton2* m_pInstance;

public:
    static Singleton2* GetInstance()
    {
        if(m_pInstance == nullptr){
            m_pInstance = new Singleton2();
        }

        return m_pInstance;
    }
};

Singleton2* Singleton2::m_pInstance = nullptr;



//懒汉模式，检查是否为空时加锁
//多线程安全，但是加锁效率低，每次判断为空都需加锁，实际上只有一次需要加锁
//存在效率问题
class Singleton3
{
private:
    Singleton3() {
        pthread_mutex_init(&m_mutex,NULL);
    }
    Singleton3(const Singleton3 &rhs) {}
    Singleton3& operator=(const Singleton3 &rhs) {}

    static Singleton3* m_pInstance;
    static pthread_mutex_t m_mutex;

public:
    ~Singleton3(){
        pthread_mutex_destroy(&m_mutex);
    }

    static Singleton3* GetInstance()
    {
        //当后续单体实例已经创建后，其实就没必要对这里进行加锁了
        pthread_mutex_lock(&m_mutex);
        if(m_pInstance == nullptr){
            m_pInstance = new Singleton3();
        }
        pthread_mutex_unlock(&m_mutex);

        return m_pInstance;
    }
};

Singleton3* Singleton3::m_pInstance = nullptr;




//懒汉模式，在生成单体实例时加锁
//多线程不安全，可能会创建多个实例，还可能存在下面说的非法访问内存的问题（不太确定是不是这样）
//这个问题其实可以使用内存屏障解决
//内存屏障：屏障点之前的指令必须都执行完之后才能执行屏障点之后的指令
class Singleton4{
private:
    Singleton4() {
        pthread_mutex_init(&m_mutex,NULL);
    }
    Singleton4(const Singleton3 &rhs) {}
    Singleton4& operator=(const Singleton3 &rhs) {}

    static Singleton4* m_pInstance;
    static pthread_mutex_t m_mutex;

public:
    ~Singleton4(){
        pthread_mutex_destroy(&m_mutex);
    }

    static Singleton4* GetInstance()
    {
        
        if(m_pInstance == nullptr)
        {
            //正常程序执行过程为：分配内存——构造对象——地址赋值；
	        //但编译器可能优化成：分配内存——地址赋值——构造对象；
	        //这时，可能在地址赋值后但未构造对象前，另一个线程进来检查不为空而直接返回，但实际上对象此时不能用
            pthread_mutex_lock(&m_mutex);
            m_pInstance = new Singleton4();
            /*
                //如果使用屏障，对象的构造总在屏障前完成，单例对象被赋值时对象一定被构造好了
                Singleton4* tmp = new Singleton4();
                barrier();
                m_pInstance = tmp;
            */
            pthread_mutex_unlock(&m_mutex);
        }

        return m_pInstance;
    }
};

Singleton4* Singleton4::m_pInstance = nullptr;




//懒汉模式，使用双检查+锁
//线程安全
//但其实如果被过度优化，也会出现Sigleton4中的问题
class Singleton5{
    private:
    Singleton5() {
        pthread_mutex_init(&m_mutex,NULL);
    }
    Singleton5(const Singleton3 &rhs) {}
    Singleton5& operator=(const Singleton3 &rhs) {}

    static Singleton5* m_pInstance;
    static pthread_mutex_t m_mutex;

public:
    ~Singleton5(){
        pthread_mutex_destroy(&m_mutex);
    }

    static Singleton5* GetInstance()
    {
        if(m_pInstance == nullptr)
        {
            pthread_mutex_lock(&m_mutex);
            if(m_pInstance == nullptr){
                m_pInstance = new Singleton5();
            }
            pthread_mutex_unlock(&m_mutex);
        }

        return m_pInstance;
    }

};

Singleton5* Singleton5::m_pInstance = nullptr;




//懒汉模式
//线程安全
//C++11中，如果当变量在初始化的时候，并发同时进入声明语句，并发线程将会阻塞等待初始化结束。
class Singleton6{
private:
    Singleton6() {}
    Singleton6(const Singleton6 &rhs) {}
    Singleton6& operator=(const Singleton6 &rhs) {}

public:
    ~Singleton6() {}
    static Singleton6& GetInstance()
    {
        static Singleton6 instance;
        return instance;
    }
};
