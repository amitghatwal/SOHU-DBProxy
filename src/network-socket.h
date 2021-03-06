/* $%BEGINLICENSE%$
 Copyright (c) 2007, 2011, Oracle and/or its affiliates. All rights reserved.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; version 2 of the
 License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 02110-1301  USA

 $%ENDLICENSE%$ */
 

#ifndef _NETWORK_SOCKET_H_
#define _NETWORK_SOCKET_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "network-exports.h"
#include "network-queue.h"

#ifdef HAVE_SYS_TIME_H
/**
 * event.h needs struct timeval and doesn't include sys/time.h itself
 */
#include <sys/time.h>
#endif

#include <sys/types.h>      /** u_char */
#ifndef _WIN32
#include <sys/socket.h>     /** struct sockaddr */

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>     /** struct sockaddr_in */
#endif
#include <netinet/tcp.h>

#ifdef HAVE_SYS_UN_H
#include <sys/un.h>         /** struct sockaddr_un */
#endif

/**
 * use closesocket() to close sockets to be compatible with win32
 */
#define closesocket(x) close(x)
#else
#include <winsock2.h>
#include <Ws2tcpip.h>
#endif
#include <glib.h>
#include <event.h>

#include "chassis-mainloop.h"
#include "network-address.h"
#include "chassis-regex.h"

typedef enum {
	NETWORK_SOCKET_SUCCESS,
	NETWORK_SOCKET_WAIT_FOR_EVENT,
	NETWORK_SOCKET_ERROR,
	NETWORK_SOCKET_ERROR_RETRY
} network_socket_retval_t;

typedef struct network_mysqld_auth_challenge network_mysqld_auth_challenge;
typedef struct network_mysqld_auth_response network_mysqld_auth_response;

typedef struct {
	int fd;             /**< socket-fd */
	struct event event; /**< events for this fd */

	network_address *src; /**< getsockname() */
	network_address *dst; /**< getpeername() */

	int socket_type; /**< SOCK_STREAM or SOCK_DGRAM for now */

	guint8   last_packet_id; /**< internal tracking of the packet_id's the automaticly set the next good packet-id */
	gboolean packet_id_is_reset; /**< internal tracking of the packet_id sequencing */

	network_queue *recv_queue;
	network_queue *recv_queue_raw;
	network_queue *send_queue;

	off_t header_read;
	off_t to_read;
	
	/**
	 * data extracted from the handshake  
	 */
	network_mysqld_auth_challenge *challenge;
	network_mysqld_auth_response  *response;

	gboolean is_authed;           /** did a client already authed this connection */

	/**
	 * store the default-db of the socket
	 *
	 * the client might have a different default-db than the server-side due to
	 * statement balancing
	 */	
	GString *default_db;     /** default-db of this side of the connection */

	/**
	 * added by jinxuan hou, 2013/04/10
	 * we should keep record the information of autocommit/charset/ip_range
	 * @@jinxuanhou
	 */
	gchar *ip_region; // for audit, count usage.
	struct ip_range *ip; // ip_range of this peer, will be use only for client
	guint8 charset; //charset
	guint8 autocommit;//whether autocommit is setted

	/**
	 * 以下几个变量用于字符集的恢复
	 * 但是只是先处理client、connection及results的字符集
	 */
	GString *character_set_client;
	GString *character_set_connection;
	GString *character_set_database;
	GString *character_set_results;
	GString *character_set_server;
	GString *collection_connect;

} network_socket;

/**
 * @brief ：
 * 	   
 *	   
 * ip_range moved here
 */
/**
 * added by jinxuan hou
 * A struct containing ip infomation
 * To avoid string comparison, we convert an IP address or a range of ips 
 * to two uint32.
 */
/*typedef struct ip_range{
        guint minip; //the lowwer num of an ip_range ip
        guint maxip; //the upper num of an ip_range ip
        GString *ip; //the string format of an ip range. such as X.X.%.%
} ip_range;
*/
// ip range related function
//ip_range *ip_range_new(void); //create a new ip_range variable
//void ip_range_free(ip_range *data);//release variable
//ip_range *create_ip_range_from_str(const gchar* ip);//get ip_range from ip string


NETWORK_API network_socket *network_socket_init(void) G_GNUC_DEPRECATED;
NETWORK_API network_socket *network_socket_new(void);
NETWORK_API void network_socket_free(network_socket *s);
NETWORK_API network_socket_retval_t network_socket_write(network_socket *con, int send_chunks);
NETWORK_API network_socket_retval_t network_socket_read(network_socket *con);
NETWORK_API network_socket_retval_t network_socket_to_read(network_socket *sock);
NETWORK_API network_socket_retval_t network_socket_set_non_blocking(network_socket *sock);
NETWORK_API network_socket_retval_t network_socket_connect(network_socket *con);
NETWORK_API network_socket_retval_t network_socket_connect_finish(network_socket *sock);
NETWORK_API network_socket_retval_t network_socket_bind(network_socket *con);
NETWORK_API network_socket *network_socket_accept(network_socket *srv);

gboolean detect_server_socket_disconnect(network_socket *sock);

#endif

