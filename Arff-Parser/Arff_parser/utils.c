
#include "stdafx.h"
#include "Arff_parser.h"

#define SECTOR_SIZE 4096
#define LINE_BUFFER_SIZE 8192
#define MEGABYTE_CHUNK 1048576

typedef struct STACK_ STACK;
HANDLE START_T, PAUSE_T, RESUME_T;
volatile uintptr_t WORKING_T1, WORKING_T2, WORKING_T3, WORKING_T4, WORKING_T5;

// ATOMIC GLOBALS
volatile LONG stack_index = 0;
STACK *st;


struct STACK_
{
	EX_DATA_ARGS *data;
};


void debug(int number)
{
	WCHAR str[50];
	wsprintfW(str, L"debug: %d", number);
	MessageBoxW(NULL, str,L"Debug", MB_OK);
}

void error(WCHAR *text, int code)
{
	WCHAR num[20];
	wsprintfW(num, L"Error: %d", code);
	MessageBoxW(NULL, text, num, MB_OK);
}


void push(EX_DATA_ARGS *data)
{
	//error(data->path);
	(st + stack_index++)->data = data;
}

inline EX_DATA_ARGS *pop(void)
{
	EX_DATA_ARGS *tmp = NULL;
	LONG st_s;

	WaitForSingleObject(ghMutex, INFINITE);
	st_s = --stack_index;
	ReleaseMutex(ghMutex);

	//debug(st_s);
	tmp = ((st + st_s)->data);
	if (!tmp)
	{
		debug((int)tmp);
		debug((int)st_s);
		debug((int)st);
		exit(EXIT_SUCCESS);
	}
	
	return tmp;
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
		error(L"Error allocating memory",122);
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
	if (!rbuffer || !tmp1 || !tmp2)
	{
		error(L"Error allocating memory", 134);
		exit(EXIT_SUCCESS);
	}
	if (fp = _wfopen(path, L"r"))
	{
		if (fgets(rbuffer, SECTOR_SIZE, fp))
		{
			strcpy(relationHeader, rbuffer);
		}
		else
		{
			error(L"Error reading file",143);
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
					
					fattrib[attrib_idx].attrib_buffer = (char *)malloc(sizeof(char)*strlen(tmp1) + 1);
					fattrib[attrib_idx].datatype = (char *)malloc(sizeof(char)*strlen(tmp2) + 1);
					if (!fattrib[attrib_idx].attrib_buffer || !fattrib[attrib_idx].datatype)
					{
						error(L"Error allocating memory",162);
						exit(EXIT_SUCCESS);
					}
					add_item(hWnd, tmp1, (HMENU)IDC_LISTBOX);
					strcpy(fattrib[attrib_idx].attrib_buffer, tmp1);
					strcpy(fattrib[attrib_idx++].datatype, tmp2);
					//attrib_idx++;
				}
			}
			else
			{
				error(L"Error reading file",172);
				exit(EXIT_SUCCESS);
			}
		}
		*atr_count = attrib_idx;
		fclose(fp);
	}
	else
	{
		error(L"Error opening file",181);
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

BOOL browser(WCHAR *path_str)
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
		return TRUE;
	}
	return FALSE;
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

void extract_data(const WCHAR * path, ATTRIBUTE * atrb, const size_t atr_count, char **out, size_t *out_s)
{
	FILE *fp = NULL;
	char *iter = *out;
	char *btmp = NULL;
	char *r_ptr = NULL;
	int tmp;
	size_t i, tmp_s;
	size_t buf_s = LINE_BUFFER_SIZE;
	register BOOL data_written = FALSE;
	//OutputDebugStringW(L"Pre file malloc\n");
	char *rbuffer = (char *)malloc(sizeof(char) * MEGABYTE_CHUNK);
	if (!rbuffer)
	{
		error(L"1 MB memory allocation failed",309);
		exit(EXIT_SUCCESS);
	}
	//OutputDebugStringW(L"Post file malloc\n");
	if (fp = _wfopen(path, L"r"))
	{
		for (;;)
		{
			if (fgets(rbuffer, SECTOR_SIZE, fp))
			{
				if (strstr(rbuffer, "@data"))
				{
					if (MEGABYTE_CHUNK == fread(rbuffer, sizeof(char), MEGABYTE_CHUNK, fp))
					{
						// TODO: IMPLEMENT 10 MB buffer extending
						error(L"File is bigger than 1 MB",324);
						exit(EXIT_SUCCESS);
					}
					//OutputDebugStringW(L"Closing file handle\n");
					fclose(fp);

					r_ptr = rbuffer;
					while (isspace(tmp = *r_ptr++));
					if ((atrb + 0)->selected)
						*iter++ = (char)tmp;
					if ((size_t)(iter - *out) == buf_s)
					{
						tmp_s = buf_s;
						if (btmp = (char *)realloc(*out, buf_s += LINE_BUFFER_SIZE))
						{
							*out = btmp;
							iter = *out + tmp_s;
						}
						else
						{
							error(L"extr data realloc failed",344);
							exit(EXIT_SUCCESS);
						}
					}
					//fputc(tmp, fp2);
					for (i = 0; i < atr_count; ++i)
					{
						if (data_written && (atrb + i)->selected)
							*iter++ = (char)tmp;
						if ((size_t)(iter - *out) == buf_s)
						{
							tmp_s = buf_s;
							if (btmp = (char *)realloc(*out, buf_s += LINE_BUFFER_SIZE))
							{
								*out = btmp;
								iter = *out + tmp_s;
							}
							else
							{
								error(L"extr data realloc failed",363);
								exit(EXIT_SUCCESS);
							}
						}
						while (',' != (tmp = *r_ptr++) && '\n' != tmp) // TODO: READ STRING AND DATA FORMATS
						{
							if ((atrb + i)->selected)
							{
								*iter++ = (char)tmp;
								if ((size_t)(iter - *out) == buf_s)
								{
									tmp_s = buf_s;
									if (btmp = (char *)realloc(*out, buf_s += LINE_BUFFER_SIZE))
									{
										*out = btmp;
										iter = *out + tmp_s;
									}
									else
									{
										error(L"extr data realloc failed",382);
										exit(EXIT_SUCCESS);
									}
								}
								data_written = TRUE;
							}

						}
					}
					if ((size_t)(iter - *out) >= buf_s - 4)
					{
						tmp_s = (size_t)(iter - *out);
						if (btmp = (char *)realloc(*out, buf_s += 4))
						{
							*out = btmp;
							iter = *out + tmp_s;
						}
						else
						{
							error(L"extr data realloc failed",401);
							exit(EXIT_SUCCESS);
						}
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
				error(L"Error reading file",414);
				exit(EXIT_SUCCESS);
			}
		}
		
	}
	else
	{
		//error(L"Error opening file: 345");
		error(L"Corrupted path",423);
		debug((int)path);
		exit(EXIT_SUCCESS);
	}
	//OutputDebugStringW(L"Freeing rbuffer\n");
	if(rbuffer)
		free(rbuffer);
	else
		OutputDebugStringW(L"local buffer deallocation fail\n");
}

unsigned int __stdcall fread_thread(void * data)
{
	EX_DATA_ARGS *arg;
	for (;;) // TODO: Suspending status check
	{
		if (InterlockedCompareExchange(&stack_index, stack_index, stack_index))
		{
			//OutputDebugStringW(L"Pre pop\n");
			arg = pop();
			//OutputDebugStringW(L"post pop\n");
			//ReleaseMutex(ghMutex);
			extract_data(arg->path, arg->atrb, arg->atr_count, &arg->output, &arg->ots);
			//WaitForSingleObject(ghMutex, INFINITE);
			SendMessage(arg->sbar_handle, PBM_STEPIT, 0, 0);
			//ReleaseMutex(ghMutex);

		}
		else
		{
			break;
		}
		if (InterlockedCompareExchange(&T_STOP, T_STOP, T_STOP))
		{
			while (InterlockedCompareExchange(&T_STOP, T_STOP, T_STOP))
				Sleep(200);
		}
		else if (InterlockedCompareExchange(&T_ABORT, T_ABORT, T_ABORT))
		{
			return 0;
		}
	}
	//EX_DATA_ARGS *arg = (EX_DATA_ARGS *)data;
	//extract_data(arg->path, arg->atrb, arg->atr_count, arg->loc, arg->out_b_size);
	//MessageBoxA(NULL, arg->output,"test",MB_OK);
	//OutputDebugStringW(L"i finished t\n");
	//Sleep(10000);
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
		error(L"Error allocating memory",477);
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

void first_file(WCHAR *pstr)
{
	WIN32_FIND_DATA fd = { 0 };
	FILE_BUFFER *fbuf = NULL;
	WCHAR *dir = (WCHAR *)calloc(MAX_PATH, sizeof(WCHAR));
	lstrcpy(dir, pstr);
	lstrcatW(dir, L"\\*");
	HANDLE hFind = FindFirstFile(dir, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!wcscmp(fd.cFileName, L".") || !wcscmp(fd.cFileName, L"..") || fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;
			else if (wcsstr(fd.cFileName, L".arff"))
			{
				//lstrcpy(dir, path_str);
				lstrcatW(pstr, L"\\");
				lstrcatW(pstr, fd.cFileName);
				break;
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	free(dir);
}

FILE_BUFFER *file_search(WCHAR * path_str, size_t *fcount, HWND hWnd)
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

void read_files(const FILE_BUFFER * files, const size_t f_count, ATTRIBUTE * atrb,
	const size_t atr_count, HWND testbar) // TODO: FIX selected attributes count
{
	EX_DATA_ARGS *exdta = NULL;
	size_t i, j;
	FILE *fp;
	DWORD t_count = 0;
	
	stack_index = 0;
	st = NULL;
	if (!(fp = fopen("test.txt", "w")))
	{
		error(L"Error opening output file", 570);
		exit(EXIT_SUCCESS);
	}
	for (i = 0; i < f_count; ++i)
	{
		if ((files + i)->selected)
		{
			++t_count;
		}
	}
	//debug(t_count);
	exdta = (EX_DATA_ARGS *)calloc(t_count, sizeof(EX_DATA_ARGS));
	SendMessage(testbar, PBM_SETRANGE, 0, MAKELPARAM(0, t_count));
	st = (STACK *)malloc((t_count+1) * sizeof(STACK));

	

	make_header(fp, atrb,  atr_count);
	
	for (i = 0, j = 0; i < f_count; ++i)
	{
		if ((files + i)->selected)
		{
			
			exdta[j].atrb = atrb;
			exdta[j].atr_count = atr_count;
			exdta[j].sbar_handle = testbar;
			exdta[j].path = (files + i)->path;
			if (!(exdta[j].output = (char *)malloc(sizeof(char) * LINE_BUFFER_SIZE)))
			{
				error(L"malloc fail",599);
				exit(EXIT_SUCCESS);
			}
			//WaitForSingleObject(ghMutex, INFINITE);
			push(&exdta[j]);
			//ReleaseMutex(ghMutex);
			
			++j;
			//extract_data((files + i)->path, atrb, atr_count, fp);
		}
	}
	//WaitForSingleObject(ghMutex, INFINITE);
	//FINISHED = TRUE;
	WORKING_T1 = (uintptr_t)_beginthreadex(0, 0, &fread_thread, 0, 0, 0);
	WORKING_T2 = (uintptr_t)_beginthreadex(0, 0, &fread_thread, 0, 0, 0);
	WORKING_T3 = (uintptr_t)_beginthreadex(0, 0, &fread_thread, 0, 0, 0);
	WORKING_T4 = (uintptr_t)_beginthreadex(0, 0, &fread_thread, 0, 0, 0);
	WORKING_T5 = (uintptr_t)_beginthreadex(0, 0, &fread_thread, 0, 0, 0);
	WaitForSingleObject((HANDLE)WORKING_T1, INFINITE);
	CloseHandle((HANDLE)WORKING_T1);
	WaitForSingleObject((HANDLE)WORKING_T2, INFINITE);
	CloseHandle((HANDLE)WORKING_T2);
	WaitForSingleObject((HANDLE)WORKING_T3, INFINITE);
	CloseHandle((HANDLE)WORKING_T3);
	WaitForSingleObject((HANDLE)WORKING_T4, INFINITE);
	CloseHandle((HANDLE)WORKING_T4);
	WaitForSingleObject((HANDLE)WORKING_T5, INFINITE);
	CloseHandle((HANDLE)WORKING_T5);
	//_endthreadex(WORKING_T1);
	//_endthreadex(WORKING_T2);
	//_endthreadex(WORKING_T3);
	//ReleaseMutex(ghMutex);
	//debug(WaitForMultipleObjects(t_spawned, t_handles, TRUE, INFINITE));
/*	
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
	error(buf);
*/
	/*
	for (j = 0; j < t_spawned; ++j)
		CloseHandle(t_handles + j);
		*/
	//WaitForSingleObject(t_handle, INFINITE);
	/*
	if(t_count <= MAXIMUM_WAIT_OBJECTS)
		debug(WaitForMultipleObjects(t_count, t_handles, TRUE, INFINITE));
	else
	{
		for (i = MAXIMUM_WAIT_OBJECTS, j = 0; i < t_count; i += MAXIMUM_WAIT_OBJECTS, j+= MAXIMUM_WAIT_OBJECTS)
		{
			WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS, t_handles + j, TRUE, INFINITE);
		}
		WaitForMultipleObjects(t_count-i-MAXIMUM_WAIT_OBJECTS, t_handles + j- MAXIMUM_WAIT_OBJECTS, TRUE, INFINITE);
	}
	*/
			
	//debug(WAIT_OBJECT_0);
	//wait_multiple_kernel_objects(t_handles, j); 
	//Wait(t_count, t_handles, TRUE, INFINITE);
	if (!InterlockedCompareExchange(&T_ABORT, T_ABORT, T_ABORT))
		for (i = 0; i < t_count; ++i)
		{
			//MessageBoxA(NULL, exdta[i].output, "test", MB_OK);
			fprintf(fp,"%s",exdta[i].output);
			free(exdta[i].output);
		
		}

	if (exdta) free(exdta);
	else error(L"exdta free fail",670);
	if(st) free(st);
	else error(L"st free fail",676);
	if(fp) fclose(fp);
	else error(L"fp close fail",680);

	SendMessage(testbar, PBM_SETPOS, 0, 0);
	T_DONE = TRUE;
	MessageBoxA(NULL, "Task Finished", "Info", MB_OK);
	//OutputDebugStringW(L"Exit\n");
}
