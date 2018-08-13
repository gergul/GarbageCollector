#include "gc.h"
#include <Windows.h>

class Test : public GCObject {
public:
	Test()
	{

	}
	~Test()
	{

	}
};

class Test2 : public GCObject {
public:
	Test* mTest;

	virtual void markChildren() {
		mTest->mark();
	}

	Test2()
	{
		
	}
	~Test2()
	{

	}
};

void t1()
{
	for (int i = 0; i < 1000; ++i)
	{
		new Test;
		Sleep(100);
	}
}

void t2()
{
	for (int i = 0; i < 1000; ++i)
	{
		new Test;
		Sleep(100);
	}
}

void t3()
{
	for (int i = 0; i < 500; ++i)
	{
		GC::instance.collect();
		Sleep(200);
	}
}

void main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);    //设置调试堆函数跟踪分配的方式 
	_CrtSetBreakAlloc(0);

	//Normal test
	Test* p = new Test;
	for (int i=0; i<100; ++i)
		GC::instance.pin(p);
	Test2* p2 = new Test2;
	GC::instance.pin(p2);
	p2->mTest = new Test;
	GC::instance.collect();

	for (int i=0; i<900; ++i)
		GC::instance.unpin(p);
	GC::instance.unpin(p2);
	GC::instance.collect();

	{
		//测试栈
		Test t;
		GC::instance.collect();

		//测试Placement new
		p = new(&t)Test;
		GC::instance.collect();
	}
	
	//不可以new数组
	//Test* parr = new Test[10];//这一句编译错误

	//多线程测试
	std::thread thread1(t1);
	std::thread thread2(t2);
	std::thread thread3(t3);
	thread1.join();
	thread2.join();
	thread3.join();
	GC::instance.collect();

	return;
}