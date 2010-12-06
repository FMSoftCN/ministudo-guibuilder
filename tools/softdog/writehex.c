// This is a Demo for EliteIV in Linux.

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "sense4.h"
#include "psense4.h"

int main(int argc, char** argv)
{
	SENSE4_CONTEXT ctx = {0};
	SENSE4_CONTEXT *pctx = NULL;
	unsigned long size = 0;
	unsigned long ret = 0;
	unsigned long FileSize=0;
	unsigned long WByte=0;
//	unsigned long len = 0;

	if (argc != 5)
	{
		fprintf(stderr, "%s format: %s hexfile hexfileid keyfile keyfileid\n", argv[0], argv[0]);
		return 1;
	}
	
	S4Enum(pctx, &size);
	if (size == 0)
	{
		printf("EliteIV not found!\n");
		return 1;
	}
	
	pctx = (SENSE4_CONTEXT *)malloc(size);
	if (pctx == NULL)
	{
		printf("Not enough memory!\n");
		return 1;
	}
	
	ret = S4Enum(pctx, &size);
	if (ret != S4_SUCCESS)
	{
		printf("Enumerate EliteIV error!\n");
		free(pctx);
		return 1;
	}
	
	memcpy(&ctx, pctx, sizeof(SENSE4_CONTEXT));
	free(pctx);
	pctx = NULL;
	
	ret = S4Open(&ctx);
	if (ret != S4_SUCCESS)
	{
		printf("Open EliteIV failed!\n");
		return 1;
	}
	
	ret = S4ChangeDir(&ctx, "\\");
	if (ret != S4_SUCCESS)
	{
		printf("No root directory found!\n");
		S4Close(&ctx);
		return 1;
	}
	ret = S4VerifyPin(&ctx, "123456781234567812345678", 24, S4_DEV_PIN);
	if (ret != S4_SUCCESS)
	{
		printf("Verify dev PIN failed!\n");
		S4Close(&ctx);
		return 1;
	}

	ret = S4EraseDir(&ctx, NULL);
	if (ret != S4_SUCCESS)
	{
		printf("EraseDir Failed\n");
		S4Close(&ctx);
		return 1;
	}

	S4CREATEDIRINFO info = {0};

	info.dwS4CreateDirInfoSize = sizeof(S4CREATEDIRINFO);
	memcpy(info.szAtr, "FMSoft", 8);

	ret = S4CreateDirEx(&ctx, "\\", 0, S4_CREATE_ROOT_DIR, &info);
	if (ret != S4_SUCCESS)
	{
		printf("EraseDir Failed\n");
		S4Close(&ctx);
		return 1;
	}

	ret = S4ChangeDir(&ctx, "\\");
	if (ret != S4_SUCCESS)
	{
		printf("No root directory found!\n");
		S4Close(&ctx);
		return 1;
	}
	
	ret = S4VerifyPin(&ctx, "123456781234567812345678", 24, S4_DEV_PIN);
	if (ret != S4_SUCCESS)
	{
		printf("Verify dev PIN failed!\n");
		S4Close(&ctx);
		return 1;
	}

	fprintf(stderr, "file name %s, fileid %s\n", argv[1], argv[2]);
	//ret = PS4WriteFile(&ctx, argv[2], argv[1], &FileSize, S4_CREATE_NEW, S4_HEX_FILE, &WByte);
	ret = PS4WriteFile(&ctx, argv[2], argv[1], &FileSize, S4_CREATE_NEW | S4_FILE_EXECUTE_ONLY, S4_EXE_FILE, &WByte);
	if( ret != S4_SUCCESS)
	{
		printf("Write HexFile failed!code=%x\n",ret);
		S4Close(&ctx);
		return 1;
	}	

	ret = PS4WriteFile(&ctx, argv[4], argv[3], &FileSize, S4_CREATE_NEW, S4_DATA_FILE, &WByte);
	if( ret != S4_SUCCESS)
	{
		printf("Write KeyFile failed!code=%x\n",ret);
		S4Close(&ctx);
		return 1;
	}	

	S4Close(&ctx);
	
	return 0;
}
