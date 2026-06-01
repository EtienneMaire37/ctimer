#pragma once

void print_at(int x, int y, char (*filter)(int, int, char), const char* fmt, ...);
char simple_filter(int x, int y, char ch);
