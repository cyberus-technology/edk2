/** @file
  EFI_FILE_PROTOCOL.GetPosition() member function for the Virtio Filesystem
  driver.

  Copyright (C) 2020, Red Hat, Inc.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "VirtioFsDxe.h"

EFI_STATUS
EFIAPI
VirtioFsSimpleFileGetPosition (
  IN     EFI_FILE_PROTOCOL  *This,
  OUT UINT64                *Position
  )
{
  VIRTIO_FS_FILE  *VirtioFsFile;
  EFI_TPL         CurrentTpl;

  VirtioFsFile = VIRTIO_FS_FILE_FROM_SIMPLE_FILE (This);
  CurrentTpl   = VirtioFsAcquireLock ();
  if (VirtioFsFile->IsDirectory) {
    VirtioFsReleaseLock (CurrentTpl);
    return EFI_UNSUPPORTED;
  }

  *Position = VirtioFsFile->FilePosition;
  VirtioFsReleaseLock (CurrentTpl);
  return EFI_SUCCESS;
}
