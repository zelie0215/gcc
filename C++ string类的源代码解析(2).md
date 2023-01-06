# C++ string类的源代码解析(2)

## 1.string类的实现方式

#### 		1.深拷贝[^深拷贝]

#### 	**2.COW[^写时拷贝]\(copy on write)**



>在之前多数是使用COW方式 但由于多线程的使用越来越多 COW技术在多线程中会有额外的性能恶化 所以现在多数使用深拷贝的方式 

------



## ==下面以GCC源码为例进行演示==

string的主要内容在<string>、<basic_string.h>、<basic_string.tcc>

### 先介绍几个概念

### 			1.size[^size]

### 			2.capacity[^capacity]

## 今天主要来介绍一下深拷贝

在<string>文件中有如下代码

~~~c++
//源码来自<string>
using string = basic_string<cahr>:
~~~



从这里我们也可以看出来 string只是真是的样子是basic_string 

下面找到<basic_string>文件  就看到basic_string的结构

~~~C++
template <typename _CharT, typename _Traits, typename _Alloc>
class basic_string {
    // Use empty-base optimization: http://www.cantrip.org/emptyopt.html
    struct _Alloc_hider : allocator_type  // TODO check __is_final
    {
        _Alloc_hider(pointer __dat, const _Alloc& __a) : allocator_type(__a), _M_p(__dat) {}

        _Alloc_hider(pointer __dat, _Alloc&& __a = _Alloc()) : allocator_type(std::move(__a)), _M_p(__dat) {}

        /**
         * _M_p指向实际的数据
         */
        pointer _M_p;  // The actual data.
    };

    _Alloc_hider _M_dataplus;
    /**
     * 真实数据的长度，等价于前面介绍的STL中的size
     */
    size_type _M_string_length;

    enum { _S_local_capacity = 15 / sizeof(_CharT) };

    /**
     * 这里有个小技巧，用了union
     * 因为使用_M_local_buf时候不需要关注_M_allocated_capacity
     * 使用_M_allocated_capacity时就不需要关注_M_local_buf
     * 继续向下看完您就会明白。
     */
    union {
        _CharT _M_local_buf[_S_local_capacity + 1];
        /**
         * 内部已经分配的内存的大小，等价于前面介绍的STL中的capacity
         */
        size_type _M_allocated_capacity;
    };
};
~~~



#### 普通构造函数

下面这段代码

~~~c++
string str;
~~~

他会调用普通构造函数 其中对应的源代码如下：

~~~c++
basic_string() : _M_dataplus(_M_local_data()) 
{ 
    _M_set_length(0); 
}
~~~



#### \_M_local_data()

而_M_local_data()的实现如下：

```cpp
const_pointer _M_local_data() const { 
    return std::pointer_traits<const_pointer>::pointer_to(*_M_local_buf); 
}
```

这里这样看出 ——M_dataplus表示实际存放数据的地方 当string为空时候 其实就是指向\_M_local_buf ,并且\_M_string_length是0

#### 当由char*构造string时，构造函数如下：

```cpp
basic_string(const _CharT* __s, size_type __n, const _Alloc& __a = _Alloc()) : _M_dataplus(_M_local_data(), __a) 
{
    _M_construct(__s, __s + __n);
}
```



首先是让 \_M_datapuls 指向 \_M_local_buf , 再看下\_M_construct的实现 

### _M_construct()

```cpp
/***
 * _M_construct有很多种不同的实现，不同的迭代器类型有不同的优化实现，
 * 这里我们只需要关注一种即可，整体思路是相同的。
 */
template <typename _CharT, typename _Traits, typename _Alloc>
template <typename _InIterator>
void basic_string<_CharT, _Traits, _Alloc>::_M_construct(_InIterator __beg, _InIterator __end,
                                                         std::input_iterator_tag) {
    size_type __len = 0;
    size_type __capacity = size_type(_S_local_capacity);
    // 现在__capacity是15，注意这个值等会可能会改变

    while (__beg != __end && __len < __capacity) {
        _M_data()[__len++] = *__beg;
        ++__beg;
    }
    /** 现在_M_data()指向的是_M_local_buf
     * 上面最多会拷贝_S_local_capacity即15个字节，继续往下看，
     * 当超过_S_local_capacity时会重新申请一块堆内存，_M_data()会去指向这块新内存
     */
    __try {
        while (__beg != __end) {
            if (__len == __capacity) {
                /**
                 * 就是在这里，当string内capacity不够容纳len个字符时，会使用_M_create去扩容
                 * 这里你可能会有疑惑，貌似每次while循环都会去重新使用_M_create来申请多一个字节的内存
                 * 但其实不是，_M_create的第一个参数的传递方式是引用传递，__capacity会在内部被修改，稍后会分析
                 */
                __capacity = __len + 1;
                pointer __another = _M_create(__capacity, __len);
                /**
                 * 把旧数据拷贝到新的内存区域，_M_data()指向的是旧数据，__another指向的是新申请的内存
                 */
                this->_S_copy(__another, _M_data(), __len);
                /**
                 * __M_dispose()
                 * 释放_M_data()指向的旧数据内存，如果是_M_local_buf则不需要释放，稍后分析
                 */
                _M_dispose();
                /**
                 * _M_data()
                 * 内部的指向内存的指针指向这块新申请的内存__another，它的实现其实就是
                 * void _M_data(pointer __p) { _M_dataplus._M_p = __p; }
                 */
                _M_data(__another);
                /**
                 * _M_allocated_capacity设置为__capacity
                 * 实现为 void _M_capacity(size_type __capacity) { _M_allocated_capacity = __capacity; }
                 */
                _M_capacity(__capacity);
            }
            _M_data()[__len++] = *__beg;
            ++__beg;
        }
    }
    __catch(...) {
        /**
         * 异常发生时，避免内存泄漏，会释放掉内部申请的内存
         */
        _M_dispose();
        __throw_exception_again;
    }
    /**
     * 最后设置string的长度为__len
     * 实现为void _M_length(size_type __length) { _M_string_length = __length; }
     */
    _M_set_length(__len);
}
```

### 内存申请函数：_M_create()

```cpp
/**
 * @brief _M_create表示申请新内存
 * @param __capacity 想要申请的内存大小，注意这里参数传递方式是引用传递，内部会改变其值
 * @param __old_capacity 以前的内存大小
 */
template <typename _CharT, typename _Traits, typename _Alloc>
typename basic_string<_CharT, _Traits, _Alloc>::pointer basic_string<_CharT, _Traits, _Alloc>::_M_create(
    size_type& __capacity, size_type __old_capacity) {
    /**
     * max_size()表示标准库容器规定的一次性可以分配到最大内存大小
     * 当想要申请的内存大小最大规定长度时，会抛出异常
     */
    if (__capacity > max_size()) std::__throw_length_error(__N("basic_string::_M_create"));

    /**
     * 这里就是常见的STL动态扩容机制，其实常见的就是申请为__old_capacity的2倍大小的内存，最大只能申请max_size()
     * 注释只是说了常见的内存分配大小思想，不全是下面代码的意思，具体可以直接看下面这几行代码哈
     */
    if (__capacity > __old_capacity && __capacity < 2 * __old_capacity) {
        __capacity = 2 * __old_capacity;
        // Never allocate a string bigger than max_size.
        if (__capacity > max_size()) __capacity = max_size();
    }

    /**
     * 使用内存分配子去分配__capacity+1大小的内存，+1是为了多存储个\0
     */
    return _Alloc_traits::allocate(_M_get_allocator(), __capacity + 1);
}
```

### 内存释放函数_M_dispose()

```cpp
/**
 * 如果当前指向的是本地内存那15个字节，则不需要释放
 * 如果不是，则需要使用_M_destroy去释放其指向的内存
 */
void _M_dispose() {
    if (!_M_is_local()) _M_destroy(_M_allocated_capacity);
}

/**
 * 判断下当前内部指向的是不是本地内存
 * _M_local_data()即返回_M_local_buf的地址
 */
bool _M_is_local() const { return _M_data() == _M_local_data(); }

void _M_destroy(size_type __size) throw() { 
    _Alloc_traits::deallocate(_M_get_allocator(), _M_data(), __size + 1); 
}
```



### basic_string的拷贝构造函数：

```c++
/**
 * basic_string的拷贝构造函数
 * 其实就是每次都做一次深拷贝
 */
basic_string(const basic_string& __str)
    : _M_dataplus(_M_local_data(), _Alloc_traits::_S_select_on_copy(__str._M_get_allocator())) {
    _M_construct(__str._M_data(), __str._M_data() + __str.length());
}
```



### basic_string的赋值构造函数：

```cpp
/**
 * 赋值构造函数，调用了assign函数
 */
basic_string& operator=(const basic_string& __str) { return this->assign(__str); }

/**
 * 调用了_M_assign函数
 */
basic_string& assign(const basic_string& __str) {
    this->_M_assign(__str);
    return *this;
}

/**
 * 赋值的核心函数
 */
template <typename _CharT, typename _Traits, typename _Alloc>
void basic_string<_CharT, _Traits, _Alloc>::_M_assign(const basic_string& __str) {
    if (this != &__str) {
        const size_type __rsize = __str.length();
        const size_type __capacity = capacity();

        /**
         * 如果capacity不够用，需要进行重新分配
         */
        if (__rsize > __capacity) {
            size_type __new_capacity = __rsize;
            pointer __tmp = _M_create(__new_capacity, __capacity);
            _M_dispose();
            _M_data(__tmp);
            _M_capacity(__new_capacity);
        }

        /**
         * 将__str指向的内存拷贝到当前对象指向的内存上
         */
        if (__rsize) this->_S_copy(_M_data(), __str._M_data(), __rsize);

        _M_set_length(__rsize);
    }
}
```



### basic_string的移动构造函数：

```cpp
/**
 * 移动构造函数，其实就是把src指向的内存移动到了dst种
 */
basic_string(basic_string&& __str) noexcept : _M_dataplus(_M_local_data(), std::move(__str._M_get_allocator())) {
    if (__str._M_is_local()) {
        traits_type::copy(_M_local_buf, __str._M_local_buf, _S_local_capacity + 1);
    } else {
        _M_data(__str._M_data());
        _M_capacity(__str._M_allocated_capacity);
    }

    // Must use _M_length() here not _M_set_length() because
    // basic_stringbuf relies on writing into unallocated capacity so
    // we mess up the contents if we put a '\0' in the string.
    _M_length(__str.length());
    __str._M_data(__str._M_local_data());
    __str._M_set_length(0);
}
```





+++



# 结束语

​		头好痒 是不是要涨脑子






[^写时拷贝]: C++中的写时拷贝技术是通过“引用计数”来实现的。也就是说，在每次分配内存时，会多分配4个字节，用来记录有多少个指针指向该内存块。当有新的指针指向该内存块时，就将它的“引用计数”加1；当要释放该空间时，就将相应的“引用计数”减1。当“引用计数”为0时，就释放该内存。当某个指针要修改内存中的内容时再为这个指针分配自己的空间。
[^深拷贝]: 如果要修改原来内存上的内容，就需要重新分配内存并将原来内存上的内容拷贝到新内存上，再进行修改，即深拷贝。
[^size]: 表示真实数据的大小，一般resize函数改变的就是这个值。
[^capacity]: 表示内部实际已经分配的内存大小，capacity一定大于等于size，当size超过这个容量时会触发重新分配机制，一般reserve函数改变的就是这个值。





