#ifndef CMD_WEAK_PTR_H
#define CMD_WEAK_PTR_H

class Referenced;
struct WeakPtrBase
{
	// _ Prefixed items should not be used by external classes, except for the Referenced and WeakPtr<T>::Type class
	inline void _Link(Referenced* obj); // internal usage
	inline WeakPtrBase();
	inline ~WeakPtrBase();
	inline void _Unlink(); // internal usage

	Referenced *_object;
	WeakPtrBase *_prev;
	WeakPtrBase *_next;
};

// similar to osg::Referenced
class Referenced
{
public:
	Referenced() {
		refCount = 0;
		weakPtrList = 0;
	}
	virtual ~Referenced() {
		for(WeakPtrBase *p = weakPtrList; p; p = p->_next)
			p->_object = 0;
	}
	int GetRefCount() { return refCount; }

	void AddRef() { refCount++; }
	void Release() {
		refCount --;
		if (refCount == 0)
			delete this;
	}

private:
	mutable int refCount;
	mutable WeakPtrBase* weakPtrList; // linked list of weak pointers

	friend struct WeakPtrBase;

	template<typename T>
	friend class RefPtr;
};

WeakPtrBase::WeakPtrBase()
{
	_object = 0;
	_prev = _next = 0;
}

WeakPtrBase::~WeakPtrBase()
{
	_Unlink();
}

void WeakPtrBase::_Link(Referenced* obj)
{
	_prev = 0;
	_next = obj->weakPtrList;
	obj->weakPtrList = this;
	_object = obj;
}

void WeakPtrBase::_Unlink()
{
	if(_object)
	{
		if (_object->weakPtrList == this) 
			_object->weakPtrList = _next;
		else _prev->_next = _next;

		if (_next) _next->_prev = _prev;		
		_object = 0;
	}
}

template<typename T>
class WeakPtr : public WeakPtrBase
{
public:
	WeakPtr()
	{}
	explicit WeakPtr(T* ptr)
	{
		_Link(ptr);
	}
	void Set(T* ptr)
	{
		_Unlink();
		_Link(ptr);
	}
	T* Get() const { return (T*)_object; }

	WeakPtr& operator=(T* ptr) { Set(ptr); }
	const T* operator*() const { return (T*)_object; }
	T* operator*() { return (T*)_object; }
	operator bool() const { return _object>=0; }

	class Type : public creg::IType
	{
	public:
		Type() {}
		std::string GetName() { return "WeakPtr<" + T::StaticClass()->name + ">"; }
		static void WeakPtrInitCallback(void *userdata) 
		{
			WeakPtrBase *wpb = (WeakPtrBase *)userdata;
			Referenced *obj = wpb->_object;
			wpb->_object = 0; 
			// now assign it the right way
			wpb->_Link(obj);
		}
		void Serialize(creg::ISerializer *s, void *instance)
		{
			WeakPtr<T> *wp = (WeakPtr<T>*)instance;
			if(s->IsWriting()) {
				void *ptr = (void*)wp->Get();
				s->SerializeObjectPtr(&ptr, wp->Get() ? wp->Get()->GetClass() : 0);
			} else {
				WeakPtrBase *wbp = (WeakPtrBase *)instance;
				s->SerializeObjectPtr((void**)&wbp->_object, T::StaticClass());
				s->AddPostLoadCallback(WeakPtrInitCallback, instance);
			}
		}
	};
};

template<typename T>
class RefPtr
{
public:
	RefPtr() { object = 0; }
	RefPtr(T* obj) { Link(obj); }
	RefPtr(const RefPtr<T>& r)
	{
		object = r.object;
		if (object) object->AddRef();
	}
	~RefPtr() 
	{ 
		Unlink(); 
	}
	void Unlink()
	{
		if (object) {
			object->Release();
			object=0;
		}
	}
	void Link(T* obj)
	{
		if (obj)
			obj->AddRef();
		object = obj;
	}
	operator bool()
	{
		return object != 0;
	}
	RefPtr& operator=(const RefPtr &obj) 
	{
		if(object!=obj.object) {
			Unlink();
			Link(obj.object);
		}
		return *this;
	}
	T* Get() const { return object; }
	T* operator*() const { return object; }
	T* operator->() const { return object; }
	
private:
	T* object;

public:
	
	class Type : public creg::ObjectPointerType<T>
	{
	public:
		std::string GetName() { return "RefPtr<" + T::StaticClass()->name + ">"; }
		static void FixRefCount(void *userdata) 
		{
			RefPtr<T> *rp = (RefPtr<T>*)userdata;
			if (rp->Get())
				rp->Get()->AddRef();
		}
		void Serialize(creg::ISerializer *s, void *instance)
		{
			creg::ObjectPointerType<T>::Serialize (s, instance);
			s->AddPostLoadCallback (FixRefCount, instance);
		}
	};

};

// creg registration
namespace creg 
{
	// RefPtr
	template<typename T> struct DeduceType < RefPtr <T> > {
		IType* Get () { return new typename RefPtr <T>::Type; }
	};

	// WeakPtr
	template<typename T> struct DeduceType < WeakPtr <T> > {
		IType* Get() { return new typename WeakPtr <T>::Type; }
	};
}


#endif
