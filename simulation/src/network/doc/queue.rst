Queues
------

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

This section documents a few queue objects, typically associated with
NetDevice models, that are maintained as part of the ``network`` module:

* DropTail
* Random Early Detection 

Model Description
*****************

The source code for the new module lives in the directory ``src/network/utils``.

ns-3 provides a couple of classic queue models and the ability to
trace certain queue operations such as enqueuing, dequeuing, and dropping.
These may be added to certain NetDevice objects that take a Ptr<Queue>
pointer.

Note that not all device models use these queue models.  
In particular, WiFi, WiMax, and LTE use specialized device queues.
The queue models described here are more often used with simpler ns-3 
device models such as PointToPoint and Csma.

Design
======

An abstract base class, class Queue, is typically used and subclassed
for specific scheduling and drop policies.  Common operations
include:

* ``bool Enqueue (Ptr<Packet> p)``:  Enqueue a packet
* ``Ptr<Packet> Dequeue (void)``:  Dequeue a packet
* ``uint32_t GetNPackets (void)``:  Get the queue depth, in packets
* ``uint32_t GetNBytes (void)``:  Get the queue depth, in packets

as well as tracking some statistics on queue operations.

There are three trace sources that may be hooked:

* ``Enqueue``
* ``Dequeue``
* ``Drop``

DropTail
########

This is a basic first-in-first-out (FIFO) queue that performs a tail drop
when the queue is full.

Random Early Detection
######################

Random Early Detection (RED) is a queue variant that aims to provide
early signals to transport protocol congestion control (e.g. TCP) that
congestion is imminent, so that they back off their rate gracefully
rather than with a bunch of tail-drop losses (possibly incurring
TCP timeout).  The model in ns-3 is a port of Sally Floyd's ns-2
RED model.

Scope and Limitations
=====================

The RED model just supports default RED.  Adaptive RED is not supported.

References
==========

The RED queue aims to be close to the results cited in:
S.Floyd, K.Fall http://icir.org/floyd/papers/redsims.ps

Usage
*****

Helpers
=======

A typical usage pattern is to create a device helper and to configure
the queue type and attributes from the helper, such as this example
from ``src/network/examples/red-tests.cc``:

:: 

  PointToPointHelper p2p;

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer devn0n2 = p2p.Install (n0n2);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("3ms"));
  NetDeviceContainer devn1n2 = p2p.Install (n1n2);

  p2p.SetQueue ("ns3::RedQueue", // only backbone link has RED queue
                "LinkBandwidth", StringValue (redLinkDataRate),
                "LinkDelay", StringValue (redLinkDelay));
  p2p.SetDeviceAttribute ("DataRate", StringValue (redLinkDataRate));
  p2p.SetChannelAttribute ("Delay", StringValue (redLinkDelay));
  NetDeviceContainer devn2n3 = p2p.Install (n2n3);


Attributes
==========

The RED queue contains a number of attributes that control the RED
policies:

* Mode (bytes or packets)
* MeanPktSize
* IdlePktSize
* Wait (time)
* Gentle mode
* MinTh, MaxTh
* QueueLimit
* Queue weight
* LInterm
* LinkBandwidth
* LinkDelay

Consult the ns-3 documentation for explanation of these attributes.

Output
======

The ns-3 ascii trace helpers used by many of the NetDevices will hook
the Enqueue, Dequeue, and Drop traces of these queues and print out 
trace statements, such as the following from ``examples/udp/udp-echo.cc``:

::

  + 2 /NodeList/0/DeviceList/1/$ns3::CsmaNetDevice/TxQueue/Enqueue ns3::EthernetHeader 
  ( length/type=0x806, source=00:00:00:00:00:01, destination=ff:ff:ff:ff:ff:ff) 
  ns3::ArpHeader (request source mac: 00-06-00:00:00:00:00:01 source ipv4: 10.1.1.1 
  dest ipv4: 10.1.1.2) Payload (size=18) ns3::EthernetTrailer (fcs=0)
  - 2 /NodeList/0/DeviceList/1/$ns3::CsmaNetDevice/TxQueue/Dequeue ns3::EthernetHeader 
  ( length/type=0x806, source=00:00:00:00:00:01, destination=ff:ff:ff:ff:ff:ff) 
  ns3::ArpHeader (request source mac: 00-06-00:00:00:00:00:01 source ipv4: 10.1.1.1 
  dest ipv4: 10.1.1.2) Payload (size=18) ns3::EthernetTrailer (fcs=0)

which shows an enqueue "+" and dequeue "-" event at time 2 seconds.

Users are, of course, free to define and hook their own trace sinks to
these trace sources.

Examples
========

The drop-tail queue is used in several examples, such as 
``examples/udp/udp-echo.cc``.  The RED queue example is found at
``src/network/examples/red-tests.cc``.

Validation
**********

The RED model has been validated and the report is currently stored
at: https://github.com/downloads/talau/ns-3-tcp-red/report-red-ns3.pdf 

