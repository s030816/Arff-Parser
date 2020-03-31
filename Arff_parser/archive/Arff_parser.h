#pragma once

#include "resource.h"


typedef struct ATTRIBUTE_ ATTRIBUTE;
typedef struct FILE_BUFFER_ FILE_BUFFER;

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

HWND reg_obj(const WCHAR *name, WCHAR *classname, unsigned long attributes,
	unsigned short xPos, unsigned short yPos, unsigned short width, 
	unsigned short height, HWND parent, HMENU num);
void add_item(HWND hWnd, char *text, HMENU id);
ATTRIBUTE *read_data_attributes(WCHAR *path, HWND hWnd, size_t *atr_count);
void update_lists(HWND hWnd, ATTRIBUTE *atrb, size_t atr_count);
void insert_item(ATTRIBUTE *atrb, HWND hWnd);
void remove_item(ATTRIBUTE *atrb, HWND hWnd);
INT_PTR CALLBACK BrowseCallbackProc(HWND, UINT, LPARAM, LPARAM);
void browser(WCHAR *path_str);

//Implement
void extract_data(WCHAR *path_str, ATTRIBUTE *atrb, size_t atr_count);
FILE_BUFFER *file_search(WCHAR *path_str, size_t *f_count, HWND hWnd);