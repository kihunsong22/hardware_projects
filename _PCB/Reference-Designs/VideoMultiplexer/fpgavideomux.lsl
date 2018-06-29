memory iram
{
    mau = 8;
    type = ram;
    size = 256;
    map ( dest=bus:c51:idata_bus, src_offset=0x0, dest_offset=0x0, size=256 );
}
memory xram
{
    mau = 8;
    type = ram;
    size = 64k;
    map ( dest=bus:c51:xdata_bus, src_offset=0x0, dest_offset=0x0, size=64k );
}
memory xrom
{
    mau = 8;
    type = rom;
    size = 64k;
    map ( dest=bus:c51:code_bus, src_offset=0x0, dest_offset=0x0, size=64k );
}
section_layout c51:c51:xdata {
    group (run_addr = 0x0, ordered, contiguous) reserved "no_null_ptr" (size=1);
}
#define __STACK 0x20
#define __VSTACK_XDATA 0x400
#define __VSTACKADDR_XDATA
#define __XHEAP 0x200
#define __XPAGE 0x00
#define __MEMORY
