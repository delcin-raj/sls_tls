/*
 * =====================================================================================
 *
 *       Filename:  https.c
 *
 *    Description: Implementation of simple http client 
 *
 *        Version:  1.0
 *        Created:  06/22/2022 09:41:11 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  Bitsmaximus
 *
 * =====================================================================================
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX_GET_COMMAND 255
#define BUFFER_SIZE 255

/* *
 * uri is supposed to be a well formed url like http://example.com/page.html
 * Input:
 * 	uri
 * Output:
 * 	host
 * 	path
 * Error:
 * 	-1 when the uri is not well formed
 * 	0 if everything is correct
 * */

int parse_url( char *uri, char **host, char **path) {
	char *pos;

	pos = strstr(uri, "//");
	// now pos will point to the start of the string "//" in uri if it
	// exists else NULL
	
	if ( pos == NULL ) {
		return -1;
	}

	*host = pos + 2;
	// Now here is where the understanding of c pointers are crucial
	// Note that "//" is 2 in length so pos + 2 will point to char e in the
	// example url. So if you do printf("%s",*host); you'll get 
	// example.com/page.html as the output

	pos = strchr( *host, '/');
	// Now pos will point to char '/' in the string *host which separates
	// com and page if it exists else NULL
	
	if ( pos == NULL ) {
		*path = NULL; 
		// example url for this case is http://xyz.com
	} else {
		*pos = '\0';
		// So that printf("%s",*host); will result in example.com
		*path = pos + 1;
	}
	return 0;
}

int http_get( int connection, const char *path, const char *host ) {
	static char get_command[ MAX_GET_COMMAND ];
	
	sprintf( get_command, "GET /%s HTTP/1.1\r\n", path );
	if ( send( connection, get_command, strlen( get_command ), 0 ) == -1 ) {
		return -1;
	}
	sprintf( get_command, "Host: %s\r\n", host );
	if ( send( connection, get_command, strlen( get_command ), 0 ) == -1 ) {
		return -1;
	}
	sprintf( get_command, "Connection: close\r\n\r\n" );
	if ( send( connection, get_command, strlen( get_command ), 0 ) == -1 ) {
		return -1;
	}

	return 0;
}

void display_result( int connection ) {
	int received = 0;
	static char recv_buf[ BUFFER_SIZE ];

	while ( ( received = recv( connection, recv_buf, BUFFER_SIZE, 0 ) > 0 ) ) {
		recv_buf[ received ] = '\0';
		printf( "%s", recv_buf );
	}
	printf( "\n" );
}

int main( int argc, char *argv[] ) {
	int client_connection;
	char *host, *path;
	struct hostent *host_name;
	struct sockaddr_in host_address;

	if ( argc < 2 ) {
		fprintf( stderr, "Usage: %s: <URL>\n", argv[0]);
		return 1;
	}

	if ( parse_url( argv[1], &host, &path ) == -1 ) {
		fprintf( stderr, "Error - malformed url\n");
		return 1;
	}

	printf( "Connecting to host '%s'\n", host);

	// Step 1: Open a socket connection on http port with the destination

	client_connection = socket( AF_INET, SOCK_STREAM, 0);
	// SOCK_STREAM with 0 usually implies TCP connection
	// The value of client_connection is an integer which represents a file
	// descriptor (refer os implementations)
	
	if ( client_connection <= 0 ) {
		perror( "Unable to create local socket\n");
		return 2;
	}

	host_name = gethostbyname(host);

	if ( host_name == NULL ) {
		perror( " Error in name resolution i.e url does not exist\n");
		return 3;
	}
	 host_address.sin_family = AF_INET;
	 host_address.sin_port = htons( 80 );
	 memcpy( &host_address.sin_addr, host_name->h_addr_list[0],
			 sizeof( struct in_addr ) );

	 if ( connect( client_connection, (struct sockaddr *) &host_address,
				sizeof( host_address ) ) == -1 ) {
 		perror( " Unable to connect to host address\n");
		return 4;		
	 } 
	 // Socket connection is established. Now we can interact with server
	 printf( "Retrieving document: '%s'\n", path);
	 http_get( client_connection, path, host );

	 display_result( client_connection );
	 printf( "Shutting Down\n" );

	 if ( close( client_connection ) ) {
		 perror( "Error closing connection\n");
		 return 5;
	 }
	 return 0;
}
