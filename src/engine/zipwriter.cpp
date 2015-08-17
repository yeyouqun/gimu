//
// zip writer and its block definition.
// Author:yeyouqun@163.com
// 2009-11-9
//

#include <gmpre.h>
#include <engine/defs.h>
#include <engine/uifblock.h>
#include <engine/uifalgo.h>
#include <engine/exception.h>
#include <engine/writer.h>
#include <engine/encdeflate.h>
#include <engine/zipblock.h>
#include <engine/zipcrc.h>
#include <engine/reader.h>
#include <engine/event.h>
#include <engine/execitem.h>
#include <engine/uniindex.h>
#include <engine/restorenode.h>
#include <engine/zipfile.h>
#include <engine/zipwriter.h>

GmZipDecorator::GmZipDecorator (GmZipDataHandler * pZipStream)
	: _pZipStream (pZipStream)
{
	//assert (_pZipStream);
}

unsigned char * GmZipDecorator::CaculateData (unsigned char * pData
									, unsigned int	uiInLen
									, unsigned int * puiOutLen
									, unsigned long reversed)
{
	if (0 == _pZipStream) {
		*puiOutLen = uiInLen;
		return pData;
	}

	return _pZipStream->CaculateData (pData, uiInLen, puiOutLen, reversed);
}

GmZipDecorator::~GmZipDecorator ()
{
	delete _pZipStream;
}

GmDeflateHandler::GmDeflateHandler (GmZipDataHandler * pZipStream
					, GmZippor * pCompressor)
	: GmZipDecorator (pZipStream)  // before compressing, do nothing.
	, _pCompressor (pCompressor)
{
	assert (_pCompressor);
	//
	// The out buffer is 5M, so the data to be delt with must lesser than (5M - 12) / 1.1
	// this is the zlib's specification.Because scheduler's the buffer size is 4M, so this is 
	// guaranteed.
	//
	_pOutBuffer = new ACE_Message_Block (5 * 1024 * 1024);
}

/*
 * @param status				Control the handler's status, the value
 *								can be GmDeflateHandler::BEGIN_STREAM
 *								, GmDeflateHandler::ZIP_STEEAM
 *								, GmDeflateHandler::END_STREAM.
 */
unsigned char * GmDeflateHandler::CaculateData (unsigned char * pData
									, unsigned int	uiInLen
									, unsigned int * puiOutLen
									, unsigned long status)
{
	unsigned char * pOut = GmZipDecorator::CaculateData (pData
															, uiInLen
															, puiOutLen
															, status);
	unsigned int size = *puiOutLen;
	//
	// Before call this interface, client must initialize the compressor.
	//
	unsigned char * pOut2	= reinterpret_cast<unsigned char *> (_pOutBuffer->base ());
	unsigned int	outSize = static_cast<unsigned int> (_pOutBuffer->size ());
	/**
	* Due to that the AES encryption encyrypt data of block 16 bytes.so 
	* here pData must pointer to the buffer that has least 16 bytes available
	* before the pData.So if compress before encrypting, compression out put
	* buffer has to do so, and if not compress, input buffer has to do so.
	* Of course, we can select another way to put available bytes on end, but it is
	* not effiecient method.
	*/
	pOut2 += AES_BLOCK_LEN;
	bool bIsEnd = status == GmDeflateHandler::END_STREAM ? true : false;

	_pCompressor->compress (pData
							, uiInLen
							, pOut2
							, outSize
							, bIsEnd);
	*puiOutLen = outSize;
	return pOut2;
}

GmDeflateHandler::~GmDeflateHandler ()
{
	_pOutBuffer->release ();
}



GmZipEncHandler::GmZipEncHandler (GmZipDataHandler * pZipStream, GmWizipAesEnc * pEncoder)
	: GmZipDecorator (pZipStream)
	, _pEncoder (pEncoder)
{
	assert (_pEncoder);
	_vRemains.reserve (AES_BLOCK_LEN);
}

/**
 * Before call this interface, client must initialize the encoder.
 * Due to that the AES encryption encyrypt data of block 16 bytes.so 
 * here pData must pointer to the buffer that has least 16 bytes available
 * before the pData.So if compress before encrypting, compression out put
 * buffer has to do so, and if not compress, input buffer has to do so.
 * Of course, we can select another way to put available bytes on end, but it is
 * not effiecient method.
 */
unsigned char * GmZipEncHandler::CaculateData (unsigned char * pData
									, unsigned int	uiInLen
									, unsigned int * puiOutLen
									, unsigned long status)
{
	unsigned char * pOut = GmZipDecorator::CaculateData (pData
															, uiInLen
															, puiOutLen
															, status);

	unsigned int inputSize = *puiOutLen;
	if (_vRemains.size () != 0) {
		inputSize += (unsigned int)_vRemains.size ();
		pOut -= _vRemains.size ();
		memcpy (pOut, &_vRemains[0], _vRemains.size ());
		_vRemains.clear ();
	}

	if (status != GmDeflateHandler::END_STREAM) {
		//
		// not end stream.
		//
		unsigned int remains = inputSize % AES_BLOCK_LEN;
		inputSize -= remains;
		//
		//copy remained bytes to a vector.
		//
		_vRemains.assign (pOut + inputSize, pOut + inputSize + remains);
	}

	_pEncoder->CaculateData (pOut, inputSize);
	*puiOutLen = inputSize;
	return pOut;
}



GmInflateHandler::GmInflateHandler (GmZipDataHandler * pZipStream
						, GmUnzippor * pDeompressor)
						: GmZipDecorator (pZipStream)
						, _pDeompressor (pDeompressor)
{
	assert (_pDeompressor);
	//
	// The out buffer is 5M, so the data to be delt with must lesser than (5M - 12) / 1.1
	// this is the zlib's specification.Because scheduler's the buffer size is 4M, so this is 
	// guaranteed.
	//
	_pOutBuffer = new ACE_Message_Block (5 * 1024 * 1024);
}

GmInflateHandler::~GmInflateHandler ()
{
	_pOutBuffer->release ();
}

unsigned char * GmInflateHandler::CaculateData (unsigned char * pData
									, unsigned int	uiInLen
									, unsigned int * puiOutLen
									, unsigned long status)
{
	unsigned char * pOut = GmZipDecorator::CaculateData (pData
															, uiInLen
															, puiOutLen
															, status);
	
	unsigned char * pOutBuffer = (unsigned char *)_pOutBuffer->base ();
	ubyte4			uOutSize = (ubyte4)_pOutBuffer->size ();

	bool bIsEnd = false;
	_pDeompressor->decompress (pOut, *puiOutLen, pOutBuffer, uOutSize, bIsEnd);
	assert ((status == GmDeflateHandler::END_STREAM ? true : false) == bIsEnd);

	*puiOutLen = uOutSize;
	return pOutBuffer;
}

wxString GetZipFileName (const wxString & suOriginName, ubyte4 uiArchive)
{
	wchar_t extension[30];

	if (uiArchive < 100)
		wsprintf (extension, L"%.2d", uiArchive);
	else
		wsprintf (extension, L"%d", uiArchive);

	wxString sName = suOriginName;
	wchar_t fileName[MAX_PATH];
	
	_wsplitpath (sName.c_str (), NULL, NULL, fileName, NULL);
	sName = fileName;
	sName += _(".z");
	sName += extension;

	return sName;
}


wxString GmZipWriter::getNextFile ()
{
	return GetZipFileName (_pZipFile->m_szFileName, _stZipStat.uiArchive + 1);
}

void GmZipWriter::Write (unsigned char * pBuffer
						 , ubyte4 size
						 , bool bIsHeader/* = false*/
						 , bool bRightNow/* = false*/)
{
	if (bRightNow) {
		//
		// 在缓冲区内的数据保证在此这前已经被写入到文件，调用到这里只有在回写一个文件头时需要，
		// 所以，这里不需要对文件位置进行累加。
		//
		_pWriter->Write (pBuffer, size);
		return;
	}

	if (bIsHeader) {
		ubyte8 fileFree = getFileFree ();
		ubyte8 i64BufSpace = getBufferFree ();

		if (i64BufSpace >= size && fileFree >= size) {
			_pBlock->copy (reinterpret_cast<char *> (pBuffer), size);
			_stZipStat.ui64Position += size;
		}
		else if (fileFree < size) {
			// 文件空间已经不足，先将缓冲中的数据写入文件，再把pBuffer里的数据写入可用文件空间中。
			padFile ();
			createNewZipFile ();
		}
		else if (i64BufSpace < size) {
			//缓冲空间不足，先将数据写入文件。
			flush ();
			_pBlock->copy (reinterpret_cast<char *> (pBuffer), size);
			_stZipStat.ui64Position += size;
		}
    }
	else {
		while (true) {
			ubyte8 fileFree = getFileFree ();
			ubyte8 i64BufSpace = getBufferFree ();

			if (i64BufSpace >= size && fileFree >= size) {
				_pBlock->copy (reinterpret_cast<char *> (pBuffer), size);
				_stZipStat.ui64Position += size;
				break;
			}
			else if (fileFree < size) {
				// 文件空间已经不足，先将缓冲中的数据写入文件，再把pBuffer里的数据写入可用文件空间中。
				flush ();
				_pWriter->Write (pBuffer, (ubyte4) fileFree);
				pBuffer += (ubyte4) fileFree;
				size -= (ubyte4) fileFree;
				createNewZipFile ();
			}
			else if (i64BufSpace < size) {
				//缓冲空间不足，先将数据写入文件。
				flush ();
			}
		}
	}

	return;
}

ubyte8 GmZipWriter::getFileFree ()
{
	if (!_pZipFile->m_ZipParam.bSplit)
		return -1 - _stZipStat.ui64Position;

	return _pZipFile->m_ZipParam.i64SplitSize - _stZipStat.ui64Position;
}

void GmZipWriter::flush ()
{
	unsigned char *base	= reinterpret_cast<unsigned char *> (_pBlock->base ());
	ubyte4 size			= static_cast<ubyte4> (_pBlock->length ());

	if (size == 0)
		return;

	_pWriter->Write (base, size);

	_pBlock->reset ();
}

GmZipWriter::GmZipWriter (GmWriter * _pWriter, GmZipFile * pZipFile)
						: _pWriter (_pWriter)
						, _pBlock (0)
						, _pZipFile (pZipFile)
{
	assert (_pZipFile);
	_pBlock = new ACE_Message_Block (MAX_BLOCK_SIZE);
}

ubyte8 GmZipWriter::seekToFileHeader ()
{
	ubyte4 uiCurFile = _stZipStat.uiCurFileArchive;
	ubyte8 uiCurFilePos = _stZipStat.ui64CurFilePos;
	flush ();

	if (uiCurFile == _stZipStat.uiArchive) {
		return _pWriter->Seek (uiCurFilePos, wxFromStart);
	}

	wxString sCurFile = getCorrespondingFile (uiCurFile);
	_pWriter->Close ();
	_pWriter->Rebind (sCurFile, wxFile::write_append);
	
	return _pWriter->Seek (uiCurFilePos, wxFromStart);
}

wxString GmZipWriter::getCorrespondingFile (ubyte4 uiCurFile)
{
	if (uiCurFile == _stZipStat.uiArchive)
		return _pZipFile->m_szFileName;

	return GetZipFileName (_pZipFile->m_szFileName, uiCurFile + 1);
}


ubyte8 GmZipWriter::seekLastFileToEnd ()
{
	ubyte4 uiCurFile = _stZipStat.uiCurFileArchive;

	if (uiCurFile == _stZipStat.uiArchive)
		return _pWriter->Seek (0, wxFromEnd);
	
	wxString curArchive = getCorrespondingFile (_stZipStat.uiArchive);
	_pWriter->Rebind (curArchive, wxFile::write_append);
	return _pWriter->Seek (0, wxFromEnd);
}

void GmZipWriter::beginNewFile ()
{
	_stZipStat.uiCurFileArchive = _stZipStat.uiArchive;
	_stZipStat.ui64CurFilePos = _stZipStat.ui64Position;
	return;
}

ubyte8 GmZipWriter::curEntries = 0;

void GmZipWriter::padFile ()
{
	// random data are written to file end.
	char buffer[ZIP_BLOCK_BUFFER_LEN];
	flush ();
	ubyte8 fileFree = getFileFree ();

	while (true) {
		if (fileFree < ZIP_BLOCK_BUFFER_LEN) {
			_pWriter->Write (reinterpret_cast<unsigned char *> (buffer), (ubyte4) fileFree);
			_stZipStat.ui64Position += fileFree;
			break;
		}

		_pWriter->Write (reinterpret_cast<unsigned char *> (buffer), ZIP_BLOCK_BUFFER_LEN);
		_stZipStat.ui64Position += ZIP_BLOCK_BUFFER_LEN;
		fileFree -= ZIP_BLOCK_BUFFER_LEN;
	}

	return;
}

void GmZipWriter::createNewZipFile ()
{
	flush ();

	wxString nextFile = getNextFile ();
	_pWriter->Rename (nextFile);

	_pWriter->Rebind (_pZipFile->m_szFileName, wxFile::write_append);
	++_stZipStat.uiArchive;
	_stZipStat.ui64Position = 0;

	GmZipWriter::curEntries = 0;//reset.
	return;
}