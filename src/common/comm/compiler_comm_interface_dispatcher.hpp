/*
 Copyright 2016-2020 Intel Corporation
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
     http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/
#pragma once
#include <cstddef>
#include <memory>

#include "oneapi/ccl/ccl_types.hpp"
#include "supported_topologies.hpp"
#include "communicator_traits.hpp"

namespace native {
struct ccl_device;
}
namespace ccl {
#ifdef MULTI_GPU_SUPPORT
struct gpu_comm_attr;
#endif
struct communicator_interface;
class device_comm_split_attr;

using communicator_interface_ptr = std::shared_ptr<communicator_interface>;

struct communicator_interface_dispatcher {
    using device_t = typename ccl::unified_device_type::ccl_native_t;
    using context_t = typename ccl::unified_device_context_type::ccl_native_t;

#ifdef MULTI_GPU_SUPPORT
    virtual void visit(ccl::gpu_comm_attr& comm_attr) = 0;
#endif //MULTI_GPU_SUPPORT

    virtual ccl::device_index_type get_device_path() const = 0;
    virtual device_t get_device() = 0;
    virtual context_t get_context() = 0;
    virtual const device_comm_split_attr& get_comm_split_attr() const = 0;
    virtual device_group_split_type get_topology_type() const = 0;
    virtual device_topology_type get_topology_class() const = 0;

    // create communicator for device & cpu types (from device class)
    template <class DeviceType,
              typename std::enable_if<not std::is_same<typename std::remove_cv<DeviceType>::type,
                                                       device_index_type>::value,
                                      int>::type = 0>
    static communicator_interface_ptr create_communicator_impl(const DeviceType& device,
                                                               size_t thread_idx,
                                                               size_t process_idx,
                                                               const device_comm_split_attr& attr);

    // create communicator for device & cpu types (from device index)
    template <class DeviceType,
              typename std::enable_if<
                  std::is_same<typename std::remove_cv<DeviceType>::type, device_index_type>::value,
                  int>::type = 0>
    static communicator_interface_ptr create_communicator_impl(DeviceType device_id,
                                                               size_t thread_idx,
                                                               size_t process_idx,
                                                               const device_comm_split_attr& attr);

private:
    static communicator_interface_ptr create_communicator_from_unified_device(
        unified_device_type&& device_id,
        size_t thread_idx,
        size_t process_idx,
        const device_comm_split_attr& attr);
};
} // namespace ccl