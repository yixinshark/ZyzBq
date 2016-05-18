/*****************************************
 Copyright (c) 2001-2007
 Sigma Designs, Inc. All Rights Reserved
 Proprietary and Confidential
 *****************************************/
/**
  @file   rmmmimplementation.c
  @brief

  @author Sebastian Frias Feltrer
  @date   2007-10-12
*/

/*
  **********************************************
  DISCLAIMER:

  - THIS IS TEST CODE, provided as sample code
  to help you understand what you should do to
  develop your own application based in RMFP.

  - This is NOT production grade code; It is not
  even a library, so any API defined here, CAN
  and WILL CHANGE without notice.

  **********************************************
*/

#include "rmmmimplementation.h"
#include "rmlibcw/include/rmcriticalsections.h"


#if (EM86XX_MODE == EM86XX_MODEID_WITHHOST)
#include <execinfo.h>
#endif




// if set to 1 then tracing will be disabled
#define DISABLE_TRACE (0)

// if set to 1 RMMalloc's won't be initialised
#define DO_NOT_INIT_MEM (0)

// if set to 1 then basic checks will be performed when RMFree is called (this uses more memory)
#define ENABLE_BUFFER_OVERFLOW_CHECK 1
#define CHECK_WORD 0xbadc0ded

static inline RMuint32 RMLEBufToUint32(const RMuint8 *buf) {
	return (((RMuint32) buf[3] << 24) +
		((RMuint32) buf[2] << 16) +
		((RMuint32) buf[1] << 8) +
		(RMuint32)  buf[0]);
}

static inline void RMuint32ToLEBuf(RMuint32 val, RMuint8 *buf)
{
	buf[3] = (RMuint8)(val >> 24);
	buf[2] = (RMuint8)(val >> 16);
	buf[1] = (RMuint8)(val >>  8);
	buf[0] = (RMuint8)val;
}


// disabled in release mode
#if (DISABLE_TRACE || !defined(_DEBUG))


inline void *RMMalloc(RMuint32 size)
{
	return malloc(size);
}

inline void RMFree(void *ptr)
{
	free(ptr);
}

inline void RMCheckMemory(void)
{
	return;
}

inline void *RMRealloc(void *ptr, RMuint32 size)
{
	return realloc(ptr, size);
}

#else // DISABLE_TRACE


/* thread safety : malloc and free are suppose to be thread safe so we have to make
   them safe. This is when debugging heavy thread concurrency to be sure that malloc
   will not become an issue. */
#define THREAD_SAFETY (1)

#define MAX_TRACEABLE_ENTRIES 100000

#define FUNC_NAME_LEN 80

#define PRINT_BACKTRACE 0
#define BACKTRACE_DEPTH 10

#define ALLOC_DBG DISABLE

static RMint32 count = 0;
static RMuint32 total = 0;
static RMuint32 max_memory_used = 0;
static RMuint32 largest_requested_block = 0;
static RMuint32 memory_in_use = 0;
static RMuint32 overflows = 0;
static RMuint32 underflows = 0;
static RMuint32 size_mismatchs = 0;

#if (THREAD_SAFETY && !defined(WITHOUT_THREADS))
static RMuint8 RMMallocCSPlacement[MAX_PRIMITIVE_CRITICALSECTION_SIZE] = {0, };
static RMcriticalsection RMMallocCS = NULL;
#endif

struct allocated_type {
	RMuint32 size;
	void *ptr;
	RMuint8 callerName[FUNC_NAME_LEN];
};


static struct allocated_type allocated_entries[MAX_TRACEABLE_ENTRIES];

#if ((EM86XX_MODE == EM86XX_MODEID_WITHHOST) && (PRINT_BACKTRACE == 1))
static void print_trace(RMuint32 adjust, RMuint8 *callerName, RMuint32 maxNameSize)
{
	void *array[BACKTRACE_DEPTH];
	RMuint32 array_size;
	RMuint8 **stacked_up_func_names;
	RMuint32 i;

	array_size = (RMuint32)backtrace (array, BACKTRACE_DEPTH);

	if (array_size)
		stacked_up_func_names = (RMuint8 **)backtrace_symbols (array, array_size);
	else
		return;

	//+1: because the last entry in the stack is the caller of print_trace(...)
	//+adjust: because we want the name of the caller of RMMalloc (this shouldn't happen since it's supposed to be inlined)

	if (adjust + 1 < array_size) {


		for (i = adjust + 1; i < array_size; i++) {
			//printf("[depth %ld] '%s'\n", i, stacked_up_func_names[i]);
			RMDBGPRINT((ENABLE, "[depth %ld] '%s'\n", i, stacked_up_func_names[i]));
		}


		i = RMmin(maxNameSize - 1, strlen((char*)stacked_up_func_names[adjust+1]));
		memcpy(callerName, stacked_up_func_names[adjust+1], i);
		callerName[i+1] = '\0';

	}

	free(stacked_up_func_names);
}
#endif



inline void *RMMalloc(RMuint32 size)
{

	void *ptr = NULL;

	if (!size)
		return NULL;

#if (THREAD_SAFETY && !defined(WITHOUT_THREADS))
	// This is not thread safe but it is only for debug.
	if (!RMMallocCS)
		RMMallocCS = RMPlacementCreateCriticalSection(RMMallocCSPlacement);
	RMEnterCriticalSection(RMMallocCS);
#endif

#if ENABLE_BUFFER_OVERFLOW_CHECK
	{
		RMuint8 *pBuffer;

		pBuffer = (RMuint8*)malloc(size + (sizeof(RMuint32)*3));
		if (pBuffer) {
			RMuint8 *pBuf = pBuffer;

			RMuint32ToLEBuf(size, pBuf);
			pBuf += sizeof(RMuint32);

			RMuint32ToLEBuf(CHECK_WORD, pBuf);
			pBuf += sizeof(RMuint32);

			RMuint32ToLEBuf(CHECK_WORD, pBuf + size);

			ptr = (void*)pBuf;
		}
		else
			ptr = NULL;
	}
#else
	ptr = malloc(size);
#endif


	if (ptr) {
		RMuint32 current = 0;
		count++;
		
		for (current = 0; current < total; current++) {
			if (allocated_entries[current].ptr == NULL)
				break;
		}
		RMDBGLOG((ALLOC_DBG, "[+] [%5ld %5lu/%5lu] RMMalloc(%lu) @ 0x%08lx\n", count, current, total, size, (RMuint32)ptr));

#if ((EM86XX_MODE == EM86XX_MODEID_WITHHOST) && (PRINT_BACKTRACE == 1))
		print_trace(1, allocated_entries[current].callerName, FUNC_NAME_LEN);
#endif

		if (largest_requested_block < size)
			largest_requested_block = size;

		if (current < MAX_TRACEABLE_ENTRIES) {
			allocated_entries[current].size = size;
			allocated_entries[current].ptr = ptr;
			total++;

			memory_in_use += size;

			if (max_memory_used < memory_in_use)
				max_memory_used = memory_in_use;

		}
		else
			RMDBGLOG((ENABLE, "maximum number of traceable mallocs reached, no more mallocs are traced\n"));


#if !DO_NOT_INIT_MEM
		memset(ptr, 0x1, size);
#endif

	}
	else {

#if ((EM86XX_MODE == EM86XX_MODEID_WITHHOST) && (PRINT_BACKTRACE == 1))
		RMuint8 callerName[FUNC_NAME_LEN];

		RMDBGLOG((ENABLE, "RMMalloc of %lu bytes failed!!\n", size));

		print_trace(1, callerName, FUNC_NAME_LEN);
#else

		RMDBGLOG((ENABLE, "RMMalloc of %lu bytes failed!!\n", size));
#endif

	}

#if (THREAD_SAFETY && !defined(WITHOUT_THREADS))
	RMLeaveCriticalSection(RMMallocCS);
#endif

	return ptr;
}

inline void RMFree(void *ptr)
{
	RMuint32 i = 0;
	RMuint32 freed_entry_size = 0;
	RMbool found = FALSE;
#ifdef _DEBUG
	RMuint8 callerName[FUNC_NAME_LEN] = { 0, };
#endif

#if (THREAD_SAFETY && !defined(WITHOUT_THREADS))
	// Malloc always come before free
	RMEnterCriticalSection(RMMallocCS);
#endif

	count--;

	for (i = 0; i < total; i++) {
		if (allocated_entries[i].ptr == ptr) {
			memory_in_use -= allocated_entries[i].size;
			freed_entry_size = allocated_entries[i].size;
			allocated_entries[i].size = 0;
			allocated_entries[i].ptr = NULL;
			found = TRUE;
			break;
		}
	}



	RMDBGLOG((ALLOC_DBG, "RMFree 0x%08lx backtrace:\n", (RMuint32)ptr));

#if ((EM86XX_MODE == EM86XX_MODEID_WITHHOST) && (PRINT_BACKTRACE == 1))
	print_trace(1, callerName, FUNC_NAME_LEN);
#endif


	if (found) {

		RMDBGLOG((ALLOC_DBG, "[-] [%5ld %5lu/%5lu] RMFree 0x%08lx size %lu; allocatedIn '%s'; freedIn '%s'\n",
			  count,
			  i,
			  total,
			  (RMuint32)ptr,
			  freed_entry_size,
			  allocated_entries[i].callerName,
			  callerName));
	}
	else
		RMDBGLOG((ALLOC_DBG, "[-] [%5ld %5lu] RMFree 0x%08lx (not traced release)\n", count, total, (RMuint32)ptr));

	RMDBGLOG((ALLOC_DBG, "____RMFree 0x%08lx\n", (RMuint32)ptr));


#if ENABLE_BUFFER_OVERFLOW_CHECK
	{
		RMuint8 *pBuffer;
		RMuint32 CheckWord = CHECK_WORD;
		RMuint32 StoredSize;
		RMuint32 StoredCheckWord;

		pBuffer = (RMuint8*)ptr;

		// adjust 'ptr' to the real allocated PTR so that it can be freed
		ptr = (void*)(pBuffer - (sizeof(RMuint32)*2));

		StoredCheckWord = RMLEBufToUint32(pBuffer - sizeof(RMuint32));
		StoredSize      = RMLEBufToUint32(pBuffer - (sizeof(RMuint32)*2));

#if 0
		{
			RMuint32 i;
			RMuint8  *buffer = (RMuint8*)ptr;
			RMuint32 bufferSize = RMmin(64, (freed_entry_size + (sizeof(RMuint32)*3)));

			RMDBGLOG((ENABLE, "\n********* HEX\n"));
			for (i = 0; i < bufferSize; i++) {
				if ((i % 16) == 0)
					RMDBGPRINT((ENABLE, "\n[%05lu] ", i));

				RMDBGPRINT((ENABLE, "%02x ", buffer[i]));
			}
			RMDBGPRINT((ENABLE, "\n"));
			RMDBGLOG((ENABLE, "********* HEX\n"));
		}
#endif


		if (StoredCheckWord != CheckWord) {
			RMNOTIFY((NULL, RM_ERROR, "!!!!RMFree 0x%08lx: Consistency check 1 failed, read 0x%08lx, expected 0x%08lx (Probably underflow)\n", (RMuint32)pBuffer, StoredCheckWord, CheckWord));
			underflows++;
		}
		else {
			// check size and perform more checks

			if (StoredSize != freed_entry_size) {
				RMNOTIFY((NULL, RM_ERROR, "!!!!RMFree 0x%08lx: Consistency check 2 failed (TracedSize %ld != PTRSize %ld)\n", (RMuint32)pBuffer, freed_entry_size, StoredSize));
				size_mismatchs++;
			}
			else {
				StoredCheckWord = RMLEBufToUint32(pBuffer + StoredSize);
				if (StoredCheckWord != CheckWord) {
					RMNOTIFY((NULL, RM_ERROR, "!!!!RMFree 0x%08lx: Consistency check 3 failed, read 0x%08lx, expected 0x%08lx (Probably overflow)\n", (RMuint32)pBuffer, StoredCheckWord, CheckWord));
					overflows++;
				}

			}

		}

	}
#endif

	free(ptr);

#if (THREAD_SAFETY && !defined(WITHOUT_THREADS))
	RMLeaveCriticalSection(RMMallocCS);
#endif
}


inline void RMCheckMemory(void)
{
	RMuint32 i = 0, j = 0;

#if (THREAD_SAFETY && !defined(WITHOUT_THREADS))
	// Release the critical section here
	RMPlacementDeleteCriticalSection(RMMallocCS);
#endif


	if ((count) || (total)) {
		RMDBGLOG((ENABLE, "malloc/free count %ld (zero if balanced)\n", count));
		RMDBGLOG((ENABLE, "total mallocs %lu, max_memory_used %lu, largest block %lu\n", total, max_memory_used, largest_requested_block));
	}

	for (i = 0; i < total; i++) {
		if (allocated_entries[i].ptr) {
			RMDBGLOG((ENABLE, "entry %lu: ptr 0x%08lx, size %6lu, callerName '%s'\n",
				  i,
				  (RMuint32)allocated_entries[i].ptr,
				  allocated_entries[i].size,
				  allocated_entries[i].callerName));

			j++;
		}
	}
	if (j)
		RMDBGLOG((ENABLE, "%lu orphan mallocs\n", j));

	if (!j && count)
		RMDBGLOG((ENABLE, "WARNING: malloc/free count is unbalanced but we were not able to trace it, potential memory leak\n"));

	if (underflows || overflows || size_mismatchs) {
		RMNOTIFY((NULL, RM_ERROR, "WARNING: over/under flow check failed! (overflows %lu, underflows %lu, size_mismatchs %lu)\n",
			  overflows,
			  underflows,
			  size_mismatchs));
	}

}

inline void *RMRealloc(void *ptr, RMuint32 size)
{

	void *newptr = NULL;
#if ENABLE_BUFFER_OVERFLOW_CHECK
	RMuint32 OldSize;
#endif

	if (!ptr && size) {
		/* Malloc */
		return RMMalloc(size);
	}

	if (ptr && !size) {
		RMFree(ptr);
		return NULL;
	}

	if (!ptr && !size) {
		RMDBGLOG((ENABLE, "ERROR: RMRealloc(%p, %lu) not valid\n", ptr, size));
		return NULL;
	}

#if ENABLE_BUFFER_OVERFLOW_CHECK
	/* Allocate the new buffer */
	newptr = RMMalloc(size);
	if (!newptr)
		return NULL;

	/* Get the old size */
	OldSize=RMmin(RMLEBufToUint32(ptr-8), size);

	/* Copy the memory */
	memcpy(newptr, ptr, OldSize);

	/* Free the old pointer */
	RMFree(ptr);

#else
	newptr = realloc(ptr, size);

	if (newptr) {
		RMuint32 i = 0;
		RMbool found = FALSE;
#ifdef _DEBUG
		RMuint8 callerName[FUNC_NAME_LEN] = { 0, };
#endif

		for (i = 0; i < total; i++) {
			if (allocated_entries[i].ptr == ptr) {
				memory_in_use = memory_in_use - allocated_entries[i].size + size;
				//memory_in_use -= allocated_entries[i].size;
				allocated_entries[i].size = size;
				allocated_entries[i].ptr = newptr;
				found = TRUE;
				break;
			}
		}



		RMDBGLOG((ALLOC_DBG, "RMRealloc 0x%08lx to 0x%08lx backtrace:\n", (RMuint32)ptr, (RMuint32)newptr));

#if ((EM86XX_MODE == EM86XX_MODEID_WITHHOST) && (PRINT_BACKTRACE == 1))
		print_trace(1, callerName, FUNC_NAME_LEN);
#endif


		if (found) {

			RMDBGLOG((ALLOC_DBG, "[- +] [%5ld %5lu] RMRealloc 0x%08lx to 0x%08lx size %lu; allocatedIn '%s'; freedIn '%s'\n",
				  count,
				  total,
				  (RMuint32)ptr,
				  (RMuint32)newptr,
				  allocated_entries[i].size,
				  allocated_entries[i].callerName,
				  callerName));
		}
		else
			RMDBGLOG((ALLOC_DBG, "[- +] [%5ld %5lu] RMRealloc 0x%08lx to 0x%08lx (not traced release)\n", count, total, (RMuint32)ptr, (RMuint32)newptr));

		RMDBGLOG((ALLOC_DBG, "____RMRealloc 0x%08lx\n", (RMuint32)newptr));
	}
#endif
	return newptr;
}

#endif // DISABLE_TRACE

inline void *RMMemset(void *buffer, RMuint8 byte, RMuint32 size)
{
	return memset(buffer, byte, size);
}

inline void *RMMemcpy(void *dest, const void *src, RMuint32 size)
{
	return memcpy(dest, src, size);
}

inline RMint32 RMMemcmp(const void *buffer1, const void *buffer2, RMuint32 size)
{
	return memcmp(buffer1, buffer2, size);
}

inline void *RMCalloc(RMuint32 elements, RMuint32 elementSize)
{
	void *ptr = RMMalloc(elements * elementSize);
	if (ptr != NULL)
		RMMemset(ptr, 0, elements * elementSize);
	return ptr;
}

