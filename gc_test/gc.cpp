// Copyright (C) 2009 Chris Double. All Rights Reserved.
// See the license at the end of this file
#include "gc.h"
#include <vector>

// GCObject
GCObject::GCObject()
	: mMarked(false)
{

}

GCObject::GCObject(GCObject const&)
	: mMarked(false)
{
	
}

GCObject::~GCObject()
{
}

void GCObject::mark()
{
	if (!mMarked)
	{
		mMarked = true;
		markChildren();
	}
}

void GCObject::markChildren()
{
}

void* GCObject::operator new(size_t size)
{
	void* p = ::operator new(size);
	GC::instance.addObject((GCObject*)p);
	return p;
}

void* GCObject::operator new(size_t size, void* p)
{
	return ::operator new(size, p);
}

void GCObject::operator delete(void *p)
{
	GC::instance.removeObject((GCObject*)p);
	::operator delete(p);
}

void* GCObject::operator new[](size_t size)
{
	void* p = ::operator new(size);
	return p;
}

void GCObject::operator delete[](void *p)
{
	::operator delete(p);
}

// GCMemory
GCMemory::GCMemory(int size)
	: mSize(size)
{
	mMemory = new unsigned char[size];
}

GCMemory::~GCMemory()
{
	delete[] mMemory;
}

unsigned char* GCMemory::get()
{
	return mMemory;
}

int GCMemory::size()
{
	return mSize;
}


// GarbageCollector
GC GC::instance;

void GC::collect()
{
	MUTEXT_LOCK;
	// Mark root objects
	for (ObjectSet::iterator it = mRoots.begin();
		it != mRoots.end(); ++it)
	{
		(*it)->mark();
	}

	// Mark pinned objects
	for (PinnedSet::iterator it = mPinned.begin();
		it != mPinned.end(); ++it)
	{
		(*it).first->mark();
	}
	MUTEXT_UNLOCK;

	sweep();
}

void GC::addRoot(GCObject* root)
{
	MUTEXT_LOCK;
	mRoots.insert(root);
	MUTEXT_UNLOCK;
}

void GC::removeRoot(GCObject* root)
{
	MUTEXT_LOCK;
	mRoots.erase(root);
	MUTEXT_UNLOCK;
}

void GC::pin(GCObject* o)
{
	MUTEXT_LOCK;
	PinnedSet::iterator it = mPinned.find(o);
	if (it == mPinned.end())
	{
		mPinned.insert(std::make_pair(o, 1));
	}
	else
	{
		(*it).second++;
	}
	MUTEXT_UNLOCK;
}

void GC::unpin(GCObject* o)
{
	MUTEXT_LOCK;
	do 
	{
		PinnedSet::iterator it = mPinned.find(o);
		if (it == mPinned.end())
			break;

		if (--((*it).second) == 0)
			mPinned.erase(it);
	} while (0);
	MUTEXT_UNLOCK;
}

void GC::addObject(GCObject* o)
{
	MUTEXT_LOCK;
	mHeap.insert(o);
	MUTEXT_UNLOCK;
}

void GC::removeObject(GCObject* o)
{
	MUTEXT_LOCK;
	mHeap.erase(o);
	MUTEXT_UNLOCK;
}

void GC::sweep()
{
	std::vector<ObjectSet::iterator> erase;

	MUTEXT_LOCK;
	for (ObjectSet::iterator it = mHeap.begin();
		it != mHeap.end(); ++it)
	{
		GCObject* p = *it;
		if (p->mMarked)
		{
			p->mMarked = false;
		}
		else
		{
			erase.push_back(it);
		}
	}
	MUTEXT_UNLOCK;

	for (std::vector<ObjectSet::iterator>::iterator it = erase.begin();
		it != erase.end(); ++it)
	{
		GCObject* p = **it;
		//mHeap.erase(*it);
		delete p;
	}
}

int GC::liveCount()
{
	MUTEXT_LOCK;
	int nCount = mHeap.size();
	MUTEXT_UNLOCK;
	return nCount;
}

// Copyright (C) 2009 Chris Double. All Rights Reserved.
// The original author of this code can be contacted at: chris.double@double.co.nz
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// DEVELOPERS AND CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
