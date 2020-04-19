#pragma once

#include "resource.h"

typedef struct ATTRIBUTE_ ATTRIBUTE;
typedef struct FILE_BUFFER_ FILE_BUFFER;
typedef struct EX_DATA_ARGS_ EX_DATA_ARGS;

struct ATTRIBUTE_
{
	BOOL selected;
	size_t indx;
	char *attrib_buffer;
	char *datatype;
};

struct FILE_BUFFER_
{
	BOOL selected;
	size_t indx;
	WCHAR path[MAX_PATH];
};

struct EX_DATA_ARGS_
{
	WCHAR *path;
	ATTRIBUTE *atrb;
	size_t atr_count;
	char *output;
	HWND sbar_handle;
	size_t ots;
};

extern char relationHeader[4096];
extern HANDLE ghMutex;

HWND reg_obj(const WCHAR *name, WCHAR *classname, unsigned long attributes,
	unsigned short xPos, unsigned short yPos, unsigned short width, 
	unsigned short height, HWND parent, HMENU num);
void add_item(HWND hWnd, char *text, HMENU id);
ATTRIBUTE *read_data_attributes(WCHAR *path, HWND hWnd, size_t *atr_count);
//void update_lists(HWND hWnd, ATTRIBUTE *atrb, const size_t atr_count);
void update_selected_file_list(FILE_BUFFER *fbuf, HWND hWnd);
void insert_item(ATTRIBUTE *atrb, HWND hWnd);
void remove_item(ATTRIBUTE *atrb, HWND hWnd);
void make_header(FILE *fp, ATTRIBUTE * atrb, const size_t atr_count);
INT_PTR CALLBACK BrowseCallbackProc(HWND, UINT, LPARAM, LPARAM);
void browser(WCHAR *path_str);
void extract_data(WCHAR * path, ATTRIBUTE * atrb, const size_t atr_count, char **out, size_t *out_s);
unsigned int __stdcall fread_thread(void* data);
FILE_BUFFER *file_search(WCHAR *path_str, size_t *f_count, HWND hWnd);

// Improve
void read_files(FILE_BUFFER *files, const size_t f_count,
	ATTRIBUTE *atrb, const size_t atr_count, HWND testbar);
