#ifndef CORELIB___READER_WRITER__HPP
#define CORELIB___READER_WRITER__HPP

/*  $Id: reader_writer.hpp 103491 2007-05-04 17:18:18Z kazimird $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Anton Lavrentiev
 *
 * File Description:
 *   Abstract reader-writer interface classes
 *
 */

/// @file reader_writer.hpp
/// Abstract reader-writer interface classes
///
/// Slightly adapted, however, to build std::streambuf on top of them.


#include <corelib/ncbistl.hpp>
#include <stddef.h>


BEGIN_NCBI_SCOPE


/** @addtogroup Stream
 *
 * @{
 */


/// Result codes for I/O operations
/// @sa IReader, IWriter, IReaderWriter
enum ERW_Result {
    eRW_NotImplemented = -1,
    eRW_Success = 0,
    eRW_Timeout,
    eRW_Error,
    eRW_Eof
};


/// A very basic data-read interface.

class IReader
{
public:
    /// Read as many as "count" bytes into a buffer pointed
    /// to by "buf" argument.  Store the number of bytes actually read,
    /// or 0 on EOF or error, via the pointer "bytes_read", if provided.
    /// Special case:  if "count" passed as 0, then the value of
    /// "buf" is ignored, and the return value is always eRW_Success,
    /// but no change should be actually made to the state of input device.
    virtual ERW_Result Read(void*   buf,
                            size_t  count,
                            size_t* bytes_read = 0) = 0;

    /// Via parameter "count" (which is guaranteed to be supplied non-NULL)
    /// return the number of bytes that are ready to be read from input
    /// device without blocking.  Return eRW_Success if the number of
    /// pending bytes has been stored at the location pointed to by "count".
    /// Return eRW_NotImplemented if the number cannot be determined.
    /// Otherwise, return other eRW_... condition to reflect the problem.
    /// Note that if reporting 0 bytes ready, the method may return either
    /// both eRW_Success and zero *count, or return eRW_NotImplemented alone.
    virtual ERW_Result PendingCount(size_t* count) = 0;

    virtual ~IReader() {}
};



/// A very basic data-write interface.

class IWriter
{
public:
    /// Write up to "count" bytes from the buffer pointed to by
    /// "buf" argument onto output device.  Store the number
    /// of bytes actually written, or 0 if "count" has been passed as 0
    /// ("buf" is ignored in this case), via the "bytes_written" pointer,
    /// if provided non-NULL.
    virtual ERW_Result Write(const void* buf,
                             size_t      count,
                             size_t*     bytes_written = 0) = 0;

    /// Flush pending data (if any) down to output device.
    virtual ERW_Result Flush(void) = 0;

    virtual ~IWriter() {}
};



/// A very basic data-read/write interface.

class IReaderWriter : public IReader, public IWriter
{
public:
    virtual ~IReaderWriter() {}
};


/* @} */


END_NCBI_SCOPE

#endif  /* CORELIB___READER_WRITER__HPP */
