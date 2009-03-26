%{
#include <list>

namespace std {
	template<typename T>
	class list_iterator : public list<T>::iterator
	{
	public:
		list_iterator() {}
		list_iterator(const typename list<T>::iterator& li) : typename list<T>::iterator(li) {}
	};
	template<typename T>
	class list_reverse_iterator : public list<T>::reverse_iterator
	{
	public:
		list_reverse_iterator () {}
		list_reverse_iterator (const typename list<T>::reverse_iterator& li) : typename list<T>::reverse_iterator(li) {}
	};
}

%}
%include <std_except.i> // the general exepctions

namespace std {

	template<class T>
	class list_iterator
	{
	public:
		%extend { 
			T value() { return *(*self); }
			void next() { ++(*self); }
			void prev() { --(*self); }
			bool operator==(const list_iterator<T>& li) { return (list<T>::iterator)li == (list<T>::iterator)*self; }
		}
	};
	template<class T>
	class list_reverse_iterator
	{
	public:
		%extend { 
			T value() { return *(*self); }
			void next() { ++(*self); }
			void prev() { --(*self); }
			bool operator==(const list_reverse_iterator<T>& li) { return (list<T>::reverse_iterator)li == (list<T>::reverse_iterator)*self; }
		}
	};

    %rename(begin_it) list::begin();
    %rename(end_it) list::end();
    %rename(rbegin_it) list::rbegin();
    %rename(rend_it) list::rend();
	
	template<class T>
    class list {
      public:
      
        list();
        list(const list&);
        list(unsigned int,T);
        unsigned int size() const;
        bool empty() const;
        void clear();
        void push_back(T val);
        void pop_back();
        void push_front(T val);
        void pop_front();
        T front()const; // only read front & back
        T back()const;  // not write to them
        
        list_iterator<T> begin();
        list_iterator<T> end();
        list_reverse_iterator<T> rbegin();
        list_reverse_iterator<T> rend();

		%extend // this is a extra bit of SWIG code
		{
			int __len() 
			{
				return (*self).size();
			}
		};
    };

}

/*
Vector<->LuaTable fns
These look a bit like the array<->LuaTable fns
but are templated, not %defined
(you must have template support for STL)

*/
/*
%{
// reads a table into a list of numbers
// lua numbers will be cast into the type required (rounding may occur)
// return 0 if non numbers found in the table
// returns new'ed ptr if ok
template<class T>
std::list<T>* SWIG_read_number_list(lua_State* L,int index)
{
	int i=0;
	std::list<T>* l=new std::list<T>();
	while(1)
	{
		lua_rawgeti(L,index,i+1);
		if (!lua_isnil(L,-1))
		{
			lua_pop(L,1);
			break;	// finished
		}
		if (!lua_isnumber(L,-1))
		{
			lua_pop(L,1);
			delete l;
			return 0;	// error
		}
		l->push_back((T)lua_tonumber(L,-1));
		lua_pop(L,1);
		++i;
	}
	return l;	// ok
}
// writes a vector of numbers out as a lua table
template<class T>
int SWIG_write_number_vector(lua_State* L,std::list<T> *l)
{
	lua_newtable(L);
	int index=1;
	for(std::list<T>::iterator li=l->begin();li!=l->end();++li)
	{
		lua_pushnumber(L,(double)*li);
		lua_rawseti(L,-2,index);// -1 is the number, -2 is the table
		index++;
	}
}
%}

// then the typemaps

%define SWIG_TYPEMAP_NUM_VECTOR(T)

// in
%typemap(in) std::vector<T> *INPUT
%{	$1 = SWIG_read_number_vector<T>(L,$input);
	if (!$1) SWIG_fail;%}

%typemap(freearg) std::vector<T> *INPUT
%{	delete $1;%}

// out
%typemap(argout) std::vector<T> *OUTPUT
%{	SWIG_write_number_vector(L,$1); SWIG_arg++; %}

%enddef
*/
