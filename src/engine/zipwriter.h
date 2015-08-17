//
// zip writer and its block definition.
// Author:yeyouqun@163.com
// 2009-11-9
//

#ifndef __GM_ZIP_WRITER_H__
#define __GM_ZIP_WRITER_H__

/**
 * Here we use altered decorator pattern.
 */
//class GmZipDataHandler;
class GmZipDataHandler
{
public:
	/**
	 *  Calculate the data and return the result.
	 *  @param pData			Data to be handled.
	 *  @param uiInLen			How many to be handled.
	 *  @param puiOutLen		After handling the data, how many bytes output.
	 *  @param reserved			A special parameter reserved for the other handler, 
	 *							such ziphandler, need reserved to indicate the end of stream.
	 *  @return					Where the handled data locate.
	 */
	virtual unsigned char * CaculateData (unsigned char * pData
										, unsigned int	uiInLen
										, unsigned int * puiOutLen
										, unsigned long reversed = 0) = 0;

	virtual ~GmZipDataHandler () {}
};

class GmZipDecorator : public GmZipDataHandler
{
public:
	GmZipDecorator (GmZipDataHandler * pZipStream);
	virtual unsigned char * CaculateData (unsigned char * pData
										, unsigned int	uiInLen
										, unsigned int * puiOutLen
										, unsigned long reversed);

	~GmZipDecorator ();
private:
	GmZipDataHandler * _pZipStream;
};

class GmZipNullHandler : public GmZipDecorator
{
public:
	GmZipNullHandler () : GmZipDecorator (0) {}

	virtual unsigned char * handleData (unsigned char * pData
										, unsigned int	uiInLen
										, unsigned int * puiOutLen
										, unsigned long reversed)
	{
		// Do nothing.
		*puiOutLen = uiInLen;
		return pData;
	}

};

class GmDeflateHandler : public GmZipDecorator
{
public:
	enum { BEGIN_STREAM = 1  // Begin to compress a file.
			, ZIP_STEEAM = 2 // Continue to compress data.
			, END_STREAM = 3}; // End compress a file.

	GmDeflateHandler (GmZipDataHandler * pZipStream, GmZippor * pCompressor);

	/*
	 * @param status				Control the handler's status, the value
	 *								can be GmDeflateHandler::BEGIN_STREAM
	 *								, GmDeflateHandler::ZIP_STEEAM
	 *								, GmDeflateHandler::END_STREAM.
	 */
	virtual unsigned char * CaculateData (unsigned char * pData
										, unsigned int	uiInLen
										, unsigned int * puiOutLen
										, unsigned long status);

	~GmDeflateHandler ();
private:
	GmZippor *				_pCompressor;
	ACE_Message_Block *		_pOutBuffer;
};

class GmZipEncHandler : public GmZipDecorator
{
public:
	GmZipEncHandler (GmZipDataHandler * pZipStream, GmWizipAesEnc * pEncoder);
	virtual unsigned char * CaculateData (unsigned char * pData
										, unsigned int	uiInLen
										, unsigned int * puiOutLen
										, unsigned long status);
private:
	GmWizipAesEnc * _pEncoder;
	vector<unsigned char>	_vRemains;
};

class GmInflateHandler : public GmZipDecorator
{
public:
	GmInflateHandler (GmZipDataHandler * pZipStream, GmUnzippor * pDeompressor);
	~GmInflateHandler ();
	virtual unsigned char * CaculateData (unsigned char * pData
										, unsigned int	uiInLen
										, unsigned int * puiOutLen
										, unsigned long status);

private:
	GmUnzippor * _pDeompressor;
	ACE_Message_Block * _pOutBuffer;
};

class GmZipFile;
class GmZipWriter
{
public:
	GmZipWriter (GmWriter * pWriter, GmZipFile * pZipFile);

	/**
	 * Get buffer's free space.
	 */

	ubyte4 getBufferFree ()
	{
		return (ubyte4)_pBlock->space ();
	}

	/**
	 * Notify the writer that a new file will begin to add to zipfile, so the writer
	 * can reset and save some status and argument for later use.
	 */
	void beginNewFile ();

	/**
	 * When split file, pad empty free file space to ensure every zip file has the same size.
	 * Except the last one.
	 */
	void padFile ();

	/**
	 * Create a new zip file ,change corresponding status, or rename file and etc.
	 */
	void createNewZipFile ();

	/**
	 * Write buffer to file.
	 */
	void flush ();

	/**
	 * Get remained free space to write to current file .
	 */
	ubyte8 getFileFree ();

	/**
	 *  Get current file offset.
	 */
	ubyte8 getCurrentOffset () const
	{
		return _stZipStat.ui64Position;
	}

	/**
	 *  Get current span (split) disk number, also split number, span not supportted here.
	 */

	ubyte4 getDiskNum () const 
	{
		return _stZipStat.uiArchive;
	}

	ubyte4 getTotalDisk () const
	{
		return _stZipStat.uiArchive + 1;// 0 based.
	}

	/**
	 *  Write pBuffer to interal buffer or to file.
	 *  @param pBuffer			buffer to write.
	 *  @param size				how many to write.
	 *  @param bIsHeader		data in pBuffer is header's content.
	 *  @param bRightNow		data has to be written to file right away.
	 *  @return					void.
	 */
	void Write (unsigned char * pBuffer
				, ubyte4 size
				, bool bIsHeader = false
				, bool bRightNow = false);

	/**
	 * Set current file's position in zip archive to the start point. 
	 */
	ubyte8 seekToFileHeader ();

	/**
	 * Set the zip archive's pointer to the end.
	 */
	ubyte8 seekLastFileToEnd ();

	/**
	 * Get entries on current entries.when splitting, this value reset,
	 * and increment by zip file addNewFile.
	 */
	static ubyte8			curEntries;
protected:
	/**
		*  Get next file to write, splitting use it.
		*/
	wxString getNextFile ();

	/**
	 *  Get zip file name by uiCurFile;
	 */
	wxString getCorrespondingFile (ubyte4 uiCurFile);
	struct GmZipCurrStat
	{
		GmZipCurrStat () : uiArchive (0)
							, ui64Position (0)
							, uiCurFileArchive (1)
							, ui64CurFilePos (0)
		{
			;
		}
		// current archive number.for zip64 four bytes are used. 0 based.
		ubyte4		uiArchive;
		// current position on arichive uiArichive.
		ubyte8		ui64Position;
		// record the current added file's at ui64CurFilePos positon in uiCurFileArchive zip file.
		ubyte4		uiCurFileArchive;
		ubyte8		ui64CurFilePos;
	};

	GmZipCurrStat 	_stZipStat;

	GmWriter *		_pWriter;
	ACE_Message_Block *		_pBlock;

	GmZipFile *				_pZipFile;
};

wxString GetZipFileName (const wxString & suOriginName, ubyte4 uiArchive);

#endif // __GM_ZIP_WRITER_H__