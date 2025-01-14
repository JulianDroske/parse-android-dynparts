/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBLP_UTILITY_H
#define LIBLP_UTILITY_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include <cassert>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>

#include <liblp/liblp.h>

#define LP_TAG "[liblp]"
#define LWARN NewlineLogger(std::cerr).stream << "[W]" << LP_TAG
#define LINFO NewlineLogger(std::cerr).stream << "[I]" << LP_TAG
#define LERROR NewlineLogger(std::cerr).stream << "[E]" << LP_TAG
#define PWARNING LWARN
#define PERROR LERROR

#define CHECK assert

#define CALL_RETRY(retvar, expression) do { \
        retvar = (expression); \
    } while (retvar == -1 && errno == EINTR);

namespace android {
namespace fs_mgr {

// Return the offset of the primary or backup geometry.
int64_t GetPrimaryGeometryOffset();
int64_t GetBackupGeometryOffset();

// Return the offset of a primary metadata slot, relative to the start of the
// device.
int64_t GetPrimaryMetadataOffset(const LpMetadataGeometry& geometry, uint32_t slot_number);

// Return the offset of a backup metadata slot, relative to the end of the
// device.
int64_t GetBackupMetadataOffset(const LpMetadataGeometry& geometry, uint32_t slot_number);

// Return the total space at the start of the super partition that must be set
// aside from headers/metadata and backups.
uint64_t GetTotalMetadataSize(uint32_t metadata_max_size, uint32_t max_slots);

// Cross-platform helper for lseek64().
int64_t SeekFile64(int fd, int64_t offset, int whence);

// Compute a SHA256 hash.
void SHA256(const void* data, size_t length, uint8_t out[32]);

// Align |base| such that it is evenly divisible by |alignment|, which does not
// have to be a power of two. Return false on overflow.
template <typename T>
bool AlignTo(T base, uint32_t alignment, T* out) {
    static_assert(std::numeric_limits<T>::is_integer);
    static_assert(!std::numeric_limits<T>::is_signed);
    if (!alignment) {
        *out = base;
        return true;
    }
    T remainder = base % alignment;
    if (remainder == 0) {
        *out = base;
        return true;
    }
    T to_add = alignment - remainder;
    if (to_add > std::numeric_limits<T>::max() - base) {
        return false;
    }
    *out = base + to_add;
    return true;
}

// Update names from C++ strings.
bool UpdateBlockDevicePartitionName(LpMetadataBlockDevice* device, const std::string& name);
bool UpdatePartitionGroupName(LpMetadataPartitionGroup* group, const std::string& name);
bool UpdatePartitionName(LpMetadataPartition* partition, const std::string& name);

// Call BLKROSET ioctl on fd so that fd is readonly / read-writable.
bool SetBlockReadonly(int fd, bool readonly);

// Forcefully set metadata header version to 1.0, clearing any incompatible flags and attributes
// so that when downgrading to a build with liblp V0, the device still boots.
void SetMetadataHeaderV0(LpMetadata* metadata);

bool ReadFully(int fd, void* data, size_t byte_count);

class NewlineLogger {
  public:
    NewlineLogger(std::ostream& sink) : sink_(sink) {}
    ~NewlineLogger() {
        sink_ << stream.str() << std::endl;
    }

    std::ostringstream stream;

  private:
    std::ostream& sink_;
};

}  // namespace fs_mgr
}  // namespace android

#endif  // LIBLP_UTILITY_H
