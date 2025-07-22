//
//  SimplePingHelper.m
//  PingTester
//
//  Created by Chris Hulbert on 18/01/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "SimplePingHelper.h"

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <err.h>

@interface SimplePingHelper()
@property(nonatomic,retain) SimplePing* simplePing;
@property(nonatomic,retain) id target;
@property(nonatomic,assign) SEL sel;
- (id)initWithAddress:(NSString*)address target:(id)_target sel:(SEL)_sel;
- (void)go;
@end

@implementation SimplePingHelper
@synthesize simplePing, target, sel;

#pragma mark - Run it

// Pings the address, and calls the selector when done. Selector must take a NSnumber which is a bool for success
+ (void)ping:(NSString*)address target:(id)target sel:(SEL)sel {
	// The helper retains itself through the timeout function
    
    struct addrinfo *servinfo,*res=NULL;
    struct addrinfo hints;
    
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int error = getaddrinfo([address UTF8String], NULL , &hints, &servinfo);
    res = servinfo;
    
    if(res==NULL)
    {
        NSLog(@"SimplePingHelper addrinfo = null");
        //ipv4
        [[[SimplePingHelper alloc] initWithAddress:address target:target sel:sel]  go];
    }
    else
    {
        NSLog(@"SimplePingHelper addrinfo = not null");
        void *addr;
        
        if (res->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
            addr = &(ipv4->sin_addr);
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
            addr = &(ipv6->sin6_addr);
        }

        char ipstr[INET6_ADDRSTRLEN];
        
        // convert the IP to a string and print it:
        inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
        
        [[[SimplePingHelper alloc] initWithAddress:[NSString stringWithCString:ipstr encoding:NSUTF8StringEncoding]
                                            target:target
                                               sel:sel]
         go];
    }

    freeaddrinfo(servinfo);
}

#pragma mark - Init/dealloc

- (void)dealloc {
	self.simplePing = nil;
	self.target = nil;
}

- (id)initWithAddress:(NSString*)address target:(id)_target sel:(SEL)_sel {
	if (self = [self init]) {
		self.simplePing = [SimplePing simplePingWithHostName:address];
		self.simplePing.delegate = self;
		self.target = _target;
		self.sel = _sel;
	}
	return self;
}

#pragma mark - Go

- (void)go {
	[self.simplePing start];
	[self performSelector:@selector(endTime) withObject:nil afterDelay:1]; // This timeout is what retains the ping helper
}

#pragma mark - Finishing and timing out

// Called on success or failure to clean up
- (void)killPing {
	[self.simplePing stop];
	self.simplePing = nil;
}

- (void)successPing {
	[self killPing];
    NSLog(@"SimplePingHelper successPing");
	[target performSelector:sel withObject:[NSNumber numberWithBool:YES]];
}

- (void)failPing:(NSString*)reason {
	[self killPing];
    NSLog(@"SimplePingHelper failPing reason = %@",reason);
	[target performSelector:sel withObject:[NSNumber numberWithBool:NO]];
}

// Called 5s after ping start, to check if it timed out
- (void)endTime {
	if (self.simplePing) { // If it hasn't already been killed, then it's timed out
		[self failPing:@"timeout"];
	}
}

#pragma mark - Pinger delegate

// When the pinger starts, send the ping immediately
- (void)simplePing:(SimplePing *)pinger didStartWithAddress:(NSData *)address {
	[self.simplePing sendPingWithData:nil];
}

- (void)simplePing:(SimplePing *)pinger didFailWithError:(NSError *)error {
    NSLog(@"didFailWithError failPing reason = %@",error);
	[self failPing:@"didFailWithError"];
}

- (void)simplePing:(SimplePing *)pinger didFailToSendPacket:(NSData *)packet sequenceNumber:(uint16_t)sequenceNumber error:(NSError *)error
{
    // Eg they're not connected to any network
    NSLog(@"didFailToSendPacket failPing reason = %@",error);
    [self failPing:@"didFailToSendPacket"];
}

- (void)simplePing:(SimplePing *)pinger didReceivePingResponsePacket:(NSData *)packet sequenceNumber:(uint16_t)sequenceNumber
{
 	[self successPing];
}


@end
