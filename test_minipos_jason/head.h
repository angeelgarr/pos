#ifndef HEAD_H
#define HEAD_H


struct productList
{
	double price;
	int  productId;
	char varPriceFlag;
	char *productName;
};



typedef struct _folder
{
	int isFolder;
	int isProduct;
	char *folderName;
	struct productList *product;
	struct _folder *next;  //next to folderArray
}folderArray[6];


#endif
