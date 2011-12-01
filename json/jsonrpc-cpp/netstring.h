/*
 *  JsonRpc-Cpp - JSON-RPC implementation.
 *  Copyright (C) 2008-2011 Sebastien Vincent <sebastien.vincent@cppextrem.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file netstring.h
 * \brief NetString implementation (http://cr.yp.to/proto/netstrings.txt).
 * \author Sebastien Vincent
 */

#ifndef NETSTRING_H
#define NETSTRING_H 

#include <string>

/**
 * \namespace netstring
 * \brief Netstring related functions.
 * \see http://cr.yp.to/proto/netstrings.txt
 */
namespace netstring
{
  /**
   * \class NetstringException
   * \brief Netstring related exception.
   */
  class NetstringException : public std::exception
  {
    public:
      /**
       * \brief Constructor.
       * \param msg error message
       */
      NetstringException(const std::string& msg = "") throw();

      /**
       * \brief Destructor.
       */
      virtual ~NetstringException() throw();

      /**
       * \brief Get the exception message.
       * \return message C-string
       */
      virtual const char* what() const throw();

    private:
      /**
       * Exception message.
       */
      std::string m_msg;
  };

  /**
   * \brief Encode a string into netstring.
   * \param str string to encode
   * \return encoded netstring
   */
  std::string encode(const std::string& str);

  /**
   * \brief Decode a netstring into string.
   * \param netstr netstring
   * \return decoded string
   * \throw NetstringException if netstr is not a valid netstring
   */
  std::string decode(const std::string& netstr) throw(netstring::NetstringException);

} /* namespace netstring */

#endif /* NETSTRING_H */

