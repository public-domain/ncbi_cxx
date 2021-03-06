#ifndef CONNECT___NCBI_CONNUTIL__H
#define CONNECT___NCBI_CONNUTIL__H

/* $Id: ncbi_connutil.h 151480 2009-02-04 19:25:28Z lavr $
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
 * Author:  Denis Vakatov, Anton Lavrentiev
 *
 * File Description:
 *   Auxiliary API to:
 *    1.Retrieve connection related info from the registry:
 *       SConnNetInfo
 *       ConnNetInfo_Create()
 *       ConnNetInfo_AdjustForHttpProxy()
 *       ConnNetInfo_Clone()
 *       ConnNetInfo_Print()
 *       ConnNetInfo_Destroy()
 *       ConnNetInfo_Log()
 *       ConnNetInfo_ParseURL()
 *       ConnNetInfo_SetUserHeader()
 *       ConnNetInfo_AppendUserHeader()
 *       ConnNetInfo_DeleteUserHeader()
 *       ConnNetInfo_OverrideUserHeader()
 *       ConnNetInfo_ExtendUserHeader()
 *       ConnNetInfo_AppendArg()
 *       ConnNetInfo_PrependArg()
 *       ConnNetInfo_DeleteArg()
 *       ConnNetInfo_DeleteAllArgs()
 *       ConnNetInfo_PreOverrideArg()
 *       ConnNetInfo_PostOverrideArg()
 *       ConnNetInfo_SetupStandardArgs()
 *       #define REG_CONN_***
 *       #define DEF_CONN_***
 *
 *    2.Make a connection to an URL:
 *       URL_Connect[Ex]()
 *       
 *    3.Perform URL encoding/decoding of data:
 *       URL_Encode()
 *       URL_Decode[Ex]()
 *
 *    5.Compose or parse NCBI-specific Content-Type's:
 *       EMIME_Type
 *       EMIME_SubType
 *       EMIME_Encoding
 *       MIME_ComposeContentType()
 *       MIME_ParseContentType()
 *
 *    6.Search for a token in the input stream (either CONN or SOCK):
 *       CONN_StripToPattern()
 *       SOCK_StripToPattern()
 *       BUF_StripToPattern()
 *
 */

#include <connect/ncbi_buffer.h>
#include <connect/ncbi_connection.h>
#include <connect/ncbi_socket.h>


/** @addtogroup UtilityFunc
 *
 * @{
 */


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    eURL_Unspec = 0,
    eURL_Https,
    eURL_Http,
    eURL_File,
    eURL_Ftp
} EURLScheme;


typedef enum {
    eReqMethod_Any = 0,
    eReqMethod_Post,
    eReqMethod_Get
} EReqMethod;


typedef enum {
    eDebugPrintout_None = 0,
    eDebugPrintout_Some,
    eDebugPrintout_Data
} EDebugPrintout;


/* Network connection related configurable info struct.
 * ATTENTION:  Do NOT fill out this structure (SConnNetInfo) "from scratch"!
 *             Instead, use ConnNetInfo_Create() described below to create
 *             it, and then fix (hard-code) some fields, if really necessary.
 * NOTE:       "scheme", "user", and "pass" are reserved (unused) fields.
 */
typedef struct {
    char           client_host[256]; /* effective client hostname ('\0'=def) */
    EURLScheme     scheme;           /* only pre-defined types (limited)     */
    char           user[128];        /* username (if specified)              */
    char           pass[128];        /* password (if any, clear text!!!)     */
    char           host[256];        /* host to connect to                   */
    unsigned short port;             /* port to connect to, host byte order  */
    char           path[1024];       /* service: path(e.g. to  a CGI script) */
    char           args[1024];       /* service: args(e.g. for a CGI script) */
    EReqMethod     req_method;       /* method to use in the request (HTTP)  */
    STimeout*      timeout;          /* ptr to i/o tmo (infinite if NULL)    */
    unsigned short max_try;          /* max. # of attempts to connect (>= 1) */
    char           http_proxy_host[256]; /* hostname of HTTP proxy server    */
    unsigned short http_proxy_port;      /* port #   of HTTP proxy server    */
    char           proxy_host[256];  /* CERN-like (non-transp) f/w proxy srv */
    EDebugPrintout debug_printout;   /* printout some debug info             */
    int/*bool*/    stateless;        /* to connect in HTTP-like fashion only */
    int/*bool*/    firewall;         /* to use firewall/relay in connects    */
    int/*bool*/    lb_disable;       /* to disable local load-balancing      */
    const char*    http_user_header; /* user header to add to HTTP request   */
    const char*    http_referer;     /* default referrer (when not spec'd)   */

    /* the following field(s) are for the internal use only -- don't touch!  */
    int/*bool*/    http_proxy_adjusted;
    STimeout       tmo;              /* default storage for finite timeout   */
    const char*    service;          /* service for which this info created  */
} SConnNetInfo;


/* Defaults and the registry entry names for "SConnNetInfo" fields
 */
#define DEF_CONN_REG_SECTION      "CONN"

#define REG_CONN_SCHEME           "SCHEME"
#define DEF_CONN_SCHEME           0

#define REG_CONN_USER             "USER"
#define DEF_CONN_USER             ""

#define REG_CONN_PASS             "PASS"
#define DEF_CONN_PASS             ""

#define REG_CONN_HOST             "HOST"
#define DEF_CONN_HOST             "www.ncbi.nlm.nih.gov"

#define REG_CONN_PORT             "PORT"
#define DEF_CONN_PORT             0

#define REG_CONN_PATH             "PATH"
#define DEF_CONN_PATH             "/Service/dispd.cgi"

#define REG_CONN_ARGS             "ARGS"
#define DEF_CONN_ARGS             ""

#define REG_CONN_REQ_METHOD       "REQ_METHOD"
#define DEF_CONN_REQ_METHOD       "ANY"

#define REG_CONN_TIMEOUT          "TIMEOUT"
#define DEF_CONN_TIMEOUT          30.0

#define REG_CONN_MAX_TRY          "MAX_TRY"
#define DEF_CONN_MAX_TRY          3

#define REG_CONN_HTTP_PROXY_HOST  "HTTP_PROXY_HOST"
#define DEF_CONN_HTTP_PROXY_HOST  ""

#define REG_CONN_HTTP_PROXY_PORT  "HTTP_PROXY_PORT"
#define DEF_CONN_HTTP_PROXY_PORT  ""

#define REG_CONN_PROXY_HOST       "PROXY_HOST"
#define DEF_CONN_PROXY_HOST       ""

#define REG_CONN_DEBUG_PRINTOUT   "DEBUG_PRINTOUT"
#define DEF_CONN_DEBUG_PRINTOUT   ""

#define REG_CONN_STATELESS        "STATELESS"
#define DEF_CONN_STATELESS        ""

#define REG_CONN_FIREWALL         "FIREWALL"
#define DEF_CONN_FIREWALL         ""

#define REG_CONN_LB_DISABLE       "LB_DISABLE"
#define DEF_CONN_LB_DISABLE       ""

#define REG_CONN_HTTP_USER_HEADER "HTTP_USER_HEADER"
#define DEF_CONN_HTTP_USER_HEADER ""

#define REG_CONN_HTTP_REFERER     "HTTP_REFERER"
#define DEF_CONN_HTTP_REFERER     0

/* Environment/registry keys that are *not* kept in SConnNetInfo */
#define REG_CONN_SERVICE_NAME     "SERVICE_NAME"
#define REG_CONN_LOCAL_ENABLE     "LOCAL_ENABLE"
#define REG_CONN_LBSMD_DISABLE    "LBSMD_DISABLE"
#define REG_CONN_DISPD_DISABLE    "DISPD_DISABLE"

/* Local service dispatcher */
#define REG_CONN_LOCAL_SERVICES   "LOCAL_SERVICES"
#define REG_CONN_LOCAL_SERVER     DEF_CONN_REG_SECTION "_LOCAL_SERVER"


extern NCBI_XCONNECT_EXPORT const char* ConnNetInfo_GetValue
(const char* service,
 const char* param,
 char*       value,
 size_t      value_size,
 const char* def_value
 );


/* This function to fill out the "*info" structure using
 * registry entries named (see above) in macros REG_CONN_<NAME>:
 *
 *  -- INFO FIELD --  ----- NAME -----  ---------- REMARKS/EXAMPLES ---------
 *  client_host       local host name   assigned automatically
 *  service_name      SERVICE_NAME      no search/no value without service
 *  host              HOST
 *  port              PORT
 *  path              PATH
 *  args              ARGS
 *  req_method        REQ_METHOD
 *  timeout           TIMEOUT           "<sec>.<usec>": "3.00005", "infinite"
 *  max_try           MAX_TRY  
 *  http_proxy_host   HTTP_PROXY_HOST   no HTTP proxy if empty/NULL
 *  http_proxy_port   HTTP_PROXY_PORT
 *  proxy_host        PROXY_HOST
 *  debug_printout    DEBUG_PRINTOUT
 *  stateless         STATELESS
 *  firewall          FIREWALL
 *  lb_disable        LB_DISABLE
 *  http_user_header  HTTP_USER_HEADER  "\r\n" if missing is appended
 *  http_referer      HTTP_REFERER      may be assigned automatically
 *
 * A value of the field NAME is first looked up in the environment variable
 * of the form service_CONN_<NAME>; then in the current corelib registry,
 * in the section 'service' by using key CONN_<NAME>; then in the environment
 * variable again, but using the name CONN_<NAME>; and finally in the default
 * registry section (DEF_CONN_REG_SECTION), using just <NAME>. If service
 * is NULL or empty then the first 2 steps in the above lookup are skipped.
 *
 * For default values see right above, in macros DEF_CONN_<NAME>.
 */
extern NCBI_XCONNECT_EXPORT SConnNetInfo* ConnNetInfo_Create
(const char* service
 );


/* Adjust the "host:port" to "proxy_host:proxy_port", and
 * "path" to "http://host:port/path" to connect through an HTTP proxy.
 * Return FALSE if cannot adjust (e.g. if "host" + "path" are too long).
 * NOTE:  it does nothing if applied more than once to the same "info"
 *        (or its clone), or when "http_proxy_host" is empty, but
 *        returns TRUE.
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ ConnNetInfo_AdjustForHttpProxy
(SConnNetInfo* info
 );


/* Make an exact and independent copy of "*info".
 */
extern NCBI_XCONNECT_EXPORT SConnNetInfo* ConnNetInfo_Clone
(const SConnNetInfo* info
 );


/* Convenience routines to manipulate SConnNetInfo::args[].
 * In "arg" all routines below assume to have a single arg name
 * or an "arg=value" pair.  In the former case, additional "val"
 * may be supplied separately (and will be prepended by "=" if
 * necessary).  In the latter case, having a non-zero string in
 * "val" may result in an erroneous behavior.  Ampersand (&) gets
 * automatically added to keep the arg list correct.
 * Return value (if any): none-zero on success; 0 on error.
 */

/* append argument to the end of the list */
extern NCBI_XCONNECT_EXPORT int/*bool*/ ConnNetInfo_AppendArg
(SConnNetInfo* info,
 const char*   arg,
 const char*   val
 );

/* put argument in the front of the list */
extern NCBI_XCONNECT_EXPORT int/*bool*/ ConnNetInfo_PrependArg
(SConnNetInfo* info,
 const char*   arg,
 const char*   val
 );

/* delete one (first) argument from the list of arguments in "info" */
extern NCBI_XCONNECT_EXPORT void ConnNetInfo_DeleteArg
(SConnNetInfo* info,
 const char*   arg
 );

/* delete all arguments specified in "args" from the list in "info" */
extern NCBI_XCONNECT_EXPORT void ConnNetInfo_DeleteAllArgs
(SConnNetInfo* info,
 const char*   args
 );

/* same as sequence DeleteAll(arg) then Prepend(arg, val), see above */
extern NCBI_XCONNECT_EXPORT int/*bool*/ ConnNetInfo_PreOverrideArg
(SConnNetInfo* info,
 const char*   arg,
 const char*   val
 );

/* same as sequence DeleteAll(arg) then Append(arg, val), see above */
extern NCBI_XCONNECT_EXPORT int/*bool*/ ConnNetInfo_PostOverrideArg
(SConnNetInfo* info,
 const char*   arg,
 const char*   val
 );


/* Set user header (discard previously set header, if any).
 * Reset the old header (if any) if "header" == NULL.
 * Return non-zero if successful, otherwise return 0 to indicate an error.
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ ConnNetInfo_SetUserHeader
(SConnNetInfo* info,
 const char*   header
 );


/* Append user header (same as ConnNetInfo_SetUserHeader() if no previous
 * header was set, or if "header" == NULL).
 * Return non-zero if successful, otherwise return 0 to indicate an error.
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ ConnNetInfo_AppendUserHeader
(SConnNetInfo* info,
 const char*   header
 );


/* Override user header.
 * Tags replaced (case-insensitively), and tags with empty values effectively
 * delete existing tags from the old user header, e.g. "My-Tag:\r\n" deletes
 * any appearence (if any) of "My-Tag: [<value>]" from the user header.
 * Unmatched tags with non-empty values are simply added to the existing user
 * header (as with "Append" above).
 * Return non-zero if successful, otherwise return 0 to indicate an error.
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ ConnNetInfo_OverrideUserHeader
(SConnNetInfo* info,
 const char*   header
 );


/* Extend user header.
 * Existings tags matching (case-insensitively) those from "header" are
 * appended with new value (separated by a comma and a space) if the added
 * value is non-empty, otherwise, the tags are left untouched. All new
 * unmatched tags from "header" with non-empty values get added to the end
 * of the user header.
 * Return non-zero if successful, otherwise return 0 to indicate an error.
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ ConnNetInfo_ExtendUserHeader
(SConnNetInfo* info,
 const char*   header
 );


/* Delete entries from current user header, if their tags match those
 * passed in "hdr" (regardless of the values, if any, in the latter).
 */
extern NCBI_XCONNECT_EXPORT void ConnNetInfo_DeleteUserHeader
(SConnNetInfo* info,
 const char*   hdr
 );


/* Parse URL into "*info", using (service-specific, if any) defaults.
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ ConnNetInfo_ParseURL
(SConnNetInfo* info,
 const char*   url
 );


/* Setup standard arguments:  service(as passed), address, and platform.
 * Return non-zero on success; zero on error.
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ ConnNetInfo_SetupStandardArgs
(SConnNetInfo* info,
 const char*   service
 );


/* Log the contents of "*info".
 */
extern NCBI_XCONNECT_EXPORT void ConnNetInfo_Log
(const SConnNetInfo* info,
 LOG                 log
 );


/* Destroy and deallocate "info" (if not NULL).
 */
extern NCBI_XCONNECT_EXPORT void ConnNetInfo_Destroy(SConnNetInfo* info);



/* Hit URL "http[s]://host[:port]/path?args" with the following
 * request (argument substitution enclosed in angle brackets, with
 * optional parts in square brackets):
 *
 *    {POST|GET} <path>[?<args>] HTTP/1.0\r\n
 *    Host: <host>[:port]
 *    [<user_header>]
 *    Content-Length: <content_length>\r\n
 *
 * Request method eReqMethod_Any selects appropriate method depending on
 * the passed value of "content_length":  GET when no content is expected
 * (content_length==0), and POST when "content_length" provided non-zero. 
 *
 * If "port" is not specified (0) it will be assigned automatically
 * to a well-known value depending on the setting of fSOCK_Secure in
 * the passed "flags" parameter.
 *
 * The "content_length" is mandatory, and it specifies an exact(!) amount of
 * data that you are planning to send to the resultant socket (0 if none).
 *
 * If string "user_header" is not NULL/empty, then it *must* be terminated
 * by a single '\r\n'.
 *
 * If "encode_args" is TRUE then URL-encode the "args".
 * "args" can be NULL/empty -- then the '?' symbol does not get added.
 *
 * On success, return eIO_Success and non-NULL handle of a socket via last
 * parameter.
 * ATTENTION:  due to the very essence of the HTTP connection, you may
 *             perform only one { WRITE, ..., WRITE, READ, ..., READ } cycle.
 * Returned socket must be closed exipicitly by "ncbi_socket.h:SOCK_Close()"
 * when no longer needed.
 * On error, return specific code (last parameter may not be updated),
 * no socket gets created.
 *
 * NOTE: Returned socket may not be immediately readable/writeable if open
 *       and/or read/write timeouts were passed as {0,0}, meaning that both
 *       connection and HTTP header write operation may still be pending in
 *       the resultant socket. It is responsibility of the application to
 *       analyze the actual socket state in this case (see "ncbi_socket.h").
 */

extern NCBI_XCONNECT_EXPORT EIO_Status URL_ConnectEx
(const char*     host,            /* must be provided                        */
 unsigned short  port,            /* may be 0, defaulted to either 80 or 443 */
 const char*     path,            /* must be provided                        */
 const char*     args,            /* may be NULL or empty                    */
 EReqMethod      req_method,      /* ANY selects method by "content_length"  */
 size_t          content_length,
 const STimeout* c_timeout,       /* timeout for the CONNECT stage           */
 const STimeout* rw_timeout,      /* timeout for READ and WRITE              */
 const char*     user_header,
 int/*bool*/     encode_args,     /* URL-encode the "args", if any           */
 TSOCK_Flags     flags,           /* additional socket requirements          */
 SOCK*           sock             /* returned socket (on eIO_Success only)   */
 );

/* Equivalent to the above except that it returns non-NULL socket handle
 * on success, and NULL on error without providing a reason for the failure. */
extern NCBI_XCONNECT_EXPORT SOCK URL_Connect
(const char*     host,            /* must be provided                        */
 unsigned short  port,            /* may be 0, defaulted to either 80 or 443 */
 const char*     path,            /* must be provided                        */
 const char*     args,            /* may be NULL or empty                    */
 EReqMethod      req_method,      /* ANY selects method by "content_length"  */
 size_t          content_length,
 const STimeout* c_timeout,       /* timeout for the CONNECT stage           */
 const STimeout* rw_timeout,      /* timeout for READ and WRITE              */
 const char*     user_header,
 int/*bool*/     encode_args,     /* URL-encode the "args", if any           */
 TSOCK_Flags     flags            /* additional socket requirements          */
 );


/* Discard all input data before(and including) the first occurrence of
 * "pattern". If "buf" is not NULL then add the discarded data(including
 * the "pattern") to it. If "n_discarded" is not NULL then "*n_discarded"
 * will return # of discarded bytes.
 * NOTE: "pattern" == NULL causes stripping to the EOF.
 */
extern NCBI_XCONNECT_EXPORT EIO_Status CONN_StripToPattern
(CONN        conn,
 const void* pattern,
 size_t      pattern_size,
 BUF*        buf,
 size_t*     n_discarded
 );

extern NCBI_XCONNECT_EXPORT EIO_Status SOCK_StripToPattern
(SOCK        sock,
 const void* pattern,
 size_t      pattern_size,
 BUF*        buf,
 size_t*     n_discarded
 );

extern NCBI_XCONNECT_EXPORT EIO_Status BUF_StripToPattern
(BUF         buffer,
 const void* pattern,
 size_t      pattern_size,
 BUF*        buf,
 size_t*     n_discarded
 );



/* URL-encode up to "src_size" symbols(bytes) from buffer "src_buf".
 * Write the encoded data to buffer "dst_buf", but no more than "dst_size"
 * bytes.
 * Assign "*src_read" to the # of bytes successfully encoded from "src_buf".
 * Assign "*dst_written" to the # of bytes written to buffer "dst_buf".
 */
extern NCBI_XCONNECT_EXPORT void URL_Encode
(const void* src_buf,    /* [in]     non-NULL */
 size_t      src_size,   /* [in]              */
 size_t*     src_read,   /* [out]    non-NULL */
 void*       dst_buf,    /* [in/out] non-NULL */
 size_t      dst_size,   /* [in]              */
 size_t*     dst_written /* [out]    non-NULL */
 );


/* URL-decode up to "src_size" symbols(bytes) from buffer "src_buf".
 * Write the decoded data to buffer "dst_buf", but no more than "dst_size"
 * bytes.
 * Assign "*src_read" to the # of bytes successfully decoded from "src_buf".
 * Assign "*dst_written" to the # of bytes written to buffer "dst_buf".
 * Return FALSE (0) only if cannot decode anything, and an unrecoverable
 * URL-encoding error (such as an invalid symbol or a bad "%.." sequence)
 * has occurred.
 * NOTE:  the unfinished "%.." sequence is fine -- return TRUE, but dont
 *        "read" it.
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ URL_Decode
(const void* src_buf,    /* [in]     non-NULL */
 size_t      src_size,   /* [in]              */
 size_t*     src_read,   /* [out]    non-NULL */
 void*       dst_buf,    /* [in/out] non-NULL */
 size_t      dst_size,   /* [in]              */
 size_t*     dst_written /* [out]    non-NULL */
 );


/* Act just like URL_Decode (see above) but caller can allow the specified
 * non-standard URL symbols in the input buffer to be decoded "as is".
 * The extra allowed symbols are passed in a '\0'-terminated string
 * "allow_symbols" (it can be NULL or empty -- then this will be an exact
 * equivalent of URL_Decode).
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ URL_DecodeEx
(const void* src_buf,      /* [in]     non-NULL  */
 size_t      src_size,     /* [in]               */
 size_t*     src_read,     /* [out]    non-NULL  */
 void*       dst_buf,      /* [in/out] non-NULL  */
 size_t      dst_size,     /* [in]               */
 size_t*     dst_written,  /* [out]    non-NULL  */
 const char* allow_symbols /* [in]     '\0'-term */
 );



/****************************************************************************
 * NCBI-specific MIME content type and sub-types
 * (the API to compose and parse them)
 *    Content-Type: <type>/<MIME_ComposeSubType()>\r\n
 *
 *    Content-Type: <type>/<subtype>-<encoding>\r\n
 *
 * where  MIME_ComposeSubType(EMIME_SubType subtype, EMIME_Encoding encoding):
 *   "x-<subtype>-<encoding>":
 *     "x-<subtype>",   "x-<subtype>-urlencoded",   "x-<subtype>-<encoding>",
 *     "x-dispatch",    "x-dispatch-urlencoded",    "x-dispatch-<encoding>
 *     "x-asn-text",    "x-asn-text-urlencoded",    "x-asn-text-<encoding>
 *     "x-asn-binary",  "x-asn-binary-urlencoded",  "x-asn-binary-<encoding>"
 *     "x-www-form",    "x-www-form-urlencoded",    "x-www-form-<encoding>"
 *     "html",          "html-urlencoded",          "html-<encoding>"
 *     "x-unknown",     "x-unknown-urlencoded",     "x-unknown-<encoding>"
 *
 *  Note:  <subtype> and <encoding> are expected to contain only
 *         alphanumeric symbols, '-' and '_'. They are case-insensitive.
 ****************************************************************************/


/* Type
 */
typedef enum {
    eMIME_T_Undefined = -1,
    eMIME_T_NcbiData = 0,  /* "x-ncbi-data"  (NCBI specific data) */
    eMIME_T_Text,          /* "text"                              */
    eMIME_T_Application,   /* "application"                       */
    /* eMIME_T_???, "<type>" here go other types                  */
    eMIME_T_Unknown        /* "unknown"                           */
} EMIME_Type;


/* SubType
 */
typedef enum {
    eMIME_Undefined = -1,
    eMIME_Dispatch = 0,  /* "x-dispatch"    (dispatcher info)          */
    eMIME_AsnText,       /* "x-asn-text"    (text ASN.1 data)          */
    eMIME_AsnBinary,     /* "x-asn-binary"  (binary ASN.1 data)        */
    eMIME_Fasta,         /* "x-fasta"       (data in FASTA format)     */
    eMIME_WwwForm,       /* "x-www-form"                               */
    /* standard MIMEs */
    eMIME_Html,          /* "html"                                     */
    eMIME_Plain,         /* "plain"                                    */
    eMIME_Xml,           /* "xml"                                      */
    eMIME_XmlSoap,       /* "xml+soap"                                 */
    /* eMIME_???,           "<subtype>" here go other NCBI subtypes    */
    eMIME_Unknown        /* "x-unknown"     (an arbitrary binary data) */
} EMIME_SubType;


/* Encoding
 */
typedef enum {
    eENCOD_None = 0, /* ""              (the content is passed "as is") */
    eENCOD_Url,      /* "-urlencoded"   (the content is URL-encoded)    */
    /* eENCOD_???,      "-<encoding>" here go other NCBI encodings      */
    eENCOD_Unknown   /* "-encoded"      (unknown encoding)              */
} EMIME_Encoding;


/* Write up to "buflen" bytes to "buf":
 *   Content-Type: <type>/[x-]<subtype>-<encoding>\r\n
 * Return pointer to the "buf".
 */
#define MAX_CONTENT_TYPE_LEN 64
extern NCBI_XCONNECT_EXPORT char* MIME_ComposeContentTypeEx
(EMIME_Type     type,
 EMIME_SubType  subtype,
 EMIME_Encoding encoding,
 char*          buf,
 size_t         buflen    /* must be at least MAX_CONTENT_TYPE_LEN */
 );

/* Parse the NCBI-specific content-type; the (case-insensitive) "str"
 * can be in the following two formats:
 *   Content-Type: <type>/x-<subtype>-<encoding>
 *   <type>/x-<subtype>-<encoding>
 *
 * NOTE:  all leading spaces and all trailing spaces (and any trailing symbols,
 *        if they separated from the content type by at least one space) will
 *        be ignored, e.g. these are valid content type strings:
 *           "   Content-Type: text/plain  foobar"
 *           "  text/html \r\n  barfoo coocoo ....\n boooo"
 *
 * If it does not match any of NCBI MIME type/subtypes/encodings, then
 * return TRUE, eMIME_T_Unknown, eMIME_Unknown or eENCOD_None, respectively.
 * If the passed "str" has an invalid (non-HTTP ContentType) format
 * (or if it is NULL/empty), then
 * return FALSE, eMIME_T_Undefined, eMIME_Undefined, and eENCOD_None
 */
extern NCBI_XCONNECT_EXPORT int/*bool*/ MIME_ParseContentTypeEx
(const char*     str,      /* the HTTP "Content-Type:" header to parse */
 EMIME_Type*     type,     /* can be NULL */
 EMIME_SubType*  subtype,  /* can be NULL */
 EMIME_Encoding* encoding  /* can be NULL */
 );


#ifndef NCBI_DEPRECATED
#  define NCBI_CONNUTIL_DEPRECATED
#else
#  define NCBI_CONNUTIL_DEPRECATED NCBI_DEPRECATED
#endif

/* Exactly equivalent to MIME_ComposeContentTypeEx(eMIME_T_NcbiData, ...)
 * Use more explicit Ex variant instead.
 */
extern NCBI_XCONNECT_EXPORT NCBI_CONNUTIL_DEPRECATED
char* MIME_ComposeContentType
(EMIME_SubType  subtype,
 EMIME_Encoding encoding,
 char*          buf,
 size_t         buflen
 );

/* Requires the MIME type be "x-ncbi-data".
 * Use more explicit Ex variant instead.
 */
extern NCBI_XCONNECT_EXPORT NCBI_CONNUTIL_DEPRECATED
int/*bool*/ MIME_ParseContentType
(const char*     str,      /* the HTTP "Content-Type:" header to parse */
 EMIME_SubType*  subtype,  /* can be NULL */
 EMIME_Encoding* encoding  /* can be NULL */
 );


#ifdef __cplusplus
}  /* extern "C" */
#endif


/* @} */

#endif /* CONNECT___NCBI_CONNUTIL__H */
