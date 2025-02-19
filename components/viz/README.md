# //components/viz

Viz - short for visuals - is the client library and service implementations for
compositing and gpu presentation.

See [//services/viz](../../services/viz/README.md) for more information about
Viz overall.

**Table of Contents**
1. [Terminology](#terminology)
2. [Directory structure](#directory-structure)
    1. [common](#directory-structure-common)
    2. [client](#directory-structure-client)
    3. [host](#directory-structure-host)
    4. [service](#directory-structure-service)
3. [Naming guidelines with Mojo](#naming-guidelines)

## Terminology

**Mojo Interface**: The interface definition, found in a .mojom file. This is
an abstract interface and can be thought of as a process/thread-independent C++
abstract class.

**Mojo Implementation**: The default implementation of a mojom interface for
production. Many interfaces will only have a single implementation that we
ship.

**Alternate Mojo Implementations**: Secondary implementations of a mojom
interface for platform-specific details, for tests, etc.

**Service**: Where Mojo implementations live and run.

**Host**: A privileged process that provides access to viz Mojo interfaces for
Clients. Most services in Chrome don’t have these, and Clients find the service
directly, but they are more common in Viz. Currently the Host is in the browser
process. In a fully-realized mus+ash, the Host moves outside of Chrome to the
window server.

**Client**: All users of a Mojo interface who are not the Host. They will
usually need to go through the Host to gain access to the Mojo interface.

**Host-side Wrapper**: A C++ wrapper around a Mojo interface for use in Hosts,
that often also exposes or uses some privileged interfaces that Clients don’t
have. Generally prefer to use the Mojo interfaces directly, but sometimes we
need another C++ helper around it.

**Client-side Wrapper**: A C++ wrapper around a Mojo interface for use in
Clients. Generally prefer to use the Mojo interface directly, but sometimes we
need another C++ helper around it.

**Host/Client-side Abstraction**: A C++ wrapper around a Mojo interface that is
also a subclass of a C++ interface. Generally prefer to use the Mojo interfaces
directly, but sometimes we need a higher-level C++ abstraction, usually because
other subclasses cannot use the Mojo interface.


## Directory Structure <a name="directory-structure"></a>
To keep dependencies clear into the Viz source tree, all source code files
should appear in leaf directories.

### common <a name="directory-structure-common"></a>
Data types and simple helpers that are used by the client library, or by
clients directly, and by service implementations.

### client <a name="directory-structure-client"></a>
Client library for accessing Viz services. May be used from privileged (eg
browser) or unprivileged (eg renderer) processes.

| Can depend on: |
|:---------------|
| viz/common/    |

### host <a name="directory-structure-host"></a>
Privileged client library for owning and controlling Viz services. May only be
used from privileged (eg browser) processes.

This should not depend on other parts of Viz, as they are core data types and
helpers only, and can be used from anywhere else in Viz.

| Can depend on: |
|:---------------|
| viz/common/    |

### service <a name="directory-structure-service"></a>
Service-side implementations of Viz Mojo interfaces, which are found in
[//services/viz](https://cs.chromium.org/chromium/src/services/viz/). Each
component of the service-side implementation is located in its own
sub-directory.

As of this writing, these service components may be instantiated and used
directly from the browser process. But these services are intended to be
abstracted away through Mojo interfaces so that they are able to live entirely
outside the browser process, and gain in-process access to the Gpu.

#### service/display
**Display compositor**: The display compositor uses Gpu or software to composite
a set of frames, from multiple clients, into a single backing store for display
to the user. Also deals in getting screenshots of content by drawing to
offscreen buffers.

The top-level scheduler that coordinates when the compositor should draw, along
with when clients should be submitting frames.

This component is platform-agnostic, with any platform-specific details
abstracted away from it. It accesses Gpu services through the command buffer as
a client even though it is in the same process as the Gpu service in order to
be scheduled as a peer among other clients.

| Can depend on:        |
|:----------------------|
| viz/common/*          |
| viz/service/surfaces/ |

#### service/display_embedder
**Platform details for display compositor**: While the display compositor is
platform-agnostic, this component provides implementations of platform-specific
behaviour needed for the display compositor, and injected into it.

Code here supports presentation of the backing store drawn by the display
compositor (typically thought of as SwapBuffers), as well as the use of
overlays.

| Can depend on: |
|:---------------|
| viz/common/*   |

#### service/frame_sinks
**Frame sinks**: This component implements the Mojo interfaces to send frames,
resources, and other data types from ``viz/common/`` for display to the
compositing service. It receives and organizes relationships between what should
be composited.

| Can depend on:        |
|:----------------------|
| viz/common/*          |
| viz/service/display/  |
| viz/service/surfaces/ |

#### service/gl
**GL**: This component implements the Mojo interfaces for allocating (and
deallocating) gpu memory buffers, setting up a channel for the command buffer,
etc.

| Can depend on:        |
|:----------------------|
| viz/common/*          |

#### service/hit_test
**Hit testing**: Service-side code to resolve input events to individual
clients.

| Can depend on:        |
|:----------------------|
| viz/common/*          |
| viz/service/surfaces/ |

#### service/main
**Main**: TODO(fsamuel): This will hold implementation of the root interface
from which other service interfaces are accessed.

As the root of the service/ code structure, it instantiates and connects all
other parts of Viz.

| Can depend on: |
|:---------------|
| viz/common/*   |
| viz/service/*  |

#### service/surfaces
**Surfaces**: This component acts like the data model for the compositing service.
It holds data received from frame sinks, and provides access to them for the
display compositor.

| Can depend on: |
|:---------------|
| viz/common/*   |


## Naming guidelines with Mojo <a name="naming-guidelines"></a>

Viz makes extensive use of Mojo, and there are conflicting patterns with
regard to naming types around Mojo interfaces in the codebase today. This aims
to provide a standard to adhere to for future naming to increase our
consistency within the team.

For a given mojo service called `TimeTraveller`, we would use the following.

**Mojo Interface**: `mojom::TimeTraveller`
* This is the abstract interface definition. It comes with no prefix or suffix,
and lives in a (sometimes nested) mojom namespace.

**Mojo Implementation**: `TimeTravellerImpl`
* This is the implementation of the interface. For other C++ interfaces we
commonly use the Impl suffix, and we will repeat this here, as it’s already
commonly used and is well understood.
* If there will be multiple implementations, see below.

**Alternative Mojo Implementation**: `DescriptivePrefixTimeTravellerImpl`
* This is a non-default implementation of the interface. It follows the same
rules as a default implementation, but is used for tests, or cases where we
don’t have a simple default production implementation.
* The descriptive prefix should describe what makes this implementation
different from others, as such why it exists.

**Host-side Wrapper**: `HostTimeTraveller`
* This wraps the Mojo interface, providing a higher-level abstraction and
utilities for using the Mojo interface. It only is meant to exist and be
accessed in the privileged Host process. We prefix with Host to explain what
makes this interface special and when it can be used. We do not suffix to be
consistent with Clients.

**Client-side Wrapper**: `ClientTimeTraveller`
* This wraps the Mojo interface, providing a higher-level abstraction and
utilities for using the Mojo interface. It may exist and be used in any Client
in place of the Mojo interface. Prefer to use the Mojo interface directly when
possible. We prefix with Client to explain in what situations it can be used.
We do not suffix to avoid a TimeTravellerClientClient in the case where this
class has itself a C++ client, as ClientTimeTravellerClient appeared to be a
better option of the two.

**Host/Client-side Abstraction**: `OtherAbstractionName`
* This is a case of an object implementing a C++ interface, which it should be
named after. A prefix describing what makes this implementation special can
refer to its use of the Mojo interface.
