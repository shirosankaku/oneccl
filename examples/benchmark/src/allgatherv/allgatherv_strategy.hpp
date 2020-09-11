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
#ifndef ALLGATHERV_STRATEGY_HPP
#define ALLGATHERV_STRATEGY_HPP

struct allgatherv_strategy_impl {
    size_t comm_size = 0;
    std::vector<size_t> recv_counts;
    allgatherv_strategy_impl(size_t size) : comm_size(size) {
        recv_counts.resize(size);
        //int result = posix_memalign((void**)&recv_counts, ALIGNMENT, comm_size * sizeof(size_t));
        //(void)result;
    }

    allgatherv_strategy_impl(const allgatherv_strategy_impl&) = delete;
    allgatherv_strategy_impl& operator=(const allgatherv_strategy_impl&) = delete;

    ~allgatherv_strategy_impl() {
        //free(recv_counts);
    }

    static constexpr const char* class_name() {
        return "allgatherv";
    }

    static const ccl::allgatherv_attr& get_op_attr(const bench_coll_exec_attr& bench_attr) {
        return bench_attr.get_attr<ccl::allgatherv_attr>();
    }

    template <class Dtype, class comm_t, class... Args>
    void start_internal(comm_t& comm,
                        size_t count,
                        const Dtype send_buf,
                        Dtype recv_buf,
                        const bench_coll_exec_attr& bench_attr,
                        req_list_t& reqs,
                        Args&&... args) {
        for (size_t idx = 0; idx < comm_size; idx++) {
            recv_counts[idx] = count;
        }
        reqs.push_back(
            comm.allgatherv(send_buf, count, recv_buf, recv_counts, std::forward<Args>(args)...));
    }
};

#endif /* ALLGATHER_STRATEGY_HPP */