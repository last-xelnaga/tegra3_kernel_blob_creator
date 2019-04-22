
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define PARTITION_SIZE_OFFSET   96
#define COPY_BUFFER_SIZE        5000


unsigned char blob_header [] =
{
    // header, 28 bytes
    '-', 'S', 'I', 'G', 'N', 'E', 'D','-', 'B', 'Y', '-', 'S', 'I', 'G', 'N', 'B', 'L', 'O', 'B', '-',
    0x29, 0xc3, 0x94, 0x20, 0x00, 0x01, 0x00, 0x00,
    // magic, 16 bytes
    'M', 'S', 'M', '-', 'R', 'A', 'D', 'I', 'O', '-', 'U', 'P', 'D', 'A', 'T', 'E',
    // version, 4 bytes
    0x00, 0x00, 0x01, 0x00,
    // size, 4 bytes
    0x3c, 0x00, 0x00, 0x00,
    // offset, 4 bytes
    0x3c, 0x00, 0x00, 0x00,
    // partitions count, 4 bytes
    0x01, 0x00, 0x00, 0x00,
    // spare space, 28 bytes
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // partition name, 4 bytes
    'L', 'N', 'X', 0x00,
    // partition offset, 4 bytes
    0x4c, 0x00, 0x00, 0x00,
    // partition size, 4 bytes, use PARTITION_SIZE_OFFSET to update this field
    0x00, 0x00, 0x00, 0x00,
    // partition version, 4 bytes
    0x01, 0x00, 0x00, 0x00
};

void usage (
        void)
{
    fprintf (stderr, "\n");
    fprintf (stderr, "Usage:\n");
    fprintf (stderr, "kernel_blob_creator -i <img file name> -o <blob file name>\n");
    fprintf (stderr, "\n");
    fprintf (stderr, "Example:\n");
    fprintf (stderr, "./kernel_blob_creator -i boot.img -o boot.blob\n");
    fprintf (stderr, "\n");

    exit (EXIT_FAILURE);
}

int main (
        int argc,
        char** argv)
{
    int opt;
    char* p_infile_name = NULL;
    char* p_outfile_name = NULL;

    while ((opt = getopt (argc, argv, "i:o:")) != -1)
    {
        switch (opt)
        {
            case 'i':
                p_infile_name = optarg;
                break;
            case 'o':
                p_outfile_name = optarg;
                break;
            default:
                usage ();
        }
    }

    // check params and abort in case of missed one
    if (p_infile_name == NULL || p_outfile_name == NULL)
    {
        usage ();
    }

    // get the boot.img file size
    FILE* p_infile = fopen (p_infile_name, "rb");
    fseek (p_infile, 0, SEEK_END);
    long infile_size = ftell (p_infile);
    fseek (p_infile, 0, SEEK_SET);

    // modify header, update the partition size using little endian notation
    unsigned int infile_size_temp = infile_size;
    blob_header [PARTITION_SIZE_OFFSET] = (unsigned char) (infile_size_temp & 0xff);
    infile_size_temp >>= 8;
    blob_header [PARTITION_SIZE_OFFSET + 1] = (unsigned char) (infile_size_temp & 0xff);
    infile_size_temp >>= 8;
    blob_header [PARTITION_SIZE_OFFSET + 2] = (unsigned char) (infile_size_temp & 0xff);
    infile_size_temp >>= 8;
    blob_header [PARTITION_SIZE_OFFSET + 3] = (unsigned char) (infile_size_temp & 0xff);

    // write header to the output file
    FILE* p_outfile = fopen (p_outfile_name, "wb");
    fwrite (blob_header, sizeof (blob_header), 1, p_outfile);

    // append output file using partition file
    unsigned int buffer_size;
    unsigned char copy_buffer [COPY_BUFFER_SIZE];
    while (infile_size)
    {
        if (infile_size >= COPY_BUFFER_SIZE)
            buffer_size = COPY_BUFFER_SIZE;
        else
            buffer_size = infile_size;

        fread (copy_buffer, 1, buffer_size, p_infile);
        fwrite (copy_buffer, 1, buffer_size, p_outfile);
        infile_size -= buffer_size;
    }

    fclose (p_infile);
    fclose (p_outfile);

    return 0;
}
