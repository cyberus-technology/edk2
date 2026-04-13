/** @file
  EFI_FILE_PROTOCOL.SetPosition() member function for the Virtio Filesystem
  driver.

  Copyright (C) 2020, Red Hat, Inc.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/MemoryAllocationLib.h> // FreePool()

#include "VirtioFsDxe.h"

EFI_STATUS
EFIAPI
VirtioFsSimpleFileSetPosition (
  IN EFI_FILE_PROTOCOL  *This,
  IN UINT64             Position
  )
{
  VIRTIO_FS_FILE                      *VirtioFsFile;
  VIRTIO_FS                           *VirtioFs;
  EFI_STATUS                          Status;
  VIRTIO_FS_FUSE_ATTRIBUTES_RESPONSE  FuseAttr;
  EFI_TPL                             CurrentTpl;

  VirtioFsFile = VIRTIO_FS_FILE_FROM_SIMPLE_FILE (This);
  CurrentTpl   = VirtioFsAcquireLock ();

  //
  // Directories can only be rewound, per spec.
  //
  if (VirtioFsFile->IsDirectory) {
    if (Position != 0) {
      VirtioFsReleaseLock (CurrentTpl);
      return EFI_UNSUPPORTED;
    }

    VirtioFsFile->FilePosition = 0;
    if (VirtioFsFile->FileInfoArray != NULL) {
      FreePool (VirtioFsFile->FileInfoArray);
      VirtioFsFile->FileInfoArray = NULL;
    }

    VirtioFsFile->SingleFileInfoSize = 0;
    VirtioFsFile->NumFileInfo        = 0;
    VirtioFsFile->NextFileInfo       = 0;
    VirtioFsReleaseLock (CurrentTpl);
    return EFI_SUCCESS;
  }

  //
  // Regular file.
  //
  if (Position < MAX_UINT64) {
    //
    // Caller is requesting absolute file position.
    //
    VirtioFsFile->FilePosition = Position;
    VirtioFsReleaseLock (CurrentTpl);
    return EFI_SUCCESS;
  }

  //
  // Caller is requesting a seek to EOF.
  //
  VirtioFs = VirtioFsFile->OwnerFs;
  Status   = VirtioFsFuseGetAttr (VirtioFs, VirtioFsFile->NodeId, &FuseAttr);
  if (EFI_ERROR (Status)) {
    VirtioFsReleaseLock (CurrentTpl);
    return Status;
  }

  VirtioFsFile->FilePosition = FuseAttr.Size;
  VirtioFsReleaseLock (CurrentTpl);
  return EFI_SUCCESS;
}
