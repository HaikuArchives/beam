/*************************************************************************/
/*                                                                       */
/*  Regexx - Regular Expressions C++ solution.                           */
/*                                                                       */
/*  http://projects.nn.com.br/                                           */
/*                                                                       */
/*  Copyright (C) 2000 Gustavo Niemeyer <gustavo@nn.com.br>              */
/*                                                                       */
/*  This library is free software; you can redistribute it and/or        */
/*  modify it under the terms of the GNU Library General Public          */
/*  License as published by the Free Software Foundation; either         */
/*  version 2 of the License, or (at your option) any later version.     */
/*                                                                       */
/*  This library is distributed in the hope that it will be useful,      */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    */
/*  Library General Public License for more details.                     */
/*                                                                       */
/*  You should have received a copy of the GNU Library General Public    */
/*  License along with this library; if not, write to the                */
/*  Free Software Foundation, Inc., 59 Temple Place - Suite 330,         */
/*  Boston, MA  02111-1307, USA.                                         */
/*                                                                       */
/*************************************************************************/

// $Revision$
// $Date$

#ifndef REGEXX_HH
#define REGEXX_HH

#include <String.h>
#include <vector>
#include "split.hh"
#include "pcre.h"

namespace regexx {

  /** Class to store atoms.
   *
   *  @author Gustavo Niemeyer
   *
   */
  class RegexxMatchAtom
  {

  public:

    inline
    RegexxMatchAtom(BString& _str,
		    int32 _start,
		    int32 _length)
      : m_str(_str), m_start(_start), m_length(_length)
    {}

    inline RegexxMatchAtom&
    operator=(const RegexxMatchAtom& _rxxma)
    {
      m_str = _rxxma.m_str;
      m_start = _rxxma.m_start;
      m_length = _rxxma.m_length;
      return *this;
    }

    /// Retrieves the atom string.
    inline BString
    str() const
    { BString temp; return m_str.CopyInto(temp, m_start, m_length); }

    /// Returns the position in the original string where the atom starts.
    inline const int32&
    start() const
    { return m_start; }

    /// Length of the atom string.
    inline const int32&
    Length() const
    { return m_length; }

    /// Operator to transform a RegexxMatchAtom into a string.
    inline operator
    BString() const
    { BString temp; return m_str.CopyInto(temp, m_start, m_length); }

    /// Operator to compare a RegexxMatchAtom with a string.
    inline bool
    operator==(const BString& _s) const
    { BString temp; return m_str.CopyInto(temp, m_start, m_length) == _s; }

  private:

    BString &m_str;
    int32 m_start;
    int32 m_length;

  };

  inline ostream& operator<<(ostream& _o, RegexxMatchAtom& _rxxma)
  {
    return _o << _rxxma.str().String();
  }

  /** Class to store matches.
   *
   *  @author Gustavo Niemeyer
   *
   */
  class RegexxMatch
  {

  public:

    inline
    RegexxMatch(BString& _str,
		int32 _start,
		int32 _length)
      : m_str(_str), m_start(_start), m_length(_length)
    {}

    inline RegexxMatch&
    operator=(const RegexxMatch& _rxxm)
    {
      m_str = _rxxm.m_str;
      m_start = _rxxm.m_start;
      m_length = _rxxm.m_length;
      return *this;
    }

    /// Retrieves the match string.
    inline BString
    str() const
    { BString temp; return m_str.CopyInto(temp, m_start, m_length); }

    /// Returns the position in the original string where the match starts.
    inline const int32&
    start() const
    { return m_start; }

    /// Length of the match string.
    inline const int32&
    Length() const
    { return m_length; }

    /// Operator to transform a RegexxMatch into a string.
    inline operator
    BString() const
    { BString temp; return m_str.CopyInto(temp, m_start, m_length); }

    /// Operator to compare a RegexxMatch with a string.
    inline bool
    operator==(const BString& _s) const
    { BString temp; return m_str.CopyInto(temp, m_start, m_length) == _s; }

    /// Vector of atoms found in this match.
    std::vector<RegexxMatchAtom> atom;

  private:

    BString &m_str;
    int32 m_start;
    int32 m_length;

  };

  inline ostream& operator<<(ostream& _o, RegexxMatch& _rxxm)
  {
    return (_o << _rxxm.str().String());
  }

  /** The main Regexx class.
   *
   * Regexx is a complete regular expressions C++ solution.
   * It implements easy expression execution, global searching,
   * replace with atom substitution, easy match and atom strings
   * retrieving. It's also included in the package functions to
   * split strings with strings or regular expressions.
   *
   * @author Gustavo Niemeyer
   *
   **/
  class Regexx
  {

  public:

    /// These are the flags you can use with exec() and replace().
    enum flags {

      /// Global searching, otherwise Regexx stops after the first match.
      global  = 1,

      /// Ignore case.
      nocase  = 2,

      /** Do not store matches. If you don't want to retrieve the matched
       *  strings and atoms, use this. It's a lot faster. Note that you
       *  can retrieve the number of matches with matches(), operator int()
       *  or the return value of exec(). Ignored if used
       *  with replace().
       */
      nomatch = 4,

      /** Do not store atoms. If you don't want to retrieve the matched
       *  atoms, use this. It's a faster. If you are using nomatch, you
       *  don't need this.
       */
      noatom  = 8,

      /** If you're going to use the same regular expression several times
       *  or if you're going to apply it in a big text, use this flag.
       **/
      study   = 16,

      /** Match-any-character operators don't match the newline. A non 
       *  matching list ([^...]) not containing a newline does not match
       *  a newline. Match-beggining-of-line operator (^) matches the empty
       *  string immediately after a newline, regardless of whether
       *  Regexx::notbol is set or not. Match-end-of-line operator ($)
       *  matches the empty string immediately before a newline, regardless
       *  of wheter Regexx::noteol is set or not.
       **/
      newline = 32,

      /** The match-beginig-of-line operator always fails to match (see
       *  Regexx::newline). This flag can be used when different portions of
       *  a string are passed to Regexx and the beggining of the string
       *  should not be interpreted as the beggining of the line.
       **/
      notbol  = 64,

      /// The match-end-of-line operator always fails to match.
      noteol  = 128

    };

    /** Exceptions base class.
     *
     * This is the base class used for exceptions. If you catch this
     * one, you're catching any Exception that can be thrown by
     * Regexx.
     *
     *
     *  @author Gustavo Niemeyer
     *
     **/
    class Exception {
    public:
      Exception(const BString& _message) : m_message(_message) {}
      /// Method to retrieve Exception information.
      inline const BString& message() { return m_message; }
    private:
      BString m_message;
    };

    /** This exception is thrown when there are errors while compiling
     *  expressions.
     */
    class CompileException : public Exception {
    public:
      CompileException(const BString& _message) : Exception(_message) {}
    };

    /// Constructor
    inline
    Regexx()
      : m_compiled(false), m_study(false), m_matches(0), m_extra(NULL)
    {}

    /// Destructor
    inline
    ~Regexx()
    { if(m_compiled) { free(m_preg); if(m_study) free(m_extra); } }

    /** Constructor with regular expression execution.
     *
     *  This constructor allows you to run one-line regular expressions.
     *  Since there is a operator int(), you can use it expecting a
     *  return value or inside a boolean context.
     *
     *  @see operator int()
     */
    inline
    Regexx(const BString& _str, const BString& _expr, int _flags = 0)
      throw(CompileException)
      : m_compiled(false), m_study(false), m_matches(0), m_extra(NULL)
    { exec(_str,_expr,_flags); }

    /** Constructor with regular expression string replacing.
     *
     *  This constructor allows you to run one-line regular expression
     *  string replacing. Since there is a operator string(), you can use
     *  it expecting a return value or inside a boolean context.
     *
     *  @see operator string()
     */
    inline
    Regexx(const BString& _str, const BString& _expr, 
	   const BString& _repstr, int _flags = 0)
      throw(CompileException)
      : m_compiled(false), m_study(false), m_matches(0), m_extra(NULL)
    { replace(_str,_expr,_repstr,_flags); }
    
    /** Set the regular expression to use with exec() and replace().
     * 
     *  Use this function only if you are going to use the exec() and/or
     *  replace functions that don't provide the regular expression.
     *
     *  @return Self reference.
     */
    inline Regexx&
    expr(const BString& _expr);

    /// Retrieve the current regular expression.
    inline const BString&
    expr() const
    { return m_expr; }

    /** Set the string to use with exec() and replace().
     * 
     *  Use this function only if you are going to use the exec() and/or
     *  replace functions that don't provide the string.
     *
     *  @return Self reference.
     */
    inline Regexx&
    str(const BString& _str);

    /// Retrieve the current string.
    inline const BString&
    str() const
    { return m_str; }

    /** Execute a regular expression.
     *
     *  To use this function you have to store the string and regular
     *  expression with the str() and expr() functions before. It also works
     *  if you want to reexecute a regular expression that you've
     *  executed before.
     *
     *  @return Number of matches.
     */
    const unsigned int&
    exec(int _flags = 0)
      throw(CompileException);

    /** Execute a regular expression.
     *
     *  To use this function you have to store the string with the str() 
     *  function before. It also works if you want to reexecute a regular
     *  expression that you've executed before.
     *
     *  @return Number of matches.
     */
    inline const unsigned int&
    exec(const BString& _expr, int _flags = 0)
      throw(CompileException);

    /** Execute a regular expression.
     *  @return Number of matches.
     */
    inline const unsigned int&
    exec(const BString& _str, const BString& _expr, int _flags = 0)
      throw(CompileException);

    /** Replace string with regular expression.
     *
     *  To use this function you have to store the string and regular
     *  expression with the str() and expr() functions before. It also works
     *  if you want to reexecute a regular expression that you've
     *  executed before.
     *
     *  @param _repstr Replace string. You can reference atoms in this
     *                 string with '$1'-'$9'. If you want to include a '$'
     *                 character you have to escape it with a '\'.
     *  @return Replaced string.
     */
    const BString&
    replace(const BString& _repstr, int _flags = 0)
      throw(CompileException);

    /** Replace string with regular expression.
     *
     *  To use this function you have to store the string with the str() 
     *  function before. It also works if you want to reexecute a regular
     *  expression that you've executed before.
     *
     *  @param _repstr Replace string. You can reference atoms in this
     *                 string with '$1'-'$9'. If you want to include a '$'
     *                 character you have to escape it with another '\'.
     *  @return Replaced string.
     */
    inline const BString&
    replace(const BString& _expr, const BString& _repstr, int _flags = 0)
      throw(CompileException);

    /** Replace string with regular expression.
     *  @return Replaced string.
     */
    inline const BString&
    replace(const BString& _str, const BString& _expr, 
	    const BString& _repstr, int _flags = 0)
      throw(CompileException);

    /** Customized replace string with regular expression.
     *
     *  The first parameter to this function is a function/class with
     *  a RegexxMatch as the only parameter. This function/class will be
     *  executed for each match of the regular expression and the match
     *  will be replaced by the string returned by the function/class.
     *
     *  To use this function you have to store the string and regular
     *  expression with the str() and expr() functions before. It also works
     *  if you want to reexecute a regular expression that you've
     *  executed before.
     *
     *  @param _func Function/Class to be executed for each match.
     *  @return Replaced string.
     */
    template<class Function>
    inline const BString&
    replacef(Function _func, int _flags = 0)
      throw(CompileException);

    /** Customized replace string with regular expression.
     *
     *  The first parameter to this function is a function/class with
     *  a RegexxMatch as the only parameter. This function/class will be
     *  executed for each match of the regular expression and the match
     *  will be replaced by the string returned by the function/class.
     *
     *  To use this function you have to store the string with the str() 
     *  function before. It also works if you want to reexecute a regular
     *  expression that you've executed before.
     *
     *  @param _func Function/Class to be executed for each match.
     *  @return Replaced string.
     */
    template<class Function>
    inline const BString&
    replacef(const BString& _expr, Function _func, int _flags = 0)
      throw(CompileException);

    /** Customized replace string with regular expression.
     *
     *  The first parameter to this function is a function/class with
     *  a RegexxMatch as the only parameter. This function/class will be
     *  executed for each match of the regular expression and the match
     *  will be replaced by the string returned by the function/class.
     *
     *  @param _func Function/Class to be executed for each match.
     *  @return Replaced string.
     */
    template<class Function>
    inline const BString&
    replacef(const BString& _str, const BString& _expr, Function _func,
	    int _flags = 0)
      throw(CompileException);
    
    /** Returns the number of matches of the last exec()/replace()/replacef().
     *
     *  It works even if you use the nomatch flag.
     *
     */
    inline const unsigned int&
    matches() const
    { return m_matches; }

    /** Returns the number of matches of the last exec()/replace()/replacef().
     *
     *  It works even if you use the nomatch flag.
     *
     */
    inline operator
    unsigned int() const
    { return m_matches; }

    /// Returns the string of the last replace() or replacef().
    inline const BString&
    replaced() const
    { return m_replaced; }

    /// Returns the string of the last replace() or replacef().
    inline operator
    BString() const
    { return m_replaced; }

    /** The vector of matches.
     *
     *  Unless you set the nomatch flag, every found match is stored in
     *  this vector.
     *
     */
    std::vector<RegexxMatch> match;

  private:
    
    bool m_compiled;
    bool m_study;
    BString m_expr;
    BString m_str;
    int m_capturecount;
    
    unsigned int m_matches;
    BString m_replaced;

    pcre* m_preg;
    pcre_extra* m_extra;

  };

  inline Regexx&
  Regexx::expr(const BString& _expr)
  {
    if(m_compiled) {
      free(m_preg);
      m_compiled = false;
      if(m_study) {
	free(m_extra);
	m_study = false;
	m_extra = NULL;
      }
    }
    m_expr = _expr;
    return *this;
  }
  
  inline Regexx&
  Regexx::str(const BString& _str)
  {
    m_str = _str;
    return *this;
  }
  
  inline const unsigned int&
  Regexx::exec(const BString& _expr, int _flags)
    throw(CompileException)
  {
    expr(_expr);
    return exec(_flags);
  }
  
  inline const unsigned int&
  Regexx::exec(const BString& _str, const BString& _expr, int _flags)
    throw(CompileException)
  {
    str(_str);
    expr(_expr);
    return exec(_flags);
  }


  inline const BString&
  Regexx::replace(const BString& _expr, const BString& _repstr, int _flags)
    throw(CompileException)
  {
    expr(_expr);
    return replace(_repstr,_flags);
  }

  inline const BString&
  Regexx::replace(const BString& _str, const BString& _expr, 
		  const BString& _repstr, int _flags)
    throw(CompileException)
  {
    str(_str);
    expr(_expr);
    return replace(_repstr,_flags);
  }

  template<class Function>
  inline const BString&
  Regexx::replacef(Function _func, int _flags = 0)
    throw(CompileException)
  {
    exec(_flags&~nomatch);
    m_replaced = m_str;
    std::vector<RegexxMatch>::reverse_iterator m;
    for(m = match.rbegin(); m != match.rend(); m++) {
    	BString temp = _func(*m);
      m_replaced.Remove( m->start(), m->Length());
      m_replaced.Insert( temp, m->start());
    }
    return m_replaced;
  }

  template<class Function>
  inline const BString&
  Regexx::replacef(const BString& _expr, Function _func, int _flags = 0)
    throw(CompileException)
  {
    expr(_expr);
    return replacef(_func,_flags);
  }

  template<class Function>
  inline const BString&
  Regexx::replacef(const BString& _str, const BString& _expr, Function _func,
		  int _flags = 0)
    throw(CompileException)
  {
    str(_str);
    expr(_expr);
    return replacef(_func,_flags);
  }

}

#endif // REGEXX_HH