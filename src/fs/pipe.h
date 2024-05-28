#ifndef KERNEL_PIPE_H
#define KERNEL_PIPE_H

#include "util/hasrefcount.h"
#include "filecontext.h"

namespace kernel::fs
{

    class Pipe
    {
    public:
        Pipe();

        ~Pipe();

        int put(void *data, int n);

        int read(void *data, int n);

        FileContext *createReader();

        FileContext *createWriter();

        void addReader();

        void removeReader();

        void addWriter();

        void removeWriter();

        int getReaderCount() const;

        int getWriterCount() const;

    private:
        static const int PIPE_SIZE = 4096;

        char buffer[PIPE_SIZE];

        int writePos, readPos;

        int readerCount, writerCount;

        class PipeReader : public FileContext
        {
        public:
            PipeReader(Pipe *pipe);

            ~PipeReader();

            int read(void *buffer, int n);

            int write(const void *buffer, int n);

            FileContext *copy();

        private:
            Pipe *pipe;
        };

        class PipeWriter : public FileContext
        {
        public:
            PipeWriter(Pipe *pipe);

            ~PipeWriter();

            int read(void *buffer, int n);

            int write(const void *buffer, int n);

            FileContext *copy();

        private:
            Pipe *pipe;
        };
    };

}

#endif