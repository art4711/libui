## Use case ##

There's a big risk that the use case pixmaps were designed for might
not be the optimal design for most use cases. The use case is: go
application wants an empty RGB bitmap to efficiently draw to.

## Formats ##

Different backends support different formats.

Things to consider:

32 bit pixels or packed 24 bit pixels.
alpha or no alpha
alpha first, alpha last
pixels treated as a byte array or uint32
uint32 endianness
alpha pre-multiplied or not
0,0 corner

For our use case we can find these examples:

 - Cairo can do pre-multiplied host-endian uint32 ARGB or host-endian
   uint32 RGB with highest bits ignored. Origin in upper left corner.

 - Gdk only deals with 3 byte packed RGB images, nothing else.

 - Quartz can do pre-multiplied or not uint32 ARGB or RGBA and 32 bit
   RGB with either lowest or highest bits ignored. Big endian by
   default, cannot be set to host endian, but can be set to little or
   big (so the code needs to switch on endianness). Origin in lower
   left corner.

 - windows ???

The question is if there should be multiple formats supported with the
library doing the conversion, or just one. Some backends can deal with
format conversion more efficiently, but others can't and there'd need
to be a whole infrastructure in the library for format conversions.

For now the best option is to hard code the only supported format to
pre-multiplied host-endian ARGB with origin in upper left corner since
it is the most efficient lowest common denominator (gdk can be
ignored) and requires the least coversion before being handed to the
next layer. The conversion is a simple copy which may be necessary
anyway because of allocation.

## Allocation ##

It's tempting to punt the allocation to the application. It's the
easiest thing to implement in the library and we don't have to do any
buffer lifecycle management. Unfortunately things aren't so easy.

Since the use case is Go we have to deal with garbage collection.
There are two ways to look at it. Either strict standards compliance
or what works practically.

Strict standards compliance in Go is pretty clear: cgo may be passed a
pointer to go data, but may not use it after returning.  Quartz and
cairo documentation make it pretty obvious that the pointer will be
retained for longer than just the function call. Both have mechanisms
to let the caller know when to free the data after it's no longer used
which pretty much says it clearly that the life cycle is out of our
hands.

What works practically in Go looks a bit better, but not much. We know
that go will not reallocate data on the heap. There's no documented
way of forcing some data to the heap, but we can be pretty certain in
most cases. Still, this can change in any go version. Also, it's
pretty dangerous if the buffer is legitimately (equivalent to) freed
in Go, garbage collected and used for something else while the backend
still keeps a pointer to it. To get around that would require a lot of
effort for not much gain.

A good option then looks like dealing with the allocation within the
library. But since there is no single format that has the exact same
bit pattern between all the supported backends there either needs to
be a conversion somewhere or the API for images will become
exceedingly rich and hard to use.

For now, we let the application deal with allocation in a standard
format (32 bit host endian ARGB with pre-multiplied alpha) and
efficiently copy the data for ourselves when drawing.

## Loading of images ##

As mentioned, the use case is - application entirely deals with
generating the image. For now, no thought has been put into dealing
with loading images in multiple formats from disk, URLs, compressed
resources, etc.
