// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2014 Red Hat
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "FS.h"

#include "XFS.h"

#include "acconfig.h"

// ---------------

FS *FS::create(uint64_t f_type)
{
  switch (f_type) {
#ifdef HAVE_LIBXFS
  case XFS_SUPER_MAGIC:
    return new XFS;
#endif
  default:
    return new FS;
  }
}

FS *FS::create_by_fd(int fd)
{
  struct statfs st;
  ::fstatfs(fd, &st);
  return create(st.f_type);
}

// ---------------

int FS::set_alloc_hint(int fd, uint64_t hint)
{
  return 0;  // no-op
}

#ifdef HAVE_NAME_TO_HANDLE_AT
int FS::get_handle(int fd, std::string *h)
{
  char buf[sizeof(struct file_handle) + MAX_HANDLE_SZ];
  struct file_handle *fh = (struct file_handle *)buf;
  int mount_id;

  fh->handle_bytes = MAX_HANDLE_SZ;
  int r = name_to_handle_at(fd, "", fh, &mount_id, AT_EMPTY_PATH);
  if (r < 0) {
    return -errno;
  }
  *h = std::string(buf, fh->handle_bytes + sizeof(struct file_handle));
  return 0;
}

int FS::open_handle(int mount_fd, const std::string& h, int flags)
{
  if (h.length() < sizeof(struct file_handle)) {
    return -EINVAL;
  }
  struct file_handle *fh = (struct file_handle *)h.data();
  if (fh->handle_bytes > h.length()) {
    return -ERANGE;
  }
  int fd = open_by_handle_at(mount_fd, fh, flags);
  if (fd < 0)
    return -errno;
  return fd;
}

#else // HAVE_NAME_TO_HANDLE_AT

int FS::get_handle(int fd, std::string *h)
{
  return -EOPNOTSUPP;
}

int FS::open_handle(const std::string& h)
{
  return -EOPNOTSUPP;
}

#endif // HAVE_NAME_TO_HANDLE_AT

int FS::copy_file_range(int to_fd, uint64_t to_offset,
			int from_fd,
			uint64_t from_offset, uint64_t from_len)
{
  assert(0 == "write me");
}