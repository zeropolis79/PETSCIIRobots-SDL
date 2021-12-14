#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/memory.h>

__asm int ungzip__FPvPv(register __a0 void* input, register __a1 void* output);

unsigned char* input = 0;
unsigned char* output = 0;
unsigned long inputSize = 0;
unsigned long outputSize = 0;

int cleanup(int returnCode, char* error)
{
    if (output) {
        FreeMem(output, outputSize);
    }

    if (input) {
        FreeMem(input, inputSize);
    }

    if (error) {
        char *errorEnd = error;
        while (*errorEnd) {
            errorEnd++;
        }

        Write(Output(), error, errorEnd - error);
    }

    return returnCode;
}

int main(int argc, char *argv[])
{
    BPTR lock, file;
    struct FileInfoBlock fib;

    if (argc != 3) {
        return cleanup(10, "Usage: DecompressData input output\n");
    }

    lock = Lock(argv[1], ACCESS_READ);
    if (lock) {
        if (Examine(lock, &fib)) {
            UnLock(lock);

            inputSize = fib.fib_Size;
            input = AllocMem(inputSize, MEMF_ANY);
            if (!input) {
                return cleanup(10, "Unable to allocate memory\n");
            }

            file = Open(argv[1], MODE_OLDFILE);
            if (file) {
                Read(file, input, inputSize);
                outputSize = (input[inputSize - 1] << 24) | (input[inputSize - 2] << 16) | (input[inputSize - 3] << 8) | input[inputSize - 4];
                Close(file);

                output = AllocMem(outputSize, MEMF_ANY);
                if (!output) {
                    return cleanup(10, "Unable to allocate memory\n");
                }

                ungzip__FPvPv(input, output);

                file = Open(argv[2], MODE_NEWFILE);
                if (file) {
                    Write(file, output, outputSize);
                    Close(file);

                    return cleanup(0, 0);
                } else {
                    return cleanup(10, "Unable to open output file\n");
                }
            }
        }
    }

    return cleanup(10, "Unable to open input file\n");
}
