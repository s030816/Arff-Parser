
#include "stdafx.h"
#include "Arff_parser.h"

#define SECTOR_SIZE 4096

void debug(int number)
{
	WCHAR str[100];
	wsprintfW(str, L"debug: %d", number);
	MessageBoxW(NULL, str,L"Debug", MB_OK);
}

void error(WCHAR *text)
{
	MessageBoxW(NULL, text, L"Error", MB_OK);
}

void custom_sscanf(char *buffer, char *t1, char *t2)
{
	while (*buffer != ' ')
	{
		*t1++ = *buffer++;
	}
	*t1 = '\0';
	strcpy(t2, ++buffer);
}

HWND reg_obj(const WCHAR *name, WCHAR *classname, unsigned long attributes, 
	unsigned short xPos, unsigned short yPos, unsigned short width, unsigned short height, HWND parent, HMENU num)
{
	HWND hWnd = NULL;
	return (hWnd = CreateWindow(classname, name, attributes, xPos, yPos, width, height,
		parent, num, (HINSTANCE)GetWindowLong(parent, GWL_HINSTANCE), NULL));
}

void add_item(HWND hWnd, char *text, HMENU id)
{
	static size_t horizontal_extent = 35;
	if (strlen(text) > horizontal_extent)
	{
		horizontal_extent = strlen(text);
		//debug(horizontal_extent);
		SendMessageW(GetDlgItem(hWnd, (int)id),
			LB_SETHORIZONTALEXTENT,
			(WPARAM)horizontal_extent*9,
			(LPARAM)NULL);
	}
	SendMessageA(GetDlgItem(hWnd, (int)id),
		LB_ADDSTRING,
		(WPARAM)NULL,
		(LPARAM)text);


}
/*
void remove_item(HWND hWnd, HMENU id)
{
	int test[1] = {0};
	while(SendMessage(GetDlgItem(hWnd, (int)id), LB_GETSELITEMS, 1, (LPARAM)test))
	{
		//debug(test[0]);
		SendMessage(GetDlgItem(hWnd, (int)id), LB_DELETESTRING, (WPARAM)test[0], 0);
	}
}
*/
static void insert_attribute(ATTRIBUTE **atr, size_t size)
{
	void *tmp = NULL;
	if (tmp = realloc(*atr, size * sizeof(ATTRIBUTE)))
	{
		*atr = (ATTRIBUTE *)tmp;
	}
	else
	{
		error(L"Error allocating memory");
		exit(EXIT_SUCCESS);
	}
}

ATTRIBUTE *read_data_attributes(WCHAR *path, HWND hWnd, size_t *atr_count)
{
	FILE *fp;
	size_t attrib_idx = 0;
	ATTRIBUTE *fattrib = NULL;
	char *rbuffer = (char *)malloc(sizeof(char) * SECTOR_SIZE);
	char *tmp1 = (char *)malloc(sizeof(char) * SECTOR_SIZE);
	char *tmp2 = (char *)malloc(sizeof(char) * SECTOR_SIZE);
	if (fp = _wfopen(path, L"r"))
	{
		for (;;)
		{
			if (fgets(rbuffer, SECTOR_SIZE, fp))
			{
				if (strstr(rbuffer, "@data"))
					break;
				if (strstr(rbuffer, "@attribute"))
				{
					insert_attribute(&fattrib, attrib_idx + 1);
					custom_sscanf(rbuffer+sizeof("@attribute"), tmp1,tmp2);
					fattrib[attrib_idx].selected = FALSE;
					fattrib[attrib_idx].indx = attrib_idx;
					fattrib[attrib_idx].attrib_buffer = (char *)malloc(sizeof(char)*strlen(tmp1));
					fattrib[attrib_idx].datatype = (char *)malloc(sizeof(char)*strlen(tmp2));
					if (!fattrib[attrib_idx].attrib_buffer || !fattrib[attrib_idx].datatype)
					{
						error(L"Error allocating memory");
						exit(EXIT_SUCCESS);
					}
					add_item(hWnd, tmp1, (HMENU)IDC_LISTBOX);
					strcpy(fattrib[attrib_idx].attrib_buffer, tmp1);
					strcpy(fattrib[attrib_idx++].datatype, tmp2);
				}
			}
			else
			{
				error(L"Error reading file");
				exit(EXIT_SUCCESS);
			}
		}
		*atr_count = attrib_idx;
		fclose(fp);
	}
	else
	{
		error(L"Error opening file");
		exit(EXIT_SUCCESS);
	}
	free(rbuffer);
	free(tmp1);
	free(tmp2);
	return fattrib;
}

void update_lists(HWND hWnd, ATTRIBUTE *atrb, size_t atr_count)
{
	size_t i;
	SendMessageW(GetDlgItem(hWnd, (int)IDC_LISTBOX),
		LB_RESETCONTENT,
		(WPARAM)NULL,
		(LPARAM)NULL);
	SendMessageW(GetDlgItem(hWnd, (int)IDC_LISTBOX2),
		LB_RESETCONTENT,
		(WPARAM)NULL,
		(LPARAM)NULL);
	for (i = 0; i < atr_count; ++i)
	{
		if (atrb[i].selected)
			add_item(hWnd, atrb[i].attrib_buffer, (HMENU)IDC_LISTBOX2);
		else
			add_item(hWnd, atrb[i].attrib_buffer, (HMENU)IDC_LISTBOX);
	}
}

void insert_item(ATTRIBUTE *atrb, HWND hWnd)
{
	char tmp[SECTOR_SIZE];
	int test = 0;
	ATTRIBUTE *iterator = NULL;
	while (SendMessage(GetDlgItem(hWnd, (int)IDC_LISTBOX), LB_GETSELITEMS, 1, (LPARAM)&test))
	{
		SendMessageA(GetDlgItem(hWnd, (int)IDC_LISTBOX), LB_GETTEXT, (LPARAM)test, (WPARAM)tmp);
		iterator = atrb;
		while (strcmp(iterator[0].attrib_buffer, tmp))
			++iterator;
		iterator[0].selected = TRUE;
		add_item(hWnd, iterator[0].attrib_buffer, (HMENU)IDC_LISTBOX2);
		//debug(test[0]);
		SendMessage(GetDlgItem(hWnd, (int)IDC_LISTBOX), LB_DELETESTRING, (WPARAM)test, (LPARAM)0);
	}
}

void remove_item(ATTRIBUTE *atrb, HWND hWnd)
{
	char tmp[SECTOR_SIZE];
	int test = 0;
	size_t counter = 0;
	ATTRIBUTE *iterator = NULL;
	while (SendMessage(GetDlgItem(hWnd, (int)IDC_LISTBOX2), LB_GETSELITEMS, 1, (LPARAM)&test))
	{
		SendMessageA(GetDlgItem(hWnd, (int)IDC_LISTBOX2), LB_GETTEXT, (LPARAM)test, (WPARAM)tmp);
		iterator = atrb;
		counter = 0;
		while (strcmp(iterator[0].attrib_buffer, tmp))
		{
			if (!iterator[0].selected)
				++counter;
			++iterator;
		}
			
		iterator[0].selected = FALSE;
		//debug(counter);
		SendMessageA(GetDlgItem(hWnd, (int)IDC_LISTBOX), LB_INSERTSTRING, (WPARAM)counter, (LPARAM)tmp);
		//debug(test[0]);
		SendMessage(GetDlgItem(hWnd, (int)IDC_LISTBOX2), LB_DELETESTRING, (WPARAM)test, (LPARAM)0);
	}
}

void browser(WCHAR *path_str)
{

	LPITEMIDLIST pidl;
	BROWSEINFO bi = { 0 };
	bi.lpszTitle = L"Select arff containing folder...";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)"WHat";

	if (pidl = SHBrowseForFolder(&bi))
	{
		SHGetPathFromIDList(pidl, path_str);

		imalloc_free(pidl);

		//MessageBoxW(0, path_str, path_str, MB_OK);
	}
}

void extract_data(WCHAR * path, ATTRIBUTE * atrb, size_t atr_count)
{
	FILE *fp, *fp2;
	int tmp;
	size_t i;
	register BOOL data_written = FALSE;
	char *rbuffer = (char *)malloc(sizeof(char) * SECTOR_SIZE);
	if (fp = _wfopen(path, L"r"))
	{
		for (;;)
		{
			if (fgets(rbuffer, SECTOR_SIZE, fp))
			{
				if (strstr(rbuffer, "@data"))
				{
					fp2 = fopen("test.txt", "w");
					while (isspace(tmp = fgetc(fp)));
					fputc(tmp, fp2);
					for (i = 0; i < atr_count; ++i)
					{
						if(data_written && (atrb + i)->selected)
							fputc(',', fp2);
						while (',' != (tmp = fgetc(fp)) && '\n' != tmp)
						{
							if ((atrb + i)->selected)
							{
								fputc(tmp, fp2);
								data_written = TRUE;
							}
								
						}
					}
					fputc('\n', fp2);
					fclose(fp2);
					break;
				}
			}
			else
			{
				error(L"Error reading file");
				exit(EXIT_SUCCESS);
			}
		}
		fclose(fp);
	}
	else
	{
		error(L"Error opening file");
		exit(EXIT_SUCCESS);
	}


	free(rbuffer);
}

static void insert_file(FILE_BUFFER **atr, size_t size)
{
	void *tmp = NULL;
	if (tmp = realloc(*atr, size * sizeof(FILE_BUFFER)))
	{
		*atr = (FILE_BUFFER *)tmp;
	}
	else
	{
		error(L"Error allocating memory");
		exit(EXIT_SUCCESS);
	}
}

FILE_BUFFER *file_search(WCHAR * path_str, size_t *fcount,HWND hWnd)
{
	WIN32_FIND_DATA fd = { 0 };
	FILE_BUFFER *fbuf = NULL;
	WCHAR *dir = (WCHAR *)calloc(MAX_PATH, sizeof(WCHAR));
	WCHAR *tdir = (WCHAR *)calloc(MAX_PATH, sizeof(WCHAR));
	lstrcpy(dir, path_str);
	lstrcatW(dir, L"\\*");
	HANDLE hFind = FindFirstFile(dir, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!wcscmp(fd.cFileName, L".") || !wcscmp(fd.cFileName, L".."))
				continue;
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// Task didn't mention nested arff folders, so no action here.
			}
			else if (wcsstr(fd.cFileName, L".arff"))
			{
				lstrcpy(tdir, path_str);
				lstrcatW(tdir, L"\\");
				lstrcatW(tdir, fd.cFileName);
				insert_file(&fbuf, *fcount + 1);
				fbuf[*fcount].selected = FALSE;
				fbuf[*fcount].indx = *fcount;
				lstrcpy(fbuf[(*fcount)++].path, tdir);
				SendMessageW(GetDlgItem(hWnd, (int)IDC_FILELIST),
					LB_ADDSTRING,
					(WPARAM)NULL,
					(LPARAM)fd.cFileName);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	free(tdir);
	free(dir);
	return fbuf;
}
