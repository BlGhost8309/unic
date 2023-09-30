// unic.cpp : Defines the entry point for the console application.
//


/***************************************************************************//**
* Уникализация bmp файлов. Задумка следующая: кажды пиксель меняется на один
* из соседних цветов рандомно. Это должно уникализировать картинку для поисковых
* систем. Но пока это не работает... Поисковики палят эту тему.)) Работает только,
*  если ее изменить до неузноваемости... Пока оставим, пока не придет в голову 
*  что-нибудь поумнее. 8 лет прошло, а ничего так и не сделал!))))))))))))
* А вот ещё одна проверка  бла бла бла
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <math.h>

#define _CRT_SECURE_NO_WARNINGS



#define INT32	int
#define UINT32	unsigned int
#define UINT16	unsigned short int
#define UINT8	unsigned char
#define ERROR	(-1)
#define OK		(0)
#define	STATUS INT32

#define PLUS	(1)
#define MINUS	(0)





/******************************************************************************/

/* Макс. размер поддерживаемого нами изображения в BMP. */

#define MAX_BMP_W   1280
#define MAX_BMP_H   1280

#define MAX_BUF_SIZE	0xA00000

/* Типы данных в BMP-файле. */

#define BI_RGB          0
#define BI_RLE8         1
#define BI_RLE4         2
#define BI_BITFIELDS    3


#define MASK_FOR_24_BITS(val)	(val | 0x000001))	
#define MASK_FOR_16_BITS(val)	(val | 0x0100))	



typedef struct
{
	UINT32 magic;
	UINT8  type;
	UINT16 w;
	UINT16 h;

} IMGBUF_HEADER;

/** Заголовок BMP-файла (без первых двух байтов "BM"). */

typedef struct
{
	UINT32 bfSize;
	UINT16 bfReserved1;
	UINT16 bfReserved2;
	UINT32 bfOffBits;

} BMP_HEADER;

/** Структура с информацией о изображении в BMP-файле. */

typedef struct
{
	UINT32 biSize;
	UINT32 biWidth;
	UINT32 biHeight;
	UINT16 biPlanes;
	UINT16 biBitCount;
	UINT32 biCompression;
	UINT32 biSizeImage;
	UINT32 biXPelsPerMeter;
	UINT32 biYPelsPerMeter;
	UINT32 biClrUsed;
	UINT32 biClrImportant;

} BMP_INFO;


char buf[MAX_BUF_SIZE];


STATUS imageUnicFile(const char *filename);


/***************************************************************************//**
* Переворот байтов в 16-ти битном слове.
*
* \param value Входное значение.
* \return Входное значение, байты которого переставлены в обратном порядке.
*
*******************************************************************************/

UINT16 knsSwap16(UINT16 value)
{
    UINT16 result = (((value) & 0xFF) << 8) | (((value) & 0xFF00) >> 8);

    return result;
}

/***************************************************************************//**
* Переворот байтов в 16-ти битном слове.
*
* \param value Входное значение.
* \return Входное значение, байты которого переставлены в обратном порядке.
*
*******************************************************************************/

UINT16 knsSwap32(UINT32 value)
{
    UINT16 result = (((value & 0xFF) << 24) | ((value & 0xFF00) << 8) | ((value & 0xFF0000) >> 8) | ((value & 0xFF) >> 24));

    return result;
}





UINT32 knImageReadUINT32(FILE *f)
{
	/* TODO for LE. */

	UINT32 val = 0;

	/*if*/ fread(&val, sizeof(UINT32), 1, f);
		/*val = knsSwap32(val);*/

	return val;
}

/******************************************************************************/

UINT16 knImageReadUINT16(FILE *f)
{
	/* TODO for LE. */

	UINT16 val = 0;

	/*if*/ fread(&val, sizeof(UINT16), 1, f);

	return val;
}




int main(int argc, char **argv)
{	


	STATUS res;

	char	fileName[128];

	if(argc > 2)
	{
		printf("\r\n Too much arguments");
		return 0;
	}

	if(argc < 2)
	{
		printf("\r\n Too low arguments");
		return 0;
	}

	strcpy(fileName, argv[1]);
	

	
	imageUnicFile(fileName);

	return OK;
}


STATUS imageUnicFile(const char *filename)
{
	char	*ptrBuf;
	const char imageFuncName[] = "imageUnicFile():";
	char   id[2],
	outFileName[128];

	INT32 rndColor, sign;

	UINT32 numBytes, pos, fileSize, offset, i, imgDataOffset, size;
	UINT16 biBitCount;
	FILE *f, *fOut;

	BMP_HEADER hdr;
	BMP_INFO info;

	numBytes = 0;
	pos = 0;
	fileSize = 0;
	ptrBuf = 0;
	offset = 0;
	rndColor = 0;
	sign = 0;
	
	
	printf("\r\n  filename %s", filename);

	if (!filename)
	{
		/** Неверные параметры или устройство не открыто. */
		printf("%s ERROR. File name have been not recieved \"%s\"!\r\n", imageFuncName);
		return ERROR;
	}
	
	/** Откроем файл. */
	f = fopen(filename, "rb");
	if (!f)
	{
		/** Не открылся файл. */
		printf("%s Error opening file \"%s\"!\r\n", imageFuncName, filename);
		return ERROR;
	}

	
	
	
     /** Прочитаем первые два байта из файла и решим, в каком он формате. */
    fseek(f, 0, SEEK_SET);
    fread(id, 1, 2, f);
    fseek(f, 0, SEEK_SET);


	/* Проверим их */
	if (('B' == id[0]) && ('M' == id[1]))
	{
		/* Файл идентифицирован как BMP. */

		 knImageReadUINT16(f); /* "BM" */

    /* Читаем заголовок файла. */
		hdr.bfSize      = knImageReadUINT32(f);
		hdr.bfReserved1 = knImageReadUINT16(f);
		hdr.bfReserved2 = knImageReadUINT16(f);
		hdr.bfOffBits   = knImageReadUINT32(f);
		info.biSize = knImageReadUINT32(f);
		info.biWidth = knImageReadUINT32(f);
		info.biHeight = knImageReadUINT32(f);
		info.biPlanes = knImageReadUINT16(f);
		info.biBitCount = knImageReadUINT16(f);
		info.biCompression = knImageReadUINT32(f);
		info.biSizeImage = knImageReadUINT32(f);
		info.biXPelsPerMeter = knImageReadUINT32(f);
		info.biYPelsPerMeter = knImageReadUINT32(f);
		info.biClrUsed = knImageReadUINT32(f);
		info.biClrImportant = knImageReadUINT32(f);

		
		/* Встанем на начало */
		fseek(f, 0, SEEK_SET);


		printf("\r\n  hdr.bfSize; %04X ", hdr.bfSize);
		printf("\r\n  hdr.bfReserved1; %X ", hdr.bfReserved1);
		printf("\r\n  hdr.bfReserved2 %X ", info.biHeight);
		printf("\r\n  biPlanes; %X ",hdr.bfOffBits );
		printf("\r\n  ======================  " );
		printf("\r\n  biSize; %X ", info.biSize);
		printf("\r\n  biWidth; %X ", info.biWidth);
		printf("\r\n  biHeight; %X ", info.biHeight);
		printf("\r\n  biPlanes; %X ",info.biPlanes );
		printf("\r\n  biBitCount; %X ", info.biBitCount);
		printf("\r\n  biCompression; %X ", info.biCompression);
		printf("\r\n  biSizeImage; %X ", info.biSizeImage);
		printf("\r\n  biXPelsPerMeter; %X ", info.biXPelsPerMeter);
		printf("\r\n  biYPelsPerMeter; %X ", info.biYPelsPerMeter);
		printf("\r\n  biClrUsed; %X ", info.biClrUsed);
		printf("\r\n  biClrImportant; %X ", info.biClrImportant);

		/* Сохраним заголовок */
		/*memcpy((UINT8 *)&hdr, ptrTmp(sizeof(BMP_HEADER)));*/

		while (numBytes = fread(&buf[pos], 1, 1, f))
		{
			++pos;
			fileSize += numBytes;
		}
		printf("\r\n file size is %d", fileSize);
		fclose(f);

	

	}
	else
	{
		/* Формат файла не ясен. */
		printf("%s Can't load \"%s\". Unknown format.\r\n", imageFuncName, filename);
		fclose(f);
	
	}

	
	
	/* Теперь пробежимся по куску с данными и наложим маску. */

	pos = 0;

	/* imgDataOffset - смещение начала данных изображения в файле. */
    imgDataOffset = 2 + sizeof(BMP_HEADER) + info.biSize;

	printf("\r\n info.biSize %X", info.biSize);

	/*info.biBitCount = ((info.biBitCount & 0xFF00) >> 8);*/
	printf("\r\n info.biBitCount %d", info.biBitCount);
	
	offset = (info.biBitCount / 8);

	/* Встанем на начало данных. */

	ptrBuf = buf + imgDataOffset;
	size = imgDataOffset;

	printf("\r\n  *(UINT8*)ptrBuf %X",  *(UINT8*)ptrBuf);
	
	while(size < fileSize)
	{	
		/*Узнаем, что прибавить к очередному цвету. */
		rndColor = rand() % 70;
		sign = rand() % 2;
		if( (*(UINT8*)ptrBuf + rndColor ) >= 0xFF )
		{
			sign = MINUS;
		}
		if((*(UINT8*)ptrBuf - rndColor) <= 0)
		{
			sign = PLUS;
		}


		sign = rand() % 2;

	
		
		/*printf("\r\n After %03X", *(UINT32*)ptrBuf);*/
		
		if(2 == offset)
		{
			
		}
		if(3 == offset)
		{
			
			if(PLUS == sign)
			{
				*(UINT8*)ptrBuf += rndColor;
			}
			else
			{
				*(UINT8*)ptrBuf -= rndColor;
			}
		
			
		}
		
		ptrBuf += offset;
		size += offset;
	}

		printf("\r\n  size is %d", size);

	sprintf(outFileName, "unic_%s",filename);

		/* Откроем файл. */
	fOut = fopen(outFileName, "wb+");
	if (!fOut)
	{
		/* Не открылся файл. */
		printf("%s Error creat file %s", imageFuncName, outFileName);
		return ERROR;
	}
	
	numBytes = fwrite(buf, 1, size, fOut);
	if(numBytes > 0)
	{
		printf("\r\n Have been wrote %X bytes", numBytes);
	}
	else
		printf("\r\n Error with write file %s", outFileName);



	fclose(fOut);

}






