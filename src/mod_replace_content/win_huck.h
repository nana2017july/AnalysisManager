/*
* Copyright 2019 the original author or authors.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __WIN_HUCK_H__
#define __WIN_HUCK_H__

/**
@file
@n This file is created by me, for windows.
@n When compiling custom apache module on visual studio 2019, error has occured.
@n So for solving these errors this header file is created.
@n Compilation will be success, but probablly this is not normal way.
@n Please keep this in mind.
@n
@n My errors on compilation is like below.
@n  E0020: identifier "SOCKET" is undefined. apr_portable.h 64
@n  C2079: 'sin' uses undefined struct 'sockaddr_in'. apr_network_io.h 240
@n  C2079: 'sin6' uses undefined struct 'sockaddr_in6'. apr_network_io.h 243
@n  C2061: Syntax error: Identifier 'apr_os_sock_t'. apr_portable.h 64
@n
*/



#include <stdint.h>
#define APR_DECLARE_EXPORT
#define SOCKET int

/**
https://stackoverflow.com/questions/55153003/winsock-undefined-struct-sockaddr-in
*/
#include <ws2def.h>
//
typedef int8_t sa_family_t;
typedef uint16_t in_port_t;
struct in6_addr {
	unsigned char   s6_addr[16];   /* IPv6 address */
};
struct sockaddr_in6 {
	sa_family_t     sin6_family;   /* AF_INET6 */
	in_port_t       sin6_port;     /* port number */
	uint32_t        sin6_flowinfo; /* IPv6 flow information */
	struct in6_addr sin6_addr;     /* IPv6 address */
	uint32_t        sin6_scope_id; /* Scope ID (new in 2.4) */
};


#endif //__WIN_HUCK_H__
