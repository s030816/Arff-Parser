
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
static void insert_attribute(ATTRIBUTE **atr, const size_t size)
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
		if (fgets(rbuffer, SECTOR_SIZE, fp))
		{
			strcpy(relationHeader, rbuffer);
		}
		else
		{
			error(L"Error reading file");
			exit(EXIT_SUCCESS);
		}
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
/*
void update_lists(HWND hWnd, ATTRIBUTE *atrb, const size_t atr_count)
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
*/
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

void imalloc_free(LPITEMIDLIST pidl)
{
	IMalloc * imalloc = 0;
	if (SUCCEEDED(SHGetMalloc(&imalloc)))
	{
		imalloc->lpVtbl->Free(imalloc, pidl);
		imalloc->lpVtbl->Release(imalloc);
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
	}
}

void make_header(FILE *fp, ATTRIBUTE * atrb, const size_t atr_count)
{
	size_t i;
	fprintf(fp, "%s\n", relationHeader);
	for (i = 0; i < atr_count; ++i)
	{
		if ((atrb + i)->selected)
		{
			fprintf(fp, "@attribute %s %s", (atrb + i)->attrib_buffer, (atrb + i)->datatype);
		}
	}
	fprintf(fp, "%s", "\n@data\n");
}

void extract_data(WCHAR * path, ATTRIBUTE * atrb, const size_t atr_count, char **out, size_t *out_s)
{
	FILE *fp;
	char *iter = *out;
	char *btmp;
	int tmp;
	size_t i;
	size_t buf_s = 4096;
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
					while (isspace(tmp = fgetc(fp)));
					if ((atrb + 0)->selected)
						*iter++ = (char)tmp;
					if ((size_t)(iter - *out) == buf_s)
					{
						if (btmp = (char *)realloc(*out, buf_s += 4096))
							*out = btmp;
						else
						{
							error(L"extr data realloc failed");
							exit(EXIT_SUCCESS);
						}
					}
						//fputc(tmp, fp2);
					for (i = 0; i < atr_count; ++i)
					{
						if(data_written && (atrb + i)->selected)
							*iter++ = (char)tmp;
						if ((size_t)(iter - *out) == buf_s)
						{
							if (btmp = (char *)realloc(*out, buf_s += 4096))
								*out = btmp;
							else
							{
								error(L"extr data realloc failed");
								exit(EXIT_SUCCESS);
							}
						}
						while (',' != (tmp = fgetc(fp)) && '\n' != tmp) // TODO: READ STRING AND DATA FORMATS
						{
							if ((atrb + i)->selected)
							{
								*iter++ = (char)tmp;
								if ((size_t)(iter - *out) == buf_s)
								{
									if (btmp = (char *)realloc(*out, buf_s += 4096))
										*out = btmp;
									else
									{
										error(L"extr data realloc failed");
										exit(EXIT_SUCCESS);
									}
								}
								data_written = TRUE;
							}
								
						}
					}
					if (btmp = (char *)realloc(*out, buf_s += 2))
						*out = btmp;
					else
					{
						error(L"extr data realloc failed");
						exit(EXIT_SUCCESS);
					}
					*iter++ = '\n';
					*iter = '\0';
					*out_s = buf_s;
					//fputc('\n', fp2);
					break;
				}
			}
			else
			{
				error(L"Error reading file");
				exit(EXIT_SUCCESS);
			}
		}
	}
	else
	{
		error(L"Error opening file");
		exit(EXIT_SUCCESS);
	}


	free(rbuffer);
}

unsigned int __stdcall fread_thread(void * data)
{
	EX_DATA_ARGS *arg = (EX_DATA_ARGS *)data;
	extract_data(arg->path, arg->atrb, arg->atr_count, arg->loc, arg->out_b_size);
	//MessageBoxA(NULL, arg->output,"test",MB_OK);
	WaitForSingleObject(ghMutex, INFINITE);
	SendMessage(arg->sbar_handle, PBM_STEPIT, 0, 0);
	ReleaseMutex(ghMutex);
	return 0;
}

static void insert_file(FILE_BUFFER **atr, const size_t size)
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

void update_selected_file_list(FILE_BUFFER *fbuf, HWND hWnd)
{
	int test = 0;
	while (SendMessage(GetDlgItem(hWnd, (int)IDC_FILELIST), LB_GETSELITEMS, 1, (LPARAM)&test))
	{
		SendMessage(GetDlgItem(hWnd, (int)IDC_FILELIST), LB_SETSEL, (WPARAM)FALSE, (LPARAM)test);
		fbuf[test].selected = TRUE;
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

void read_files(FILE_BUFFER * files, const size_t f_count, ATTRIBUTE * atrb,
	const size_t atr_count, HWND testbar) // TODO: FIX selected attributes count
{
	EX_DATA_ARGS *exdta, *iter;
	size_t i, j;
	FILE *fp;
	HANDLE *t_handles = NULL;
	DWORD t_count = 0;
	
	if (!(fp = fopen("test.txt", "w")))
	{
		error(L"Error opening file");
		exit(EXIT_SUCCESS);
	}
	for (i = 0; i < f_count; ++i)
	{
		if ((files + i)->selected)
		{
			++t_count;
		}
	}
	exdta = (EX_DATA_ARGS *)calloc(t_count, sizeof(EX_DATA_ARGS));
	iter = exdta;
	SendMessage(testbar, PBM_SETRANGE, 0, MAKELPARAM(0, t_count));
	t_handles = (HANDLE *)calloc(t_count, sizeof(HANDLE));
	make_header(fp, atrb,  atr_count);
	for (i = 0, j = 0; i < f_count; ++i)
	{
		if ((files + i)->selected)
		{
			if (!(j % 10))
			{
				WaitForMultipleObjects(10, iter, TRUE, INFINITE);
				iter += 10;
			}
			
			exdta[j].atrb = atrb;
			exdta[j].atr_count = atr_count;
			exdta[j].sbar_handle = testbar;
			exdta[j].path = (files + i)->path;
			exdta[j].output = (char *)malloc(sizeof(char) * 4096);
			exdta[j].loc = &exdta[j].output;
			exdta[j].out_b_size = &exdta[j].ots;
			t_handles[j] = (HANDLE)_beginthreadex(0, 0, &fread_thread, &exdta[j], 0, 0);
			++j;
			//extract_data((files + i)->path, atrb, atr_count, fp);
		}
	}
	//WaitForSingleObject(t_handle, INFINITE);
	while(!WaitForMultipleObjects(t_count, t_handles, TRUE, INFINITE));
	for (i = 0; i < t_count; ++i)
	{
		//MessageBoxA(NULL, exdta[i].output, "test", MB_OK);
		fprintf(fp,"%s",exdta[i].output);
		free(exdta[i].output);
	}
	free(exdta);
	free(t_handles);
	fclose(fp);
	MessageBoxA(NULL, "Finish", "test", MB_OK);
}
