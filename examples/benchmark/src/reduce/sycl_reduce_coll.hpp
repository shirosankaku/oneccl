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

#include "reduce_strategy.hpp"

#ifdef CCL_ENABLE_SYCL
#include "sycl_coll.hpp"

template <class Dtype>
class reduce_buf_check {};

template <class Dtype>
class reduce_buf_fill {};

template <class Dtype>
struct sycl_reduce_coll : sycl_base_coll<Dtype, reduce_strategy_impl> {
    using coll_base = sycl_base_coll<Dtype, reduce_strategy_impl>;
    using coll_base::send_bufs;
    using coll_base::recv_bufs;
    using coll_base::single_send_buf;
    using coll_base::single_recv_buf;
    using coll_base::comm;

    sycl_reduce_coll(bench_init_attr init_attr)
            : coll_base(init_attr) {}

    virtual void prepare(size_t elem_count) override {
        size_t local_rank = coll_base::comm().rank();
        for (size_t b_idx = 0; b_idx < base_coll::get_buf_count(); b_idx++) {
            device_data::sycl_queue.submit([&](handler& cgh) {
                auto send_buf = (static_cast<sycl_buffer_t<Dtype>*>(send_bufs[b_idx]));
                auto recv_buf = (static_cast<sycl_buffer_t<Dtype>*>(recv_bufs[b_idx]));
                auto send_buf_acc = send_buf->template get_access<mode::write>(cgh);
                auto recv_buf_acc = recv_buf->template get_access<mode::write>(cgh);
                cgh.parallel_for<class reduce_buf_fill<Dtype>>(range<1>{elem_count}, [=](item<1> e_idx)
                {
                    send_buf_acc[e_idx] = local_rank;
                    recv_buf_acc[e_idx] = 0;
                });
            });
        }
    }

    virtual void finalize(size_t elem_count) override {
        bool unexpected_device_value = false;
        Dtype sbuf_expected = coll_base::comm().rank();
        Dtype rbuf_expected =
            (coll_base::comm().size() - 1) * ((float)coll_base::comm().size() / 2);
        size_t local_rank = coll_base::comm().rank();

        for (size_t b_idx = 0; b_idx < base_coll::get_buf_count(); b_idx++) {
            device_data::sycl_queue.submit([&](handler& cgh) {
                auto send_buf = (static_cast<sycl_buffer_t<Dtype>*>(send_bufs[b_idx]));
                auto recv_buf = (static_cast<sycl_buffer_t<Dtype>*>(recv_bufs[b_idx]));
                auto send_buf_acc = send_buf->template get_access<mode::write>(cgh);
                auto recv_buf_acc = recv_buf->template get_access<mode::write>(cgh);
                cgh.parallel_for<class reduce_buf_check<Dtype>>(range<1>{elem_count}, [=](item<1> e_idx) mutable
                {
                    Dtype value = send_buf_acc[e_idx];
                    if (value != sbuf_expected)
                        unexpected_device_value = true;

                    if (local_rank == COLL_ROOT) {
                        value = recv_buf_acc[e_idx];
                        if (value != rbuf_expected)
                            unexpected_device_value = true;
                    }
                });
            });
        }

        for (size_t b_idx = 0; b_idx < base_coll::get_buf_count(); b_idx++) {
            auto send_buf = (static_cast<sycl_buffer_t<Dtype>*>(send_bufs[b_idx]));
            auto recv_buf = (static_cast<sycl_buffer_t<Dtype>*>(recv_bufs[b_idx]));
            auto send_buf_acc = send_buf->template get_access<mode::read>();
            auto recv_buf_acc = recv_buf->template get_access<mode::read>();

            for (size_t e_idx = 0; e_idx < elem_count; e_idx++) {
                Dtype value = send_buf_acc[e_idx];
                if (value != sbuf_expected) {
                    std::cout << this->name() << " send_bufs: buf_idx " << b_idx << ", elem_idx "
                              << e_idx << ", expected " << sbuf_expected << ", got " << value
                              << std::endl;
                    ASSERT(0, "unexpected value");
                }

                if (local_rank != COLL_ROOT)
                    continue;

                value = recv_buf_acc[e_idx];
                if (value != rbuf_expected) {
                    std::cout << this->name() << " recv_bufs: buf_idx " << b_idx << ", elem_idx "
                              << e_idx << ", expected " << rbuf_expected << ", got " << value
                              << std::endl;
                    ASSERT(0, "unexpected value");
                }
            }
        }

        if (unexpected_device_value)
            ASSERT(0, "unexpected value on device");
    }
};
#endif /* CCL_ENABLE_SYCL */
