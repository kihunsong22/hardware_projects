
  DSF_Video demo
  ==============

Initializing the IDE drive
--------------------------

Use any (desktop) system to partition the IDE drive with a big enough
first partition to contain your video samples. A universal USB-2.0 IDE
enclosure is especially suitable for this purpose. Format this
partition as FAT32. Place some suitable AVI files in its root.

IMPORTANT: The included filesystem only supports standard CHS and LBA
drives, LBA-48 is not supported. This means the biggest harddisk usable
is about 130Gb.


Creating a suitable AVI
-----------------------

To play with this project an AVI must be created with the following
characteristics:

- video format must be 16 bit RGB (5:6:5) uncompressed, bottom up, little
  endian (sometimes called 'uncompressed 64K color' on Microsoft platforms)
  RGB 555 is supported as well ('uncompressed 32K color')
- filesize maximum 1Gb.
- audio format must be uncompressed, little endian
  (8/16-bit and mono/stereo are supported)
- width x height maximum 320x240
- framerate about 12 fps maximum
- audio samplerate 22050 Hz maximum

Framerate and samplerate are not absolute limits, this is also
dependant on the harddisk used. Playback will stutter if CPU or
harddisk cannot keep up.

A free tool to convert an existing video to this format is VirtualDub
(v1.6.9 at least, earlier versions don't support the 16-bit RGB 5:5:5 format).

To read most modern videos some VFW codes need to be installed, if not
already present on the system FFDSHOW is a free codec with VFW support for
a lot of modern formats.


A very short guide:
- open file in VirtualDub
- menu 'video->filters', add the filter 'resize'
  * new width x height=320 x 240 (correct for aspect ratio if needed, but
    don't exceed)
  * mode=Lanczos3
- menu 'video->framerate'
  * frame rate conversion = this depends on the original framerate and
    on the speed of the IDE drive used, a safe bet is to end up with about
    12 frames per second. (15 fps for 320x180=16:9 video)
    Every 2nd or 3rd frame gives the best results.
- menu 'video->colordepth'
  * decompression=autoselect
  * output=16 bit RGB (565)
- turn on menu 'audio->full processing', then menu 'audio->conversion'
  * sampling rate= if original is above 22050 convert to 22050 max, otherwise
    no change is probably needed
- play the video (by pressing 'space').
  if A/V sync problems try to correct them by changing the value in menu
  Audio->Interleaving, field 'Delay audio track by'.
- menu 'file->save as AVI'




