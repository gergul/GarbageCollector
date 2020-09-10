#include "gc.h"
#include <Windows.h>
#include <iostream>

class Test : public GCObject {
public:
	Test()
	{
		std::cout << __FUNCTION__ << std::endl;
	}
	~Test()
	{
		std::cout << __FUNCTION__ << std::endl;
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
		std::cout << __FUNCTION__ << std::endl;
	}
	~Test2()
	{
		std::cout << __FUNCTION__ << std::endl;
	}
};

class Test3 : public GCObject {
public:
	Test* mTest;

	virtual void markChildren() {
		mTest->mark();
	}

	Test3()
	{
		std::cout << __FUNCTION__ << std::endl;
	}
	~Test3()
	{
		std::cout << __FUNCTION__ << std::endl;
	}
};

void t1()
{
	for (int i = 0; i < 100; ++i)
	{
		new Test;
		Sleep(20);
	}
}

void t2()
{
	for (int i = 0; i < 100; ++i)
	{
		new Test;
		Sleep(20);
	}
}

void t3()
{
	for (int i = 0; i < 100; ++i)
	{
		GC::instance.collect();
		Sleep(10);
	}
}

void main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(0);

#if 1 //被pin后的对象将不会被自动回收
	{
		std::cout << "===Normal test===" << std::endl;

		Test* p = new Test;

		Test2* p2 = new Test2;
		p2->mTest = p;

		Test3* p3 = new Test3;
		p3->mTest = p;

		GC::instance.collect();
	}

	{
		std::cout << "===Test pin member ptr===" << std::endl;

		Test* p = new Test;
		GC::instance.pin(p);

		Test2* p2 = new Test2;
		p2->mTest = p;

		Test3* p3 = new Test3;
		p3->mTest = p;
				
		GC::instance.collect();
	}

	{
		std::cout << "===Test pin one child class===" << std::endl;

		Test* p = new Test;

		Test2* p2 = new Test2;
		GC::instance.pin(p2);
		p2->mTest = p;

		Test3* p3 = new Test3;
		p3->mTest = p;
				
		GC::instance.collect();
	}

	{
		std::cout << "===Test pin all child classes===" << std::endl;

		Test* p = new Test;

		Test2* p2 = new Test2;
		GC::instance.pin(p2);
		p2->mTest = p;

		Test3* p3 = new Test3;
		GC::instance.pin(p3);
		p3->mTest = p;

		GC::instance.collect();
	}
#endif

	{
		std::cout << "===Test unpin===" << std::endl;

		Test* p = new Test;
		GC::instance.pin(p);
		GC::instance.unpin(p);
		GC::instance.collect();
	}

	{
		std::cout << "===Test stack===" << std::endl;
		//测试栈
		Test t;
		GC::instance.collect();
	}
	
	{
		std::cout << "===Test Placement new===" << std::endl;

		Test* t = new Test;
		Test* p = new(t)Test;
		GC::instance.collect();
	}

	
	//不可以new数组
	//Test* parr = new Test[10];//这一句编译错误

#if THREAD_SAFETY
	{
		//多线程测试
		std::thread thread1(t1);
		std::thread thread2(t2);
		std::thread thread3(t3);
		thread1.join();
		thread2.join();
		thread3.join();
		GC::instance.collect();
	}
#endif

	system("pause");
	return;
}