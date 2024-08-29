#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

int *count_files(const char *dir_path, const char *houzhui, int *count_rcf)
{
    DIR *dir = opendir(dir_path);
    if (!dir)
    {
        printf("无法打开目录: %s\n", dir_path);
        return NULL;
    }

    struct dirent *d;

    while ((d = readdir(dir)) != NULL)
    {
        // 跳过目录 "." 和 ".."
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
        {
            continue;
        }

        // 构造完整的文件路径
        char filepath[4096];
        snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, d->d_name);

        if (d->d_type == 4)
            count_files(filepath, houzhui, count_rcf);

        // 检查文件后缀
        if (strstr(d->d_name, houzhui) != NULL)
        {
            // 获取文件名长度和后缀名长度
            size_t len = strlen(d->d_name);
            size_t hz_len = strlen(houzhui);

            // 判断文件名最后的部分是否与后缀名一致
            if (len > hz_len && strcmp(d->d_name + len - hz_len, houzhui) == 0)
            {
                printf("文件 %s:\n", filepath);
                count_rcf[2]++;
                FILE *fp = fopen(filepath, "r");
                if (fp == NULL)
                {
                    perror("fopen");
                    continue;
                }

                char line[1024];
                int row_num = 0, c_num = 0, empty_lines = 0, comment_lines = 0, code_lines = 0;
                int in_comment_block = 0; // 标记是否在多行注释块中

                while (fgets(line, sizeof(line), fp) != NULL)
                {
                    row_num++;
                    int len = strlen(line);
                    c_num += len;        // 该文件字数
                    count_rcf[1] += len; // 总文件字数

                    // 去除行首和行尾的空白字符
                    char *start = line;
                    while (isspace((unsigned char)*start))
                        start++;
                    char *end = line + len - 1;
                    while (end > start && isspace((unsigned char)*end))
                        end--;
                    end[1] = '\0';

                    if (start[0] == '\0')
                    {
                        // 空行
                        empty_lines++;
                        count_rcf[3]++;
                    }
                    else if (in_comment_block || strstr(start, "/*") != NULL)
                    {
                        // 多行注释
                        comment_lines++;
                        count_rcf[5]++;
                        if (strstr(start, "*/") != NULL)
                        {
                            in_comment_block = 0;
                        }
                        else
                        {
                            in_comment_block = 1;
                        }
                    }
                    else if (strstr(start, "//") != NULL)
                    {
                        // 单行注释
                        comment_lines++;
                        count_rcf[5]++;
                    }
                    else
                    {
                        // 代码行
                        code_lines++;
                        count_rcf[4]++;
                    }
                }

                count_rcf[0] += row_num;

                printf("行数:%d, 字节数:%d, ", row_num, c_num);
                printf("空行:%d, 代码行:%d, 注释行:%d\n", empty_lines, code_lines, comment_lines);

                fclose(fp);
            }
        }
    }

    closedir(dir);
    return count_rcf;
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("请指定目录!\n格式: ./count_files folder_path\n");
        return EXIT_FAILURE;
    }

    int c_f[6] = {0, 0, 0, 0, 0, 0}; // .c 文件的行数、字节数、文件数、空行数、代码行数、注释行数
    int h_f[6] = {0, 0, 0, 0, 0, 0}; // .h 文件

    count_files(argv[1], ".c", c_f);
    count_files(argv[1], ".h", h_f);

    printf("--- .c ---\n总行数: %d  总字节数: %d  总文件数: %d\n", *c_f, *(c_f + 1), *(c_f + 2));
    printf("总空行数: %d  总代码行数: %d  总注释行数: %d\n", *(c_f + 3), *(c_f + 4), *(c_f + 5));
    printf("--- .h ---\n总行数: %d  总字节数: %d  总文件数: %d\n", *h_f, *(h_f + 1), *(h_f + 2));
    printf("总空行数: %d  总代码行数: %d  总注释行数: %d\n", *(h_f + 3), *(h_f + 4), *(h_f + 5));
    printf("--- .c and .h ---\n总行数: %d  总字节数: %d  总文件数: %d  \n", 
            *c_f + *h_f, *(c_f + 1) + *(h_f + 1), *(c_f + 2) + *(h_f + 2));
    printf("总空行数: %d  总代码行数: %d  总注释行数: %d\n", 
            *(c_f + 3) + *(h_f + 3), *(c_f + 4) + *(h_f + 4), *(c_f + 5) + *(h_f + 5));

    return 0;
}
