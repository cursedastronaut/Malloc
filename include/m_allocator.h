#pragma once
#include <stddef.h>

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m" 
 
// Memory functions
void* m_malloc(size_t size);
void* m_realloc(void* ptr, size_t size);
void* m_calloc(size_t nb, size_t size);
void m_free(void* ptr);
 
void m_setup_hooks(void); // Hook this allocator when using standard library
void m_show_info(void); // Display allocator informations
