/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Once Day <once_day@qq.com>, All rights reserved.
 *
 * @FilePath: /book/ProgSelf/my_readelf.c
 * @Author: Once Day <once_day@qq.com>.
 * @Date: 2026-02-09 22:09
 * @info: Encoder=utf-8, TabSize=4, Eol=\n.
 *
 * @Description:
 *  简易版 readelf 工具, 解析 ELF 文件头和节区头信息.
 *
 * @History:
 *  2026年2月9日, Once Day <once_day@qq.com>, 创建此文件.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bfd.h>
#include <elf.h>

int main(int argc, char *argv[])
{
    // 查看 bpf 支持的 ELF 文件格式
    const char **target = bfd_target_list();
    while (*target != NULL) {
        printf("Supported ELF target: %s\n", *target);
        target++;
    }

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <elf_file>\n", argv[0]);
        return 1;
    }

    const char *elf_file = argv[1];
    printf("Parsing ELF file: %s\n", elf_file);

    FILE *file = fopen(elf_file, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    // 读取 ELF 头
    Elf64_Ehdr elf_header;
    if (fread(&elf_header, sizeof(Elf64_Ehdr), 1, file) != 1) {
        perror("Failed to read ELF header");
        fclose(file);
        return 1;
    }

    // 验证 ELF 魔数
    if (memcmp(elf_header.e_ident, ELFMAG, SELFMAG) != 0) {
        fprintf(stderr, "Not a valid ELF file\n");
        fclose(file);
        return 1;
    }

    // 输出 ELF 头信息
    printf("ELF Header:\n");
    printf("  Class: %s\n", (elf_header.e_ident[EI_CLASS] == ELFCLASS64) ? "ELF64" : "ELF32");
    printf("  Data: %s\n",
        (elf_header.e_ident[EI_DATA] == ELFDATA2LSB) ? "Little Endian" : "Big Endian");
    printf("  Version: %d\n", elf_header.e_ident[EI_VERSION]);
    printf("  OS/ABI: %d\n", elf_header.e_ident[EI_OSABI]);
    printf("  ABI Version: %d\n", elf_header.e_ident[EI_ABIVERSION]);
    printf("  Type: %d\n", elf_header.e_type);
    printf("  Machine: %d\n", elf_header.e_machine);
    printf("  Entry point: 0x%lx\n", elf_header.e_entry);
    printf("  Program header offset: %lu\n", elf_header.e_phoff);
    printf("  Section header offset: %lu\n", elf_header.e_shoff);
    printf("  Flags: %u\n", elf_header.e_flags);
    printf("  ELF header size: %u\n", elf_header.e_ehsize);
    printf("  Program header entry size: %u\n", elf_header.e_phentsize);
    printf("  Program header entry count: %u\n", elf_header.e_phnum);
    printf("  Section header entry size: %u\n", elf_header.e_shentsize);
    printf("  Section header entry count: %u\n", elf_header.e_shnum);
    printf("  Section header string table index: %u\n", elf_header.e_shstrndx);

    return 0;
}
