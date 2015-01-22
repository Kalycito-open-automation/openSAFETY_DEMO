Triple Buffer {#page_tbuf}
============

[TOC]

The triple buffer core manages the buffered data transfer between the PCP and
the user application. It is able to instantiate up to 32 buffers where each can
either be a producing or consuming buffer. The buffers are arranged in a way that
the consuming buffers (User application side of view) are always listed first
inside the memory. This enables concurrent access to all producing and consuming
buffers in a continuous stream which simplifies access via a serial bus interface.
The triple buffer core itself only provides a set of buffered memories with a
defined size. The detailed memory layout inside each buffer is not defined by
the IP-Core and is provided by the used software and it's modules.

> The addressing of all buffers is continuous and the offset and size of each
> buffer is forwarded via a header (tbuf-cfg.h) to the user application.

![Triple buffer core layout](triple_buffer_layout.png)
@image latex triple_buffer_layout.eps "Triple buffer core layout" width=0.9\textwidth

After all sizes of the producing and consuming triple buffers are adapted the
whole size of the input and output image is printed as an info message like it
is given in the following image.

![Triple buffer read write info print](tbuf_read_write_size.png)
@image latex tbuf_read_write_size.eps "Triple buffer read write info print" width=0.6\textwidth

> This size of the triple buffers are needed to be forwarded manually to the
> configuration of the SPI bridge IP-Core. For this the GUI of the bridge provides
> the option to enter the parameters **Read buffers size** and **Write buffers
> size**!

# Triple buffer core - GUI    {#sect_tbuf_gui}
The triple buffer component provides the following user options where PortA is
always the side of the user application:
- **Stream access on PortA:**: In case of a streamed access the core arranges all addresses for continuous access.
- **Number of consuming buffers:** Represents the count of incoming buffers from the side of PortA.
- **Number of producing buffers:** Represents the count of outgoing buffers from the side of PortA.
- **Size of each buffer**

# Buffer mechanism and synchronization    {#sect_tbuf_bufmech}
In order to exchange consistent data over the message buffers, they are implemented
as triple buffers. This means that each channel has three buffers of the same
size which are passed from the producer to the consumer and back. The buffer
allocation algorithm, implemented in hardware, ensures fast switching of the
buffers and enables an always available buffer for the producer. At the same
time the consumer is always able to get the latest data when reading from the
buffers.

The switching of the buffers is controlled with buffer acknowledge bits that
trigger the swapping of the buffers. The consuming acknowledge bits are always
at the lowest address in memory and the producing at the highest. The first bit
acknowledges always the first buffer in memory.

Further details about the switching of the buffers are given in the following
figure. There a loss free data exchange is pictured where each produced data
packet is also consumed. In case of a slow consumer the producer always has access
to the additional third buffer, which enables an always non-blocking write.

![Buffer allocation algorithm](buffer_alloc_algo.png)
@image latex buffer_alloc_algo.eps "Buffer allocation algorithm"

