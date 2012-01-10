// for compilers that don't support nullptr, use the workaround in section 1.1 of the proposal.

#ifdef __GNUC__
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 6) // GCC 4.6 is the first GCC to implement nullptr.

const // this is a const object...
class {
public:
	template<class T> // convertible to any type
	operator T*() const // of null non-member
	{ return 0; } // pointer...
	template<class C, class T> // or any type of null
	operator T C::*() const // member pointer...
	{ return 0; }
private:
	void operator&() const; // whose address can't be taken
} nullptr = {}; // and whose name is nullptr

#endif
#endif
