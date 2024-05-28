#ifndef KERNEL_DISK_DRIVER_H
#define KERNEL_DISK_DRIVER_H

typedef unsigned char byte;

class disk_driver {
   public:
    enum Status : int {
        OK      = 0,
        ERROR   = 1,
        TIMEOUT = 2
    };

    disk_driver();
    disk_driver(unsigned int emmc_registers_base_address);
    ~disk_driver();

    int read(unsigned int sector_offset, byte*& buffer);
    int write(unsigned int sector_offset, const byte*& buffer);

   private:
    enum Register : int {
        ARG2           = 0,
        BLKSIZECNT     = 1,
        ARG1           = 2,
        CMDTM          = 3,
        RESP0          = 4,
        RESP1          = 5,
        RESP2          = 6,
        RESP3          = 7,
        DATA           = 8,
        STATUS         = 9,
        CONTROL0       = 10,
        CONTROL1       = 11,
        INTERRUPT      = 12,
        IRPT_MASK      = 13,
        IRPT_EN        = 14,
        CONTROL2       = 15,
        FORCE_IRPT     = 16,
        BOOT_TIMEOUT   = 17,
        DBG_SEL        = 18,
        EXRDFIFO_CFG   = 19,
        EXRDFIFO_EN    = 20,
        TUNE_STEP      = 21,
        TUNE_STEPS_STD = 22,
        TUNE_STEPS_DDR = 23,
        SPI_INT_SPT    = 24,
        SLOTISR_VER    = 25
    };

    enum STATUS_MASK : unsigned int {
        READ_AVAILABLE  = 0x00000800,
        WRITE_AVAILABLE = 0x00000400,
        DATA_INHIBIT    = 0x00000002,
        CMD_INHIBIT     = 0x00000001
    };

    enum INTERRUPT_MASK : unsigned int {
        ERROR        = 0x017E8000,
        DATA_TIMEOUT = 0x00100000,
        CMD_TIMEOUT  = 0x00010000,
        READ_READY   = 0x00000020,
        WRITE_READY  = 0x00000010,
        DATA_DONE    = 0x00000002,
        CMD_DONE     = 0x00000001
    };

    enum COMMAND_MASK : unsigned int {
        ERROR         = 0xfff9c004,
        NEED_APP      = 0x80000000,
        GO_IDLE       = 0x00000000,
        ALL_SEND_CID  = 0x02010000,
        SEND_REL_ADDR = 0x03020000,
        CARD_SELECT   = 0x07030000,
        SEND_IF_COND  = 0x08020000,
        STOP_TRANS    = 0x0C030000,
        READ_SINGLE   = 0x11220010,
        READ_MULTI    = 0x12220032,
        SET_BLOCKCNT  = 0x17020000,
        WRITE_SINGLE  = 0x18220000,
        WRITE_MULTI   = 0x19220022,
        APP_CMD       = 0x37000000,
        SET_BUS_WIDTH = (0x06020000 | NEED_APP),
        SEND_OP_COND  = (0x29020000 | NEED_APP),
        SEND_SCR      = (0x33220010 | NEED_APP)
    };

    const unsigned int bytes_per_sector = 512;
    const unsigned int bytes_per_int    = 4;
    const unsigned int ints_per_sector  = bytes_per_sector / bytes_per_int;

    unsigned int* volatile registers;

    Status issue_cmd(COMMAND_MASK cmd, unsigned int arg, unsigned int& response);

    Status wait_for_cmd_done();
    Status wait_for_cmd_rdy();
    Status wait_for_read_rdy();

    bool any_errors();

    bool can_read();
    bool can_write();
    bool can_issue_cmd();
    bool cmd_done();
};

#endif