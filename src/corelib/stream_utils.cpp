/* $Id: stream_utils.cpp 154686 2009-03-13 14:44:35Z lavr $
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
 * Author:  Anton Lavrentiev, Denis Vakatov
 *
 * File Description:
 *   Stream utilities:
 *   1. Push an arbitrary block of data back to a C++ input stream.
 *   2. Non-blocking read.
 *
 *   Reader-writer utilities:
 *   1. Append an IReader's contents into a string.
 *   2. Construct an IReader object from a string.
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/stream_utils.hpp>
#include <corelib/error_codes.hpp>

#ifdef NCBI_COMPILER_MIPSPRO
#  define CPushback_StreambufBase CMIPSPRO_ReadsomeTolerantStreambuf
#else
#  define CPushback_StreambufBase CNcbiStreambuf
#endif //NCBI_COMPILER_MIPSPRO

#ifdef    HAVE_GOOD_IOS_CALLBACKS
#  undef  HAVE_GOOD_IOS_CALLBACKS
#endif
#if defined(HAVE_IOS_REGISTER_CALLBACK)  &&  \
  (!defined(NCBI_COMPILER_WORKSHOP)  ||  !defined(_MT))
#  define HAVE_GOOD_IOS_CALLBACKS 1
#endif


#define NCBI_USE_ERRCODE_X   Corelib_StreamUtil


BEGIN_NCBI_SCOPE


/*****************************************************************************
 *  Helper class: internal streambuf to be substituted instead of
 *  the original one in the stream, when the data are pushed back.
 */

class CPushback_Streambuf : public CPushback_StreambufBase
{
    friend struct CStreamUtils;

public:
    CPushback_Streambuf(istream& istream, CT_CHAR_TYPE* buf,
                        streamsize buf_size, void* del_ptr);
    virtual ~CPushback_Streambuf();

protected:
    virtual CT_POS_TYPE seekoff(CT_OFF_TYPE off, IOS_BASE::seekdir whence,
                                IOS_BASE::openmode which);
    virtual CT_POS_TYPE seekpos(CT_POS_TYPE pos, IOS_BASE::openmode which);

    virtual CT_INT_TYPE overflow(CT_INT_TYPE c);
    virtual streamsize  xsputn(const CT_CHAR_TYPE* buf, streamsize n);
    virtual CT_INT_TYPE underflow(void);
    virtual streamsize  xsgetn(CT_CHAR_TYPE* buf, streamsize n);
    virtual streamsize  showmanyc(void);

    virtual CT_INT_TYPE pbackfail(CT_INT_TYPE c = CT_EOF);

    virtual int         sync(void);

    // declared setbuf here to only throw an exception at run-time
    virtual streambuf*  setbuf(CT_CHAR_TYPE* buf, streamsize buf_size);

private:
    void                x_FillBuffer(streamsize max_size);
    void                x_DropBuffer(void);

    istream&            m_Is;      // I/O stream this streambuf is attached to
    streambuf*          m_Sb;      // original streambuf
    CT_CHAR_TYPE*       m_Buf;     // == 0 when the buffer has been emptied
    streamsize          m_BufSize;
    void*               m_DelPtr;

#ifdef HAVE_GOOD_IOS_CALLBACKS
    int                 m_Index;
    static void         x_Callback(IOS_BASE::event, IOS_BASE&, int);
#endif //HAVE_GOOD_IOS_CALLBACKS

    static const streamsize kMinBufSize;
};


const streamsize CPushback_Streambuf::kMinBufSize = 4096;


#ifdef HAVE_GOOD_IOS_CALLBACKS
void CPushback_Streambuf::x_Callback(IOS_BASE::event event,
                                     IOS_BASE&       ios,
                                     int             index)
{
    if (event == IOS_BASE::erase_event)
        delete static_cast<streambuf*> (ios.pword(index));
}
#endif //HAVE_GOOD_IOS_CALLBACKS


CPushback_Streambuf::CPushback_Streambuf(istream&      is,
                                         CT_CHAR_TYPE* buf,
                                         streamsize    buf_size,
                                         void*         del_ptr) :
    m_Is(is), m_Buf(buf), m_BufSize(buf_size), m_DelPtr(del_ptr)
{
    setp(0, 0); // unbuffered output at this level of streambuf's hierarchy
    setg(m_Buf, m_Buf, m_Buf + m_BufSize);
    m_Sb = m_Is.rdbuf(this);
#ifdef HAVE_GOOD_IOS_CALLBACKS
    try {
        m_Index             = m_Is.xalloc();
        m_Is.pword(m_Index) = this;
        m_Is.register_callback(x_Callback, m_Index);
    }
    STD_CATCH_ALL_X(1, "CPushback_Streambuf::CPushback_Streambuf");
#endif //HAVE_GOOD_IOS_CALLBACKS
}


CPushback_Streambuf::~CPushback_Streambuf()
{
#ifdef HAVE_GOOD_IOS_CALLBACKS
    m_Is.pword(m_Index) = 0;
#endif //HAVE_GOOD_IOS_CALLBACKS
    delete[] (CT_CHAR_TYPE*) m_DelPtr;
    if (m_Sb) {
        m_Is.rdbuf(m_Sb);
        if (dynamic_cast<CPushback_Streambuf*> (m_Sb))
            delete m_Sb;
    }
}


CT_POS_TYPE CPushback_Streambuf::seekoff(CT_OFF_TYPE off,
                                         IOS_BASE::seekdir whence,
                                         IOS_BASE::openmode which)
{
    if (whence == ios::cur  &&  (which & ios::in)) {
        if (off == 0  &&  which == ios::in) {
            // it's a call from tellg()
            CT_POS_TYPE ret = m_Sb->PUBSEEKOFF(0, ios::cur, ios::in);
            if (ret != (CT_POS_TYPE)((CT_OFF_TYPE)(-1))) {
                off = (CT_OFF_TYPE)(egptr() - gptr());
                if ((CT_OFF_TYPE) ret >= off)
                    return ret - off;
            }
        }
        return (CT_POS_TYPE)((CT_OFF_TYPE)(-1));
    }
    x_DropBuffer();
    return m_Sb->PUBSEEKOFF(off, whence, which);
}


CT_POS_TYPE CPushback_Streambuf::seekpos(CT_POS_TYPE pos,
                                         IOS_BASE::openmode which)
{
    x_DropBuffer();
    return m_Sb->PUBSEEKPOS(pos, which);
}


CT_INT_TYPE CPushback_Streambuf::overflow(CT_INT_TYPE c)
{
    if ( !CT_EQ_INT_TYPE(c, CT_EOF) ) {
        return m_Sb->sputc(CT_TO_CHAR_TYPE(c));
    }

    return m_Sb->PUBSYNC() == 0 ? CT_NOT_EOF(CT_EOF) : CT_EOF;
}


streamsize CPushback_Streambuf::xsputn(const CT_CHAR_TYPE* buf, streamsize n)
{
    // hope that this is an optimized copy operation (instead of overflow()s)
    return m_Sb->sputn(buf, n);
}


CT_INT_TYPE CPushback_Streambuf::underflow(void)
{
    // we are here because there is no more data in the pushback buffer
    _ASSERT(gptr()  &&  gptr() >= egptr());

#ifdef NCBI_COMPILER_MIPSPRO
    if (m_MIPSPRO_ReadsomeGptrSetLevel  &&  m_MIPSPRO_ReadsomeGptr != gptr())
        return CT_EOF;
    m_MIPSPRO_ReadsomeGptr = (CT_CHAR_TYPE*)(-1L);
#endif //NCBI_COMPILER_MIPSPRO

    x_FillBuffer(m_Sb->in_avail());
    return gptr() < egptr() ? CT_TO_INT_TYPE(*gptr()) : CT_EOF;
}


streamsize CPushback_Streambuf::xsgetn(CT_CHAR_TYPE* buf, streamsize m)
{
    size_t n_total = 0;
    while (m) {
        if (gptr() < egptr()) {
            size_t n       = (size_t) m;
            size_t n_avail = (size_t) (egptr() - gptr());
            size_t n_read  = (n <= n_avail) ? n : n_avail;
            if (buf != gptr()) // either equal or non-overlapping
                memcpy(buf, gptr(), n_read);
            gbump((int) n_read);
            m       -= (streamsize) n_read;
            buf     += (streamsize) n_read;
            n_total += (streamsize) n_read;
        } else {
            x_FillBuffer(m);
            if (gptr() >= egptr())
                break;
        }
    }
    return n_total;
}


streamsize CPushback_Streambuf::showmanyc(void)
{
    // we are here because (according to the standard) gptr() >= egptr()
    _ASSERT(gptr()  &&  gptr() >= egptr());

    return m_Sb->in_avail();
}


CT_INT_TYPE CPushback_Streambuf::pbackfail(CT_INT_TYPE /*c*/)
{
    /* We always maintain "usual backup condition" (27.5.2.4.3.13) after
     * underflow(), i.e. 1 byte backup after a good read is always possible.
     * That is, this function gets called only if the user tries to
     * back up more than once (although, some attempts may be successful,
     * this function call denotes that the backup area is full).
     */
    return CT_EOF; // always fail
}


int CPushback_Streambuf::sync(void)
{
    return m_Sb->PUBSYNC();
}


streambuf* CPushback_Streambuf::setbuf(CT_CHAR_TYPE* /*buf*/,
                                       streamsize    /*buf_size*/)
{
    NCBI_THROW(CCoreException, eCore,
               "CPushback_Streambuf::setbuf: not allowed");
    /*NOTREACHED*/
    return this;
}


void CPushback_Streambuf::x_FillBuffer(streamsize max_size)
{
    _ASSERT(m_Sb);
    if ( !max_size )
        ++max_size;
    CPushback_Streambuf* sb = dynamic_cast<CPushback_Streambuf*> (m_Sb);
    if ( sb ) {
        _ASSERT(&m_Is == &sb->m_Is);
        m_Sb     = sb->m_Sb;
        sb->m_Sb = 0;
        if (sb->gptr() >= sb->egptr()) {
            delete sb;
            x_FillBuffer(max_size);
            return;
        }
        delete[] (CT_CHAR_TYPE*) m_DelPtr;
        m_Buf        = sb->m_Buf;
        m_BufSize    = sb->m_BufSize;
        m_DelPtr     = sb->m_DelPtr;
        sb->m_DelPtr = 0;
        setg(sb->gptr(), sb->gptr(), sb->egptr());
        delete sb;
    } else {
        CT_CHAR_TYPE* bp = 0;
        streamsize buf_size = m_DelPtr
            ? (streamsize)(m_Buf - (CT_CHAR_TYPE*) m_DelPtr) + m_BufSize : 0;
        if (buf_size < kMinBufSize) {
            buf_size = kMinBufSize;
            bp = new CT_CHAR_TYPE[buf_size];
        }
        streamsize n = m_Sb->sgetn(bp ? bp : (CT_CHAR_TYPE*) m_DelPtr,
                                   min(buf_size, max_size));
        if (n <= 0) {
            // NB: For unknown reasons WorkShop6 can return -1 from sgetn :-/
            delete bp;
            return;
        }
        if (bp) {
            delete[] (CT_CHAR_TYPE*) m_DelPtr;
            m_DelPtr = bp;
        }
        m_Buf = (CT_CHAR_TYPE*) m_DelPtr;
        m_BufSize = buf_size;
        setg(m_Buf, m_Buf, m_Buf + n);
    }
}


void CPushback_Streambuf::x_DropBuffer(void)
{
    CPushback_Streambuf* sb = dynamic_cast<CPushback_Streambuf*> (m_Sb);
    if (sb) {
        m_Sb     = sb->m_Sb;
        sb->m_Sb = 0;
        delete sb;
        x_DropBuffer();
        return;
    }
    // nothing in the buffer; no putback area as well
    setg(m_Buf, m_Buf, m_Buf);
}


void CStreamUtils::x_Pushback(CNcbiIstream& is,
                              CT_CHAR_TYPE* buf,
                              streamsize    buf_size,
                              void*         del_ptr,
                              EPushback_How how)
{
    _ASSERT(!buf_size  ||  buf);
    _ASSERT(del_ptr <= buf);

    CPushback_Streambuf* sb = dynamic_cast<CPushback_Streambuf*> (is.rdbuf());

    if (sb  &&  buf_size) {
        // We may not need to create another streambuf,
        //    just recycle the existing one here...
        _ASSERT(del_ptr <= (sb->m_DelPtr ? sb->m_DelPtr : sb->m_Buf)
                ||  sb->m_Buf + sb->m_BufSize <= del_ptr);

        // 1/ points to the adjacent part of the internal buffer we just read?
        if (how == ePushback_NoCopy
            &&  sb->m_Buf <= buf  &&  buf + buf_size == sb->gptr()) {
            _ASSERT(!del_ptr  ||  del_ptr == sb->m_DelPtr);
            sb->setg(buf, buf, sb->egptr());
            return;
        }

        // 2/ [make] equal to the [reasonably-sized, if copy] adjacent part
        //    of the internal buffer with the data that have just been read?
        if (how == ePushback_Stepback
            ||  (how == ePushback_Copy
                 &&  buf_size <= (del_ptr
                                  ? CPushback_Streambuf::kMinBufSize
                                  : CPushback_Streambuf::kMinBufSize >> 4))) {
            CT_CHAR_TYPE* bp = sb->gptr();
            streamsize avail = (streamsize)(bp - sb->m_Buf);
            streamsize take  = avail < buf_size ? avail : buf_size;
            if (take) {
                bp -= take;
                buf_size -= take;
                if (how != ePushback_Stepback  &&  bp != buf + buf_size)
                    memmove(bp, buf + buf_size, take);
                sb->setg(bp, bp, sb->egptr());
            }
        }
    }

    if (!buf_size) {
        delete[] (CT_CHAR_TYPE*) del_ptr;
        return;
    }

    if (!del_ptr  &&  how != ePushback_NoCopy) {
        del_ptr = new CT_CHAR_TYPE[buf_size];
        buf = (CT_CHAR_TYPE*) memcpy(del_ptr, buf, buf_size);
    }

    (void) new CPushback_Streambuf(is, buf, buf_size, del_ptr);
}



/*****************************************************************************
 *  Public interface
 */

#ifdef   NCBI_NO_READSOME
#  undef NCBI_NO_READSOME
#endif /*NCBI_NO_READSOME*/

#if defined(NCBI_COMPILER_GCC)
#  if NCBI_COMPILER_VERSION < 300
#    define NCBI_NO_READSOME 1
#  endif /*NCBI_COMPILER_VERSION*/
#elif defined(NCBI_COMPILER_MSVC)
    /* MSVC's readsome() is buggy [causes 1 byte reads] and is thus avoided.
     * Now when we have worked around the issue by implementing fastopen()
     * in CNcbiFstreams and thus making them fully buffered, we can go back
     * to the use of readsome() in MSVC... */
    //#  define NCBI_NO_READSOME 1
#elif defined(NCBI_COMPILER_MIPSPRO)
    /* MIPSPro does not comply with the standard and always checks for EOF
     * doing one extra read from the stream [which might be a killing idea
     * for network connections]. We introduced an ugly workaround here...
     * Don't use istream::readsome() but istream::read() instead in order to be
     * able to clear fake EOF caused by the unnecessary underflow() upcall.*/
#  define NCBI_NO_READSOME 1
#endif /*NCBI_COMPILER*/


#ifndef NCBI_NO_READSOME
static inline streamsize s_Readsome(CNcbiIstream& is,
                                    CT_CHAR_TYPE* buf,
                                    streamsize    buf_size)
{
#  ifdef NCBI_COMPILER_WORKSHOP
    /* Rogue Wave does not always return correct value from is.readsome() :-/
     * In particular, when streambuf::showmanyc() returns 1 followed by
     * a failed read() [which implements extraction from the stream], which
     * encounters the EOF, the readsome() will blindly return 1, and in
     * general, always exactly the number of bytes showmanyc() reported,
     * regardless of actually extracted by subsequent read operation.  Bug!
     * NOTE that showmanyc() does not guarantee the number of bytes that can
     * be read, but returns a best guess estimate [C++ Standard, footnote 275].
     */
    streamsize n = is.readsome(buf, buf_size);
    return n ? is.gcount() : 0;
#  else
    return is.readsome(buf, buf_size);
#  endif /*NCBI_COMPILER_WORKSHOP*/
}
#endif /*NCBI_NO_READSOME*/


static streamsize s_DoReadsome(CNcbiIstream& is,
                               CT_CHAR_TYPE* buf,
                               streamsize    buf_size)
{
    _ASSERT(buf  &&  buf_size);
#ifdef NCBI_NO_READSOME
    if ( !is.good() ) {
        is.setstate(is.rdstate() | NcbiFailbit);
        return 0; // simulate construction of sentry in real readsome()
    }
    // Special case: GCC had no readsome() prior to ver 3.0;
    // read() will set "eof" (and "fail") flag if gcount() < buf_size
    streamsize avail = is.rdbuf()->in_avail();
    if (avail == 0)
        avail++; // we still must read
    else if (buf_size < avail)
        avail = buf_size;
    // Protect read() from throwing any exceptions
    IOS_BASE::iostate save = is.exceptions();
    if (save)
        is.exceptions(NcbiGoodbit);
    is.read(buf, avail);
    // readsome is not supposed to set a failbit on a stream initially good
    is.clear(is.rdstate() & ~NcbiFailbit);
    if (save)
        is.exceptions(save);
    streamsize count = is.gcount();
    // Reset "eof" flag if some data have been read
    if (count  &&  is.eof()  &&  !is.bad())
        is.clear();
    return count;
#else
    // Try to read data
    streamsize n = s_Readsome(is, buf, buf_size);
    if (n != 0  ||  !is.good())
        return n;
    // No buffered data found, try to read from the real source [still good]
    IOS_BASE::iostate save = is.exceptions();
    if (save)
        is.exceptions(NcbiGoodbit);
    is.read(buf, 1);
    // readsome is not supposed to set a failbit on a stream initially good
    is.clear(is.rdstate() & ~NcbiFailbit);
    if (save)
        is.exceptions(save);
    if ( !is.good() )
        return 0;
    if (buf_size == 1)
        return 1; // do not need more data
    // Read more data (up to "buf_size" bytes)
    return s_Readsome(is, buf + 1, buf_size - 1) + 1;
#endif /*NCBI_NO_READSOME*/
}


streamsize CStreamUtils::Readsome(CNcbiIstream& is,
                                  CT_CHAR_TYPE* buf,
                                  streamsize    buf_size)
{
#  ifdef NCBI_COMPILER_MIPSPRO
    CMIPSPRO_ReadsomeTolerantStreambuf* sb =
        dynamic_cast<CMIPSPRO_ReadsomeTolerantStreambuf*> (is.rdbuf());
    if (sb)
        sb->MIPSPRO_ReadsomeBegin();
#  endif /*NCBI_COMPILER_MIPSPRO*/
    streamsize result = s_DoReadsome(is, buf, buf_size);
#  ifdef NCBI_COMPILER_MIPSPRO
    if (sb)
        sb->MIPSPRO_ReadsomeEnd();
#  endif /*NCBI_COMPILER_MIPSPRO*/
    return result;
}


void ExtractReaderContents(IReader& reader, string& s)
{
    size_t     n      = 0;
    SIZE_TYPE  pos    = s.size();
    ERW_Result status = eRW_Success;

    if (s.size() < 4096) {
        s.resize(4096);
    }

    while (status == eRW_Success) {
        pos += n;
        // Grow exponentially to avoid possible quadratic runtime,
        // adjusting size rather than capacity as the latter would
        // require gratuitous double-buffering.
        if (s.size() <= pos + 1024) {
            s.resize(s.size() * 2);
        }
        status = reader.Read(&s[pos], s.size() - pos, &n);
    }
    // shrink back to the actual size
    s.resize(pos + n);
    // XXX - issue diagnostic message or exception if status != eRW_Eof?
}


ERW_Result CStringReader::Read(void* buf, size_t count, size_t* bytes_read)
{
    _ASSERT(m_String.size() >= m_Position);
    size_t n = min(count, size_t(m_String.size() - m_Position));
    memcpy(buf, &m_String[m_Position], n);
    m_Position += n;
    if (m_Position >= m_String.size() / 2) {
        m_String.erase(0, m_Position);
        m_Position = 0;
    }
    if (bytes_read) {
        *bytes_read = n;
    }
    if (n == 0  &&  count > 0) {
        return eRW_Eof;
    } else {
        return eRW_Success;
    }
}


ERW_Result CStringReader::PendingCount(size_t* count)
{
    _ASSERT(m_String.size() >= m_Position);
    *count = m_String.size() - m_Position;
    return *count ? eRW_Success : eRW_Eof;
}


END_NCBI_SCOPE
