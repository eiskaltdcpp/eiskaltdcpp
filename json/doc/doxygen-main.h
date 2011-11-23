/**
 * \file doxygen-main.h
 * \brief Documentation main page.
 * \author Sebastien Vincent
 * \date 2008-2009
 */

/**
 * \mainpage JsonRpc-Cpp Documentation
 *
 * \section section-intro Introduction
 *
 * This is the API of JsonRpc-Cpp, an OpenSource implementation of the JSON-RPC protocol (specification 2.0).
 *
 * JSON-RPC is a lightweight remote procedure call protocol similar to XML-RPC.
 * 
 * \section section-features Features
 * 
 * <ul>
 * <li> RPC handler (server-side) in JSON format;</li>
 * <li> Network server implementation;</li>
 * <li> Network client implementation.</li>
 * </ul>
 *
 * JsonRpc-Cpp uses <a href="http://jsoncpp.sourceforge.net">JsonCpp</a> library to encode/decode JSON message.
 * 
 * JsonRpc-Cpp provides a JSON-RPC class called Json::Rpc::Handler that contains RPC methods, check incoming JSON-RPC message,
 * reply either with success or error code (in case parsing failed, ...).
 *
 * JsonRpc-Cpp provides also simple JSON-RPC TCP and UDP server implementations (Json::Rpc::TcpServer and Json::Rpc::UdpServer) as well
 * as simple JSON-RPC TCP and UDP client implementations (Json::Rpc::TcpClient and Json::Rpc::UdpClient).
 *
 * \section section-build Build instructions
 * 
 * To build and install JsonRpc-Cpp, see INSTALL file located in the source tree.
 *
 * \section section-standard Standards
 *
 * JsonRpc-Cpp is written in C++ language which respects the following standard:
 * - ISO/CEI 14882:1998 (C++98).
 *
 * JsonRpc-Cpp is known to run on the following systems:
 * - GNU/Linux 2.6;
 * - Mac OS X;
 * - Microsoft Windows XP/Vista with MinGW compiler.
 * 
 * \section section-license License
 * 
 * JsonRpc-Cpp is licensed under the <a href="http://www.gnu.org/licenses/lgpl-3.0.html">LGPL version 3</a>.
 * 
 * \section section-links Links
 * 
 * <ul>
 * <li><a href="http://groups.google.com/group/json-rpc/web/json-rpc-1-2-proposal">JSON-RPC 2.0 specification</a>;</li>
 * <li><a href="http://jsonrpc-cpp.sourceforge.net">JsonRpc-Cpp website</a>;</li>
 * <li><a href="http://www.sourceforge.net/projects/jsonrpc-cpp">JsonRpc-Cpp project page</a>;</li>
 * <li><a href="http://jsoncpp.sourceforge.net">JsonCpp website</a>.</li>
 * </ul>
 * 
 */

