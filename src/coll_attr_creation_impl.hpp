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
#include "oneapi/ccl/ccl_types.hpp"
#include "oneapi/ccl/ccl_coll_attr.hpp"
#include "coll/coll_attributes.hpp"

namespace ccl {
/* TODO temporary function for UT compilation: would be part of ccl::environment in final*/
template <class coll_attribute_type, class... attr_value_pair_t>
coll_attribute_type create_coll_attr(attr_value_pair_t&&... avps) {
    ccl::library_version ret{};
    ret.major = CCL_MAJOR_VERSION;
    ret.minor = CCL_MINOR_VERSION;
    ret.update = CCL_UPDATE_VERSION;
    ret.product_status = CCL_PRODUCT_STATUS;
    ret.build_date = CCL_PRODUCT_BUILD_DATE;
    ret.full = CCL_PRODUCT_FULL;

    auto coll_attr = coll_attribute_type(ret);

    int expander[]{ (coll_attr.template set<attr_value_pair_t::idx()>(avps.val()), 0)... };
    (void)expander;
    return coll_attr;
}
} // namespace ccl
