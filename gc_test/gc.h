#if !defined(GC_H)
#define GC_H

#include <set>
#include <map>

// For thread safety
//#define THREAD_SAFETY 1

#if THREAD_SAFETY
#include <mutex>
#define MUTEXT_LOCK mMutex.lock()
#define MUTEXT_UNLOCK mMutex.unlock()
#else
#define MUTEXT_LOCK 
#define MUTEXT_UNLOCK 
#endif

// Base class for all objects that are tracked by
// the garbage collector.
class GCObject
{
public:

	// For mark and sweep algorithm. When a GC occurs
	// all live objects are traversed and mMarked is
	// set to true. This is followed by the sweep phase
	// where all unmarked objects are deleted.
	bool mMarked;

public:
	GCObject();
	GCObject(GCObject const&);
	virtual ~GCObject();

	// Mark the object and all its children as live
	void mark();

	// Overridden by derived classes to call mark()
	// on objects referenced by this object. The default
	// implemention does nothing.
	virtual void markChildren();

public:
	void* operator new(size_t size);
	void* operator new(size_t size, void* p);
	void operator delete(void *p);

protected:
	void* operator new[](size_t size);
	void operator delete[](void *p);
};

// Wrapper for an array of bytes managed by the garbage
// collector.
class GCMemory : public GCObject
{
public:
	unsigned char* mMemory;
	int   mSize;

public:
	GCMemory(int size);
	virtual ~GCMemory();

	unsigned char* get();
	int size();
};

// Garbage Collector. Implements mark and sweep GC algorithm.
class GC
{
public:
	// Global garbage collector object
	static GC instance;

public:
	// A collection of all active heap objects.
	typedef std::set<GCObject*> ObjectSet;
	ObjectSet mHeap;

	// Collection of objects that are scanned for garbage.
	ObjectSet mRoots;

	// Pinned objects
	typedef std::map<GCObject*, unsigned int> PinnedSet;
	PinnedSet mPinned;

public:
	// Perform garbage collection. 
	void collect();

	// Add a root object to the collector.
	void addRoot(GCObject* root);
	// Remove a root object from the collector.
	void removeRoot(GCObject* root);

	// Pin an object so it temporarily won't be collected. 
	// Pinned objects are reference counted. Pinning it
	// increments the count. Unpinning it decrements it. When
	// the count is zero then the object can be collected.
	void pin(GCObject* o);
	void unpin(GCObject* o);

	// Add an heap allocated object to the collector.
	void addObject(GCObject* o);
	// Remove a heap allocated object from the collector.
	void removeObject(GCObject* o);

	// Go through all objects in the heap, unmarking the live
	// objects and destroying the unreferenced ones.
	void sweep();

	// Number of live objects in heap
	int liveCount();

protected:

#if THREAD_SAFETY
	std::mutex mMutex;
#endif

};

#endif