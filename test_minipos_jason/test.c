

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include "fcntl.h"
#include "head.h"
#include <assert.h>

struct _folder *glHeadFolder[6];
int glHeadFolderLen;

void print_json(json_t *root,struct _folder *folder);
void print_json_aux(json_t *element,struct _folder *folder);
void print_json_object(json_t *element,struct _folder *folder);
void print_json_array(json_t *element,struct _folder *folder);


void print_json(json_t *root,struct _folder *folder) {
    print_json_aux(root,folder);
}

char *Scope(char *str)
{
	int iLen = strlen(str);
	return (strcpy(malloc(iLen + 1),str));
}

int IsEqualStr(const char *str1,const char *str2)
{
	if(strcmp(str1,str2) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
	return 0;
}

void print_json_aux(json_t *element,struct _folder *folder) {
    switch (json_typeof(element)) {
    case JSON_OBJECT:
        print_json_object(element,folder);
        break;
    case JSON_ARRAY:
        print_json_array(element,folder);
        break;
    case JSON_STRING:
    case JSON_INTEGER:
    case JSON_REAL:
    case JSON_TRUE:
    case JSON_FALSE:
    case JSON_NULL:
        break;
    default:
        fprintf(stderr, "unrecognized JSON type %d\n", json_typeof(element));
    }
}


void print_json_object(json_t *element,struct _folder *folder) {
    size_t size;
    const char *key;
    json_t *value;
    int isFolderFlag = 0;
    int isProductFlag = 0;

    size = json_object_size(element);
    json_object_foreach(element, key, value) {
       /* printf("JSON Key: \"%s\"\n", key);
        if(json_typeof(value) == JSON_STRING)
        {
        	printf("string=%s\n",json_string_value(value));
        }*/
    	if(IsEqualStr(key,"type") && IsEqualStr(json_string_value(value),"folder"))
    	{
    		folder->isFolder = 1;
    		isFolderFlag = 1;
    	}
    	else if(IsEqualStr(key,"type") && IsEqualStr(json_string_value(value),"product"))
		{
			folder->isProduct = 1;
			isProductFlag = 1;
		}
    	if(isFolderFlag == 1 && IsEqualStr(key,"name"))
    	{
    		folder->folderName = Scope(json_string_value(value));
    		printf("folder->folderName=%s\n",folder->folderName);
    		//isFolderFlag = 0;
    	}
    	else if(isProductFlag == 1 && IsEqualStr(key,"name"))
		{
    		folder->product = (struct productList*)malloc(sizeof(struct productList));
			folder->product->productName = Scope(json_string_value(value));
			printf("folder->product->productName=%s\n",folder->product->productName);
			//isProductFlag = 0;
		}
    	if(isProductFlag == 1 && IsEqualStr(key,"price"))
    	{
    		folder->product->price = json_integer_value(value);
    		printf("folder->product->price=%f\n",folder->product->price);
    	}
    /*	if(isProductFlag == 1 && IsEqualStr(key,"price"))
		{
			folder->product->price = json_integer_value(value);
			printf("folder->product->price=%f\n",folder->product->price);
		}*/
        print_json_aux(value,folder);
    }

}

void print_json_array(json_t *element,struct _folder *folder) {
    size_t i;
    size_t size = json_array_size(element);
    folderArray *folderArryType =  (folderArray *)malloc(sizeof(folderArray));
    memset(folderArryType,0,sizeof(folderArryType));
    if(folder == NULL)
    {
    	printf("folder==NULL,Line=%d\n",__LINE__);
    	for(i = 0;i < size;i++)
    	{
    		glHeadFolder[i] = &folderArryType[0][i];
    	}
    	glHeadFolderLen = size;
    	folder = &folderArryType[0];
    }
    else
    {
    	printf("folder==%d,Line=%d\n",folder,__LINE__);
    	folder->next = &folderArryType[0];
    	folder = folder->next;
    }
    for (i = 0; i < size; i++) {

        print_json_aux(json_array_get(element, i),folder+i);
    }
}


json_t *load_json(const char *text) {
    json_t *root;
    json_error_t error;

    root = json_loads(text, 0, &error);

    if (root) {
        return root;
    } else {
        fprintf(stderr, "json error on line %d: %s\n", error.line, error.text);
        return (json_t *)0;
    }
}

//for test generated unpacked data
void testJason(void)
{
	int i;
	int j;

	printf("glHeadFolderLen=%d\n",glHeadFolderLen);
	for(i = 0;i < glHeadFolderLen;i++)
	{
		if(glHeadFolder[i]->isFolder)
		{
			printf("glHeadFolder[%d].folderName=%s\n",i,glHeadFolder[i]->folderName);
			if(glHeadFolder[i]->next != NULL)
			{
				for(j = 0;j < 7;j++)
				{
					if(glHeadFolder[i]->next[j].isFolder)
					{
						printf("glHeadFolder[%d]->next[%d].FolderName=%s\n",
								i,j,glHeadFolder[i]->next[j].folderName);
					}
					else if (glHeadFolder[i]->next[j].isProduct)
					{
						printf("glHeadFolder[%d]->next[%d].product->price=%f\n",
						i,j,glHeadFolder[i]->next[j].product->price);
					}
				}
			}
		}
		else if(glHeadFolder[i]->isProduct)
		{
			printf("glHeadFolder[%d].productName=%s\n",i,glHeadFolder[i]->product->productName);
		}
	}
}


#define MAX_CHARS 4096

int main(int argc, char *argv[]) {
	size_t iFd;
	size_t i;

	i = 0;
    char str[MAX_CHARS];
	iFd = open("read.txt",O_RDONLY);
	assert(iFd >= 0);
	while(1)
	{
		if(read(iFd,&str[i++],1) == 0)
		{
			break;
		}
	}
	str[i] = '\0';
	json_t *root = load_json(str);
	if (root) {
		/* print and release the JSON structure */
		print_json(root,NULL);
		json_decref(root);
	}
	printf("\n\n\n\nstart to test json\n");
	testJason();
    return 0;
}
