#include "disk_driver.h"

void int_to_bytes(unsigned int num, byte* buffer, int offset) {
    int size = sizeof(unsigned int);
    for (int i = 0; i < size; i++)
        buffer[i + offset] = ((num >> (i * 8)) & 0xFF);
    return;
}

disk_driver::disk_driver() {
    registers = (unsigned int*)0xFFFFFF7FFF300000;
}

disk_driver::disk_driver(unsigned int emmc_registers_base_address) {
    registers = (unsigned int*)emmc_registers_base_address;
}

disk_driver::~disk_driver() {}

int disk_driver::read(unsigned int sector_offset, byte*& buffer) {
    unsigned int byte_offset = sector_offset * bytes_per_sector;

    Status status = wait_for_read_rdy();
    if (status != Status::OK)
        return status;

    registers[BLKSIZECNT] = (1 << 16) | bytes_per_sector;

    for (int i = 0; i < bytes_per_sector; i += bytes_per_int) {
        unsigned int int_byte_offset = byte_offset + (i * bytes_per_int);

        unsigned int response;
        Status status = issue_cmd(READ_SINGLE, int_byte_offset, response);
        if (status != Status::OK)
            return status;

        Status status = wait_for_read_rdy();
        if (status != Status::OK)
            return status;

        int_to_bytes(registers[DATA], buffer, i);
    }

    return Status::OK;
}

int disk_driver::write(unsigned int sector_offset, const byte*& buffer) {
}

disk_driver::Status disk_driver::issue_cmd(COMMAND_MASK cmd, unsigned int arg, unsigned int& response) {
    Status status = wait_for_cmd_rdy();
    if (status != Status::OK)
        return status;

    registers[ARG1]  = arg;
    registers[CMDTM] = cmd;

    Status status = wait_for_cmd_done();
    if (status != Status::OK)
        return status;

    response = registers[RESP0];

    return Status::OK;
}

disk_driver::Status disk_driver::wait_for_read_rdy() {
    int count = 1000000;
    while (any_errors() || !can_read()) {
        // wait(1);
        count--;
    }
    if (any_errors())
        return Status::ERROR;
    else if (!can_read())
        return Status::TIMEOUT;
    else
        return Status::OK;
}

disk_driver::Status disk_driver::wait_for_cmd_rdy() {
    int count = 1000000;
    while (any_errors() || !can_issue_cmd()) {
        // wait(1);
        count--;
    }
    if (any_errors())
        return Status::ERROR;
    else if (!can_issue_cmd())
        return Status::TIMEOUT;
    else
        return Status::OK;
}

disk_driver::Status disk_driver::wait_for_cmd_done() {
    int count = 1000000;
    while (any_errors() || !cmd_done()) {
        // wait(1);
        count--;
    }
    if (any_errors())
        return Status::ERROR;
    else if (!cmd_done())
        return Status::TIMEOUT;
    else
        return Status::OK;
}

bool disk_driver::any_errors() {
    bool any_interrupt_errors = registers[INTERRUPT] & INTERRUPT_MASK::ERROR;
    bool any_command_errors   = registers[CMDTM] & COMMAND_MASK::ERROR;
    return any_interrupt_errors || any_command_errors;
}

bool disk_driver::can_read() {
    bool read_available     = registers[STATUS] & STATUS_MASK::READ_AVAILABLE;
    bool read_ready         = registers[INTERRUPT] & INTERRUPT_MASK::READ_READY;
    bool prev_transfer_done = !(registers[STATUS] & STATUS_MASK::DATA_INHIBIT);
    return read_available && read_ready && prev_transfer_done;
}

bool disk_driver::can_write() {
    bool write_avl          = registers[STATUS] & STATUS_MASK::WRITE_AVAILABLE;
    bool write_rdy          = registers[INTERRUPT] & INTERRUPT_MASK::WRITE_READY;
    bool prev_transfer_done = !(registers[STATUS] & STATUS_MASK::DATA_INHIBIT);
    return write_avl && write_rdy && prev_transfer_done;
}

bool disk_driver::can_issue_cmd() {
    return !(registers[STATUS] & STATUS_MASK::CMD_INHIBIT);
}

bool disk_driver::cmd_done() {
    return !(registers[INTERRUPT] & INTERRUPT_MASK::CMD_DONE);
}