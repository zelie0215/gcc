# C/C++  结构体(1)构造函数

+++++

首先来说来一下在C++中结构体有两种

> 1.struct
>
> 2.class

## 一、基本用法

#### 1.构造函数概念：

>一个类的对象被创建的时候，编译系统对象分配内存空间，并自动调用该构造函数，由构造函数完成成员的初始化工作。*因此，构造函数的核心作用就是，**初始化对象的数据成员***

#### 2、构造函数的特点：

> 1.名字与**类名**相同 可以有参数 但不能有返回值
>
> ~~~c++
> class A{
>   int a;
>     A(){
>       a = 2;
>   }  
> };
> ~~~
>
> 2. 构造函数在实例化对象时自动调用 不需要手动调用
>
>    ~~~c++
>    A a;
>    printf("%d",a->a);//打印2;
>    ~~~
>
> 3. 作用是对象进行初始化工作
>
> 4. 如果定义类的时候没有写构造函数 系统会生成一个默认的无参数构造函数
>
> 5. 如果定义了构造函数 就不再生成默认的 午餐构造函数
>
> 6. 一个类中可以有多种构造函数 为重载关系

#### 3、构造函数的分类：

>- 按参数种类分为：有参构造函数 无参构造函数 有默认参构造寒素
>- 按类型分为：普通构造函数 拷贝构造函数(赋值构造函数)

#### 4.源码分析：

~~~c++
struct Time{
   
    //声明类内变量
    int hour;
    int min;
    int day;
    
     //无参数构造函数
    Time(){
        day = 0;
        hour = 0;
        min = 0;
    }
    
    //设置时间函数
    void SetTime(int day,int hour,int min){
        day =  day;
        hour = hour;
        min = min;
    }
    
    //打印时间函数
    void OutTime(){
        cout<<day<<"天"<<hour<<"时"<<min<<"分"<<endl;
    }
    
};

int main(){
    Time time;
    time.SetTime(5,2,0);// 设置时间为5 2 0
    time .OutTime(); // 打印 5 2 0;
    return 0;
    
}
~~~



## 二、带参数的构造函数

> 简单说就是构造函数中带参数 然后对参数进行操作

> ```c++
> //假设我们上面定义的Time类 
> Time time1;//这种方法是无参的构造函数
> Time,OutTime(); //打印0 0 0
> ```
>
> 如果我们在定义中加入有参构造
>
> ~~~c++
> Time(int day,int hour,int min) : day(day),hour(hour),min(min); //这种写法是为初始化列表
> ~~~
>
> ~~~c++
> Time time2(5,2,0);
> time.OutTime(); //打印5 2 0 
> ~~~

## 三、带默认参的构造函数

>还是之前的Time类
>
>~~~c++
>//写入以下代码
>Time(int day = 5,int hour = 2; int min = 0){
>    day = day;
>    hour = hour;
>    min = min ;
>}
>~~~
>
>我们在调用
>
>~~~c++
>Time time3(5); //因为传入一个参数 默认对应第一个参数 即 day =5 hour = 2 min = 0; 
>time.OutTime(); // 打印5 2 0 
>~~~
>
>
>
>

##  四、拷贝构造函数

>~~~c++
>Time(const Time &p){ //const Time &p   p为引用类型 相当于 p=实例化对象
>    day = p.day;
>    hour = p.hour;
>    min = p.min;
>}
>Time time4(time1);
>time4.OutTime();
>~~~



​                                                                                                                                                                                                                                                                                                                                                                                                                                                       