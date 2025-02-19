/*
 *  Copyright (c) 2009-2015 Erik Doernenburg and contributors
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use these files except in compliance with the License. You may obtain
 *  a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

#import "OCMNotificationPoster.h"


@implementation OCMNotificationPoster

- (id)initWithNotification:(id)aNotification
{
    if ((self = [super init]))
    {
        notification = [aNotification retain];
    }
	
	return self;
}

- (void)dealloc
{
	[notification release];
	[super dealloc];
}

- (void)handleInvocation:(NSInvocation *)anInvocation
{
	[[NSNotificationCenter defaultCenter] postNotification:notification];
}


@end
