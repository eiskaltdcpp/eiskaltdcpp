#ifndef DCPLUSPLUS_DCPP_FLAGS_H_
#define DCPLUSPLUS_DCPP_FLAGS_H_

namespace dcpp {

class Flags {
public:
	typedef int MaskType;

	Flags() : flags(0) { }
	Flags(const Flags& rhs) : flags(rhs.flags) { }
	Flags(MaskType f) : flags(f) { }
	bool isSet(MaskType aFlag) const { return (flags & aFlag) == aFlag; }
	bool isAnySet(MaskType aFlag) const { return (flags & aFlag) != 0; }
	void setFlag(MaskType aFlag) { flags |= aFlag; }
	void unsetFlag(MaskType aFlag) { flags &= ~aFlag; }
	MaskType getFlags() const { return flags; }
	Flags& operator=(const Flags& rhs) { flags = rhs.flags; return *this; }
protected:
	~Flags() { }
private:
	MaskType flags;
};

} // namespace dcpp

#endif /*FLAGS_H_*/
