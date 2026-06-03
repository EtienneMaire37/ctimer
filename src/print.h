#pragma once

int print_at(int x, int y, char (*filter)(int, int, char), const char* fmt, ...);
void print_time_at(int y, float time);

char simple_filter(int x, int y, char ch);
char basic_filter(int x, int y, char ch);
