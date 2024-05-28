#include "pipe.h"
#include "filecontext.h"
#include "types/status.h"

kernel::fs::Pipe::Pipe()
    : writePos(0), readPos(0), readerCount(0), writerCount(0)
{
}

kernel::fs::Pipe::~Pipe()
{
}

int kernel::fs::Pipe::put(void *data, int n)
{
    if (getReaderCount() == 0)
    {
        return EPIPE;
    }

    char *s = (char *)data;
    int c = 0;
    while (c < n && !(writePos == PIPE_SIZE - 1 && readPos == 0) && !(writePos + 1 == readPos))
    {
        buffer[writePos] = s[c];
        c++;
        writePos++;
        if (writePos >= PIPE_SIZE)
        {
            writePos = 0;
        }
    }

    if (n > 0 && c == 0)
    {
        return EFULL;
    }
    else
    {
        return c;
    }
}

int kernel::fs::Pipe::read(void *data, int n)
{
    char *s = (char *)data;
    int c = 0;
    while (readPos != writePos && c < n)
    {
        s[c] = buffer[readPos];
        c++;
        readPos++;
        if (readPos >= PIPE_SIZE)
        {
            readPos = 0;
        }
    }
    return c;
}

kernel::fs::FileContext *kernel::fs::Pipe::createReader()
{
    return new PipeReader(this);
}

kernel::fs::FileContext *kernel::fs::Pipe::createWriter()
{
    return new PipeWriter(this);
}

void kernel::fs::Pipe::addReader()
{
    readerCount++;
}

void kernel::fs::Pipe::removeReader()
{
    readerCount--;
}

void kernel::fs::Pipe::addWriter()
{
    writerCount++;
}

void kernel::fs::Pipe::removeWriter()
{
    writerCount--;
}

int kernel::fs::Pipe::getReaderCount() const
{
    return readerCount;
}

int kernel::fs::Pipe::getWriterCount() const
{
    return writerCount;
}

kernel::fs::Pipe::PipeReader::PipeReader(Pipe *pipe)
    : pipe(pipe)
{
    pipe->addReader();
}

kernel::fs::Pipe::PipeReader::~PipeReader()
{
    pipe->removeReader();
    if (pipe->getReaderCount() == 0 && pipe->getWriterCount() == 0)
    {
        delete pipe;
    }
}

int kernel::fs::Pipe::PipeReader::read(void *buffer, int n)
{
    int c = pipe->read(buffer, n);
    if (c == 0 && pipe->getWriterCount() == 0)
    {
        return EEOF;
    }
    else
    {
        return c;
    }
}

int kernel::fs::Pipe::PipeReader::write(const void *buffer, int n)
{
    return EIO;
}

kernel::fs::FileContext *kernel::fs::Pipe::PipeReader::copy()
{
    return new PipeReader(pipe);
}

kernel::fs::Pipe::PipeWriter::PipeWriter(Pipe *pipe)
    : pipe(pipe)
{
    pipe->addWriter();
}

kernel::fs::Pipe::PipeWriter::~PipeWriter()
{
    pipe->removeWriter();
    if (pipe->getReaderCount() == 0 && pipe->getWriterCount() == 0)
    {
        delete pipe;
    }
}

int kernel::fs::Pipe::PipeWriter::read(void *buffer, int n)
{
    return EIO;
}

int kernel::fs::Pipe::PipeWriter::write(const void *buffer, int n)
{
    return pipe->put(buffer, n);
}

kernel::fs::FileContext *kernel::fs::Pipe::PipeWriter::copy()
{
    return new PipeWriter(pipe);
}
