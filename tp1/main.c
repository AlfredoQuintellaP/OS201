/**
*    author: AlfredoQuintella
*    created: 2025-09-18 22:29:16
*    NeoVim supremacy
**/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>

#include <stdio.h>   // <-- for printf tests

// ===================== fs_header =====================
// This structure represents the superblock of the ROMFS.
// It is always located at offset 0 of the filesystem image.
// ------------------------------------------------------
// Fields:
//   magic       : (Q3) Magic string, must be "-rom1fs-".
//   full_size   : (Q4) Total size of the FS image in bytes (big-endian).
//   checksum    : (Q3) Checksum of the first 512 bytes of the image.
//   volume_name : (Q5–Q6) Null-terminated volume name, padded to 16 bytes.
// ------------------------------------------------------
// After this header, the first file header begins at the next
// 16-byte aligned offset.

struct fs_header {
    char magic[8];       // "-rom1fs-"
    uint32_t full_size;  // number of accessible bytes
    uint32_t checksum;   // checksum of first 512 bytes
    char volume_name[];  // flexible array member (null-terminated, padded to 16)
};

// ===================== file_header =====================
// This structure describes one file or directory inside the ROMFS.
// Each file header begins at a 16-byte boundary.
// ------------------------------------------------------
// Fields:
//   next     : (Q7–Q8) Offset (relative to FS start) of the next file header.
//              The lowest 4 bits are mode/type info, not part of the offset.
//              - bits 0–2 : file type
//              - bit 3    : executable flag
//   spec_info: (Q9) Extra information depending on file type.
//              - Directory : offset of first file header inside directory
//              - Hard link : offset of linked file
//              - Device    : major/minor numbers
//              - Regular file : usually 0
//   size     : (Q7, Q10) Size of the file data in bytes.
//   checksum : (Q7) Checksum covering metadata, name, and padding.
//   name     : (Q8–Q10) Null-terminated filename, padded to 16 bytes.
//              The file data follows immediately after, aligned to 16.
// ------------------------------------------------------
// File headers form a linked list. Directories are simply lists
// of file headers. (Q8–Q10 use this to implement ls/find/message.txt)

struct file_header {
    uint32_t next;       // offset of next file header 
    uint32_t spec_info;  // depends on type (dir, link, device…)
    uint32_t size;       // size of file in bytes
    uint32_t checksum;   // checksum including metadata + name + padding
    char name[];         // flexible array member (null-terminated, padded to 16)
};


uint32_t read32(char ptr[4]) {
    return ((uint32_t)(uint8_t)ptr[0] << 24) |
           ((uint32_t)(uint8_t)ptr[1] << 16) |
           ((uint32_t)(uint8_t)ptr[2] << 8)  |
           ((uint32_t)(uint8_t)ptr[3]);
}

void decode(struct fs_header *p, size_t size) {
    const uint8_t expected[8] = { '-', 'r', 'o', 'm', '1', 'f', 's', '-' };
    for (int i = 0; i < 8; i++) {
        assert(p->magic[i] == expected[i]);
    }
    printf("Magic string OK: %.8s\n", p->magic); // for testing

    uint32_t header_size = read32((char*)&p->full_size);
    printf("Header full_size = %u, actual file size = %zu\n", header_size, size); // for testing 
    assert(header_size <= size);

    printf("Checksum field = 0x%08X\n", read32((char*)&p->checksum)); // for testing 
    printf("Volume name = \"%s\"\n", p->volume_name); // for testing 

    return;
}

uint32_t round16(uint32_t n) {
    return (n + 15) & ~15; 
}

uint32_t first_file_header_offset(struct fs_header *p) {
    uint32_t len = 0;
    while (p->volume_name[len] != '\0') {
        len++;
    }
    len++; // count null terminator
    return 16 + round16(len);
}

int main(void) {
    int fd = open("tp1fs.romfs", O_RDONLY);
    assert(fd != -1);
    off_t fsize;
    fsize = lseek(fd, 0, SEEK_END);

    void *addr = mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
    assert(addr != MAP_FAILED);

    struct fs_header *hdr = (struct fs_header*)addr;
    decode(hdr, fsize);

    uint32_t first_offset = first_file_header_offset(hdr);
    printf("First file header offset = %u (0x%X)\n", first_offset, first_offset); // for testing 

    return 0;
}
