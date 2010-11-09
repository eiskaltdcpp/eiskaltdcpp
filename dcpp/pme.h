
////////////////////////////////////////////////////////////////////////////////////////////////
//
// This file is redistributable on the same terms as the PCRE 4.3 license, except copyright
// notice must be attributed to:
//
// (C) 2003 Zachary Hansen xaxxon@slackworks.com
//
// Distribution under the GPL or LGPL overrides any other restrictions, as in the PCRE license
//
////////////////////////////////////////////////////////////////////////////////////////////////
//
// Added GetStartPos
// Added GetLength
// Added Init
// Added lazy assignment operator
// Modified sub to not use iostreams.
// Added wstring versions of most functions just to avoid having to do the conversion
// everywhere they're used.
// Trem 2004


#ifndef PME_H
#define PME_H

#include "extra/regex/pcre.h"



/// PME wraps the PCRE C API into a more perl-like syntax
/**
 * PME wraps the PCRE C API into a more perl-liek syntax.  It supports single matching,
 *   global matching (where the regex resumes where it left off on the previous iteration),
 *   single and global substitutions (including using backrefs in the substituted strings),
 *   splits based on regex comparisons, and a syntactically easy way to get substrings out f
 *   from backrefs and splits.
 */
typedef std::vector < std::string > StringVector;
typedef std::vector < std::wstring > WStringVector;
class PME
{
public:

	/// default constructor -- virtually worthless
	PME( );

	/// s is the regular expression, opts are PCRE flags bit-wise or'd together
	PME(const std::string & s, unsigned opts );
	PME(const std::wstring & s, unsigned opts );

	/// s is the regular expression, opts is a perl-like string of modifier letters "gi" is global and case insensitive
	PME(const std::string & s, const std::string & opts = "");
	PME(const std::wstring & s, const std::wstring & opts = L"");

	/// s is the regular expression, opts are PCRE flags bit-wise or'd together
	PME(const char * s, unsigned opts );
	PME(const wchar_t * s, unsigned opts );

	/// s is the regular expression, opts is a perl-like string of modifier letters "gi" is global and case insensitive  
	PME ( const char * s, const std::string & opts = "" );
	PME ( const wchar_t *s, const std::wstring & opts = L"" );

	/// PME copy constructor
	PME(const PME & r);

	/// PME = PME assignment operator, lazy asignment operator, it doesn't copy state just the options.
	const PME & operator = (const PME & r);

	/// destructor
	~PME();

	/// stores results from matches
	typedef std::pair<int, int> markers;
	
	/// returns options set on this object
	unsigned				options();

	/// sets new options on the object -- doesn't work?
	void					options(unsigned opts);

	/// runs a match on s against the regex 'this' was created with -- returns the number of matches found
	int         			match(const std::string & s, unsigned offset = 0);
	int						match(const std::wstring & s, unsigned offset = 0);

	/// splits s based on delimiters matching the regex.
	int         			split(const std::string & s, ///< string to split on
								  unsigned maxfields = 0 ///< maximum number of fields to be split out.  0 means split all fields, but discard any trailing empty bits.  Negative means split all fields and keep trailing empty bits.  Positive means keep up to N fields including any empty fields less than N.  Anything remaining is in the last field.
	);
	int						split(const std::wstring &s, ///< string to split on
								  unsigned maxfields = 0 ///< maximum number of fields to be split out.  0 means split all fields, but discard any trailing empty bits.  Negative means split all fields and keep trailing empty bits.  Positive means keep up to N fields including any empty fields less than N.  Anything remaining is in the last field.
	);

	/// substitutes out whatever matches the regex for the second paramter
	std::string             sub ( const std::string & s, 
								  const std::string & r
								);
	std::wstring			sub ( const std::wstring & s,
								  const std::wstring & r
								);

	/// study the regular expression to make it faster
	void                    study();

	/// returns the substring from the internal m_marks vector requires having run match or split first
	std::string             operator[](int);
	
	/// resets the regex object -- mostly useful for global matching
	void                    reset();

	/// returns the number of back references returned by the last match/sub call
	int                     NumBackRefs ( ) { return nMatches; }

	/// returns the start position of the specified back reference 
	int						GetStartPos( int _backRef );

	/// return the length of the specified back reference
	int						GetLength( int _backRef );

	/// s is the regular expression, opts are PCRE flags bit-wise or'd together
	void					Init( const std::string & s, unsigned opts );
	void					Init( const std::wstring & s, unsigned opts );

	/// s is the regular expression, opts is a perl-like string of modifier letters "gi" is global and case insensitive
	void					Init( const std::string & s, const std::string & opts = "" );
	void					Init( const std::wstring & s, const std::wstring & opts = L"" );

	/// whether this regex is valid
	int IsValid ( ) { return nValid; }

	/// returns a vector of strings for the last match/split/sub
	StringVector  GetStringVector  ( );
	WStringVector GetStringVectorW ( );


protected:

	/// used internally for operator[]
	/** \deprecated going away */
	std::string		       	substr(const std::string & s,
								const std::vector< markers > & marks, unsigned index);


	pcre * re; ///< pcre structure from pcre_compile

	unsigned _opts; ///< bit-flag options for pcre_compile

	pcre_extra * extra;	///< results from pcre_study

	int nMatches; ///< number of matches returned from last pcre_exec call

	std::vector<markers> m_marks; ///< last set of indexes of matches

	std::string laststringmatched; ///< copy of the last string matched
	void * addressoflaststring; ///< used for checking for change of string in global match

	int m_isglobal; ///< non-pcre flag for 'g' behaviour
	int lastglobalposition; ///< end of last match when m_isglobal != 0
	
	/// compiles the regex -- automatically called on construction
	void compile(const std::string & s);

	/// used to make a copy of a regex object
	static pcre * clone_re(pcre * re);

	/// takes perl-style character modifiers and determines the corresponding PCRE flags
	unsigned int DeterminePcreOptions ( const std::string & opts = "" );
	
	/// flag as to whether this regex is valid (compiled without error)
	int                     nValid;

};


#endif // PME_H
