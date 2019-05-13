/* -*- mode: c; c-basic-offset: 8; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=8:tabstop=8:
 */
/******************************************************************************\
*
* Implement of abstract I/O interface for uMMAP.
*
\******************************************************************************/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>              /* IO operations */
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>

#include "ior.h"
#include "aiori.h"
#include "iordef.h"
#include "utilities.h"
#include "ummap.h"

/**************************** P R O T O T Y P E S *****************************/
static void *uMMAP_Create(char *, IOR_param_t *);
static void *uMMAP_Open(char *, IOR_param_t *);
static IOR_offset_t uMMAP_Xfer(int, void *, IOR_size_t *,
                               IOR_offset_t, IOR_param_t *);
static void uMMAP_Close(void *, IOR_param_t *);
static void uMMAP_Fsync(void *, IOR_param_t *);

/************************** D E C L A R A T I O N S ***************************/

static struct
{
        size_t size_rank;
        off_t  file_offset;
} aiori_cfg = { 0 };

ior_aiori_t ummap_aiori = {
        .name = "uMMAP",
        .create = uMMAP_Create,
        .open = uMMAP_Open,
        .xfer = uMMAP_Xfer,
        .close = uMMAP_Close,
        .delete = POSIX_Delete,
        .get_version = aiori_get_version,
        .fsync = uMMAP_Fsync,
        .get_file_size = POSIX_GetFileSize,
};

/***************************** F U N C T I O N S ******************************/

static void update_settings(IOR_param_t *param)
{
        int rank = INT_MAX;
        
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        
        aiori_cfg.size_rank   = param->expectedAggFileSize / param->numTasks;
        aiori_cfg.file_offset = (off_t)rank * aiori_cfg.size_rank;
}

static void ior_ummap_file(int *fd, IOR_param_t *param)
{
        const size_t size     = param->expectedAggFileSize;
        const size_t seg_size = param->blockSize;
        const int    prot     = PROT_READ | (param->open == WRITE) * PROT_WRITE;

        if (ummap(size, seg_size, prot, *fd, 0, UINT_MAX, (param->open == READ),
                  0, &param->mmap_ptr) != 0)
                ERR("ummap() failed");

        return;
}

/*
 * Create and open a file through the POSIX interface, then setup ummap.
 */
static void *uMMAP_Create(char *testFileName, IOR_param_t * param)
{
        int *fd;

        update_settings(param);

        fd = POSIX_Create(testFileName, param);
        if (ftruncate(*fd, param->expectedAggFileSize) != 0)
                ERR("ftruncate() failed");
        ior_ummap_file(fd, param);
        return ((void *)fd);
}

/*
 * Open a file through the POSIX interface and setup ummap.
 */
static void *uMMAP_Open(char *testFileName, IOR_param_t * param)
{
        int *fd;

        update_settings(param);

        fd = POSIX_Open(testFileName, param);
        ior_ummap_file(fd, param);
        return ((void *)fd);
}

/*
 * Write or read access to file using ummap
 */
static IOR_offset_t uMMAP_Xfer(int access, void *file, IOR_size_t * buffer,
                               IOR_offset_t length, IOR_param_t * param)
{
        off_t offset = aiori_cfg.file_offset;
        
        if (param->randomOffset)
        {
                offset += (param->offset % aiori_cfg.size_rank);
        }
        
        if (access == WRITE) {
                memcpy(param->mmap_ptr + offset, buffer, length);
        } else {
                memcpy(buffer, param->mmap_ptr + offset, length);
        }

        if (param->fsyncPerWrite == TRUE) {
                if (umsync(param->mmap_ptr, TRUE) != 0)
                        ERR("umsync() failed");
        }
        
        if (!param->randomOffset)
        {
                aiori_cfg.file_offset += param->blockSize;
        }
        
        return (length);
}

/*
 * Perform msync().
 */
static void uMMAP_Fsync(void *fd, IOR_param_t * param)
{
        if (umsync(param->mmap_ptr, FALSE) != 0)
                EWARN("umsync() failed");
}

/*
 * Close a file through the POSIX interface, after tear down the ummap.
 */
static void uMMAP_Close(void *fd, IOR_param_t * param)
{
        if (umunmap(param->mmap_ptr, FALSE) != 0)
                ERR("umunmap failed");
        param->mmap_ptr = NULL;
        POSIX_Close(fd, param);
}
