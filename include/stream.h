/*
 * stream.h
 *
 *  Created on: 2009-3-26
 *      Author: dongjunjie
 */

#ifndef STREAM_H_
#define STREAM_H_

#ifdef WIN32

typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
typedef unsigned int		uint32_t;

#define LITTLE_ENDIAN	0
#define BIG_ENDIAN		1

#endif
typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
typedef unsigned int		uint32_t;



class StreamStorage
{
public:
	enum { seek_begin = 0, seek_end, seek_cur};
	virtual ~StreamStorage(){}
	virtual void write(int size, const void* data) = 0;
	virtual void flush() = 0;
	virtual int seek(long offset, int where) = 0;
	virtual int tell() = 0;
	virtual bool ready() = 0;
};

class Stream
{
protected:
	StreamStorage * storage;

public:

	Stream(StreamStorage * storage) : storage(storage) {}

	virtual ~Stream(){ flush(); }

	virtual void flush(){
		if(storage)
			storage->flush();
	}

	int seek(int offset, int where){
		if(storage)
			return storage->seek(offset, where);
		return -1;
	}

	int tell(){
		if(storage)
			return storage->tell();
		return -1;
	}
};

class TextStream : public Stream
{
protected:
	char prefix[64];
	bool bNewLine;
public:
	TextStream(StreamStorage * storage)
	:Stream(storage)
	{
		prefix[0] = '\0';
		bNewLine = true;
	}

	void indent()
	{
		strcat(prefix,"\t");
	}
	void unindent()
	{
		int len = strlen(prefix);
		if(len > 0)
		prefix[len-1] = '\0';
	}

	void printf(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		vprintf(format, args);
	}

	void println(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		if(storage){
			storage->write(1,"\n");
			bNewLine = true;
		}
	}

private:
	void vprintf(const char* format, va_list args);

};

///////////////////////
//BinStream
class BinStream : public Stream
{
public:

private:
	static uint16_t _switch_uint16(uint16_t u16){ return ((u16&0xFF)<<8)|(u16>>8) ;	}
	static uint32_t _switch_uint32(uint32_t u32){
		return ((u32&0xFF)<<24)
				   |((u32&0xFF00)<<8)
						|((u32&0xFF0000)>>8)
						|((u32&0xFF000000)>>24);
	}


	static uint16_t _nochange_uint16(uint16_t u16){return u16; }
	static uint32_t _nochange_uint32(uint32_t u32){return u32; }

	static int getSysEndian(){
		union {
			uint32_t u32;
			uint8_t  u8[4];
		}v;
		v.u32 = 0x12345678;
		if(v.u8[0] == 0x78)
			return LITTLE_ENDIAN;
		return BIG_ENDIAN;
	}

protected:
	uint16_t (*to16)(uint16_t);
	uint32_t (*to32)(uint32_t);
public:
	BinStream(StreamStorage *storage, int endian=LITTLE_ENDIAN)
	:Stream(storage)
	{
		if(endian == getSysEndian())
		{
			to16 = _nochange_uint16;
			to32 = _nochange_uint32;
		}
		else
		{
			to16 = _switch_uint16;
			to32 = _switch_uint32;
		}
	}
	~BinStream(){}

	void save8(uint8_t u8){
		if(storage){
			storage->write(sizeof(uint8_t), &u8);
		}
	}
	void save16(uint16_t u16){
		if(storage){
			u16 = to16(u16);
			storage->write(sizeof(uint16_t), &u16);
		}
	}
	void save32(uint32_t u32){
		if(storage){
			u32 = to32(u32);
			storage->write(sizeof(uint32_t), &u32);
		}
	}

	void save8Arr(const uint8_t *u8s, int size){
		if(storage && u8s)
			storage->write(sizeof(uint8_t)*size, u8s);
	}

	template<typename INT>
	void saveIntArr(const INT *ints, int size,  INT (*toInt)(INT))
	{
		if(storage && ints){
			for(int i=0; i<size; i++){
				INT tint = toInt(ints[i]);
				storage->write(sizeof(INT), &tint);
			}
		}
	}

	void save16Arr(const uint16_t *u16s, int size){
		saveIntArr<uint16_t>(u16s, size, to16);
	}
	void save32Arr(const uint32_t *u32s, int size){
		saveIntArr<uint32_t>(u32s, size, to32);
	}

};

#if 0
//StrPoolBinStream
class StrPoolBinStream : public BinStream
{
	struct StrPool{
		int size;
		char pool[1024];
		StrPool *next;
	};

	StrPool *pools;
	StrPool *curPool;
	int totalOffset;

	map<DWORD, int> strKeyInfo;

public:
	StrPoolBinStream(StreamStorage *storage, int endian=LITTLE_ENDIAN)
	:BinStream(storage, endian)
	{
		pools = NULL;
		totalOffset = 0;
		curPool = NULL;
	}

	~StrPoolBinStream();

	void flush();

	void saveStr(const char* str, int size=-1);
};
#endif

//////File Storage
class FileStreamStorage : public StreamStorage
{
	FILE *file;
	bool bclose;
public:
	FileStreamStorage(const char* fileName, bool binary=false){
		file = fopen(fileName,binary?"wb":"wt");
		bclose = true;
	}
	FileStreamStorage(FILE *file){
		this->file = file;
		bclose = false;
	}

	~FileStreamStorage(){
		close();
	}

	void close(){
		flush();
		if(bclose && file)
			fclose(file);
		file = NULL;
	}

	void write(int size, const void* data)
	{
		if(size <= 0 || data == NULL || file == NULL)
			return;
		fwrite(data, size, 1, file);
	}

	void flush()
	{
		if(file)
			fflush(file);
	}

	int seek(long offset, int where)
	{
		if(file){
			if(where == seek_begin)
				where = SEEK_SET;
			else if(where == seek_end)
				where = SEEK_END;
			else
				where = SEEK_CUR;
			return fseek(file,offset,where);
		}
		return -1;
	}

	int tell(){
		return file?ftell(file):-1;
	}

	bool ready(){
		return file!=NULL;
	}
};

std::string EntityReferenceTranslate(const char* str);
#define _ERT(str)   EntityReferenceTranslate(str).c_str()

#endif /* STREAM_H_ */
