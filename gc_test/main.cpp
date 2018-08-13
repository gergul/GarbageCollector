#include "gc.h"

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


void main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);    //设置调试堆函数跟踪分配的方式 
	_CrtSetBreakAlloc(0);

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

	Test t;
	GC::instance.collect();

	p = new(&t)Test;
	GC::instance.collect();
}