/*  $Id: blast_hspstream.h 107839 2007-07-27 18:01:15Z papadopo $
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
 * Author:  Christiam Camacho
 *
 */

/** @file blast_hspstream.h
 * Declaration of ADT to save and retrieve lists of HSPs in the BLAST engine.
 */

#ifndef ALGO_BLAST_CORE__BLAST_HSPSTREAM_H
#define ALGO_BLAST_CORE__BLAST_HSPSTREAM_H

#include <algo/blast/core/blast_export.h>
#include <algo/blast/core/blast_hits.h>
#include <algo/blast/core/split_query.h>

#ifdef __cplusplus
extern "C" {
#endif

/** structure used to hold a collection of hits
    retrieved from the HSPStream */
typedef struct BlastHSPStreamResultBatch {
    Int4 num_hsplists;          /**< number of lists of HSPs returned */
    BlastHSPList **hsplist_array;  /**< array of HSP lists returned */
} BlastHSPStreamResultBatch;

/** create a new batch to hold HSP results 
 * @param num_hsplists Maximum number of results to hold
 * @return Pointer to newly allocated structure
 */
BlastHSPStreamResultBatch * Blast_HSPStreamResultBatchInit(
                                            Int4 num_hsplists);

/** free a batch of HSP results. Note that the HSPLists
 * themselves are not freed
 * @param batch Structure to free
 * @return Always NULL
 */
BlastHSPStreamResultBatch * Blast_HSPStreamResultBatchFree(
                                  BlastHSPStreamResultBatch *batch);

/** free the list of HSPLists within a batch
 * @param batch Structure to reset
 */
void Blast_HSPStreamResultBatchReset(BlastHSPStreamResultBatch *batch);

/** The BlastHSPStream ADT is an opaque data type that defines a thread-safe
 *  interface which is used by the core BLAST code to save lists of HSPs.
 *  The interface currently provides the following services:
 *  - Management of the ADT (construction, destruction)
 *  - Writing lists of HSPs to the ADT
 *  - Reading lists of HSPs from the ADT
 *  - Merging two instances of the ADT
 *  .
 *  The default implementation simply buffers HSPs from one stage of the
 *  algorithm to the next @sa FIXME
 *  Implementations of this interface should provide functions for all
 *  the functions listed above.
 */
typedef struct BlastHSPStream BlastHSPStream;

/** Function pointer typedef to create a new BlastHSPStream structure.
 * First argument is a pointer to the structure to be populated (allocated for
 * client implementations), second argument should be typecast'd to the 
 * correct type by user-defined constructor function */
typedef BlastHSPStream* (*BlastHSPStreamConstructor) (BlastHSPStream*, void*);

/** Function pointer typedef to deallocate a BlastHSPStream structure.
 * Argument is the BlastHSPStream structure to free, always returns NULL. */
typedef BlastHSPStream* (*BlastHSPStreamDestructor) (BlastHSPStream*);

/** Function pointer typedef to implement the read/write functionality of the
 * BlastHSPStream. The first argument is the BlastHSPStream structure used, 
 * second argument is the list of HSPs to be saved/read (reading assumes
 * ownership, writing releases ownership) */
typedef int (*BlastHSPStreamMethod) (BlastHSPStream*, BlastHSPList**);

/** Function pointer typedef to implement the batch read functionality of the
 * BlastHSPStream. The first argument is the BlastHSPStream structure used, 
 * second argument is the list of HSP listss that are read (reading assumes
 * ownership) */
typedef int (*BlastHSPStreamBatchReadMethod) (BlastHSPStream*, 
                                              BlastHSPStreamResultBatch*);

/** Function pointer typedef to implement the merge functionality of the
 * BlastHSPStream. The first argument is the BlastHSPStream structure to merge, 
 * second argument is the BlastHSPStream that will contain the combined
 * result. Note that this function does not lock either of it inputs */
typedef int (*BlastHSPStreamMergeFnType) (SSplitQueryBlk*, Uint4,
                                     BlastHSPStream*, BlastHSPStream*);

/** Function pointer typedef to implement the close functionality of the 
 * BlastHSPStream. Argument is a pointer to the structure to close for 
 * writing.
 */
typedef void (*BlastHSPStreamCloseFnType) (BlastHSPStream*);

/*****************************************************************************/

/** Structure that contains the information needed for BlastHSPStreamNew to 
 * fully populate the BlastHSPStream structure it returns */
typedef struct BlastHSPStreamNewInfo {
    BlastHSPStreamConstructor constructor; /**< User-defined function to 
                                           initialize a BlastHSPStream 
                                           structure */
    void* ctor_argument;                 /**< Argument to the above function */
} BlastHSPStreamNewInfo;

/** Allocates memory for a BlastHSPStream structure and then invokes the
 * constructor function defined in its first argument, passing the 
 * ctor_argument member of that same structure. If the constructor function
 * pointer is not set, NULL is returned.
 * @param bhsn_info Structure defining constructor and its argument to be
 *        invoked from this function [in]
 */
NCBI_XBLAST_EXPORT
BlastHSPStream* BlastHSPStreamNew(const BlastHSPStreamNewInfo* bhsn_info);

/** Frees the BlastHSPStream structure by invoking the destructor function set 
 * by the user-defined constructor function when the structure is initialized
 * (indirectly, by BlastHSPStreamNew). If the destructor function pointer is not
 * set, a memory leak could occur.
 * @param hsp_stream BlastHSPStream to free [in]
 * @return NULL
 */
NCBI_XBLAST_EXPORT
BlastHSPStream* BlastHSPStreamFree(BlastHSPStream* hsp_stream);

/** Closes the BlastHSPStream structure for writing. Any subsequent attempt
 * to write to the stream will return error.
 * @param hsp_stream The stream to close [in] [out]
 */
NCBI_XBLAST_EXPORT
void BlastHSPStreamClose(BlastHSPStream* hsp_stream);

/** Moves the HSPlists from an HSPStream into the list contained
 * by a second HSPStream
 * @param squery_blk Information needed to map HSPs from one HSPstream
 *                   to the combined HSPstream [in]
 * @param chunk_num Used to choose a subset of the information in
 *                  squery_blk [in]
 * @param hsp_stream The stream to merge [in][out]
 * @param combined_hsp_stream The stream that will contain the
 *         HSPLists of the first stream [in][out]
 */
NCBI_XBLAST_EXPORT
int BlastHSPStreamMerge(SSplitQueryBlk* squery_blk,
                        Uint4 chunk_num,
                        BlastHSPStream* hsp_stream,
                        BlastHSPStream* combined_hsp_stream);

/** Standard error return value for BlastHSPStream methods */
extern const int kBlastHSPStream_Error;

/** Standard success return value for BlastHSPStream methods */
extern const int kBlastHSPStream_Success;

/** Return value when the end of the stream is reached (applicable to read
 * method only) */
NCBI_XBLAST_EXPORT
extern const int kBlastHSPStream_Eof;

/** Invokes the user-specified write function for this BlastHSPStream
 * implementation.
 * @param hsp_stream The BlastHSPStream object [in]
 * @param hsp_list List of HSPs for the HSPStream to keep track of. The caller
 * releases ownership of the hsp_list [in]
 * @return kBlastHSPStream_Success on success, otherwise kBlastHSPStream_Error 
 */
NCBI_XBLAST_EXPORT
int BlastHSPStreamWrite(BlastHSPStream* hsp_stream, BlastHSPList** hsp_list);

/** Invokes the user-specified read function for this BlastHSPStream
 * implementation.
 * @param hsp_stream The BlastHSPStream object [in]
 * @param hsp_list List of HSPs for the HSPStream to return. The caller
 * acquires ownership of the hsp_list [out]
 * @return kBlastHSPStream_Success on success, kBlastHSPStream_Error, or
 * kBlastHSPStream_Eof on end of stream
 */
NCBI_XBLAST_EXPORT
int BlastHSPStreamRead(BlastHSPStream* hsp_stream, BlastHSPList** hsp_list);

/** Invokes the user-specified batch read function for this BlastHSPStream
 * implementation.
 * @param hsp_stream The BlastHSPStream object [in]
 * @param batch List of HSP listss for the HSPStream to return. The caller
 * acquires ownership of all HSP lists returned [out]
 * @return kBlastHSPStream_Success on success, kBlastHSPStream_Error, or
 * kBlastHSPStream_Eof on end of stream
 */
NCBI_XBLAST_EXPORT
int BlastHSPStreamBatchRead(BlastHSPStream* hsp_stream, 
                            BlastHSPStreamResultBatch* batch);

/*****************************************************************************/
/* The following enumeration and function are only of interest to implementors
 * of this interface */

/** Defines the methods supported by the BlastHSPStream ADT */
typedef enum EMethodName {
    eConstructor,       /**< Constructor for a BlastHSPStream implementation */
    eDestructor,        /**< Destructor for a BlastHSPStream implementation */
    eRead,              /**< Read from the BlastHSPStream */
    eBatchRead,         /**< Batch read from the BlastHSPStream */
    eWrite,             /**< Write to the BlastHSPStream */
    eMerge,             /**< Merge with a second BlastHSPStream */
    eClose,             /**< Close the BlastHSPStream for writing */ 
    eMethodBoundary     /**< Limit to facilitate error checking */
} EMethodName;

/** Union to encapsulate the supported methods on the BlastHSPStream interface
 */
typedef union BlastHSPStreamFunctionPointerTypes {
   /** Used for read/write function pointers */
   BlastHSPStreamMethod method;        
   
   /** Used for constructor function pointer */
   BlastHSPStreamConstructor ctor;     
   
   /** Used for destructor function pointer */
   BlastHSPStreamDestructor dtor;      

   /** Use for merge function pointer */
   BlastHSPStreamMergeFnType mergeFn;

   /** Use for merge function pointer */
   BlastHSPStreamBatchReadMethod batch_read;

   /** Use for close function pointer */
   BlastHSPStreamCloseFnType closeFn;
} BlastHSPStreamFunctionPointerTypes;

/** Sets implementation specific data structure 
 * @param hsp_stream structure to initialize [in]
 * @param data structure to assign to the hsp_stream [in]
 * @return kBlastHSPStream_Error if hsp_stream is NULL else,
 * kBlastHSPStream_Success;
 */
NCBI_XBLAST_EXPORT
int SetData(BlastHSPStream* hsp_stream, void* data);

/** Gets implementation specific data structure 
 * @param hsp_stream structure from which to obtain the internal data. It is
 * expected that the caller (implementation of BlastHSPStream) knows what type
 * to cast the return value to. [in]
 * @return pointer to internal data structure of the implementation of the
 * BlastHSPStream, or NULL if hsp_stream is NULL
 */
NCBI_XBLAST_EXPORT
void* GetData(BlastHSPStream* hsp_stream);

/** Use this function to set the pointers to functions implementing the various
 * methods supported in the BlastHSPStream interface 
 * @param hsp_stream structure to initialize [in]
 * @param name method for which a function pointer is being provided [in]
 * @param fnptr_type union containing the pointer to the function specified by
 * name [in]
 * @return kBlastHSPStream_Error if hsp_stream is NULL else,
 * kBlastHSPStream_Success;
 */
NCBI_XBLAST_EXPORT
int SetMethod(BlastHSPStream* hsp_stream, 
               EMethodName name,
               BlastHSPStreamFunctionPointerTypes fnptr_type);

#ifdef __cplusplus
}
#endif

#endif /* ALGO_BLAST_CORE__BLAST_HSPSTREAM_H */
