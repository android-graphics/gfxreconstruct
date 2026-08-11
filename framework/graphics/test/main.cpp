///////////////////////////////////////////////////////////////////////////////
// Copyright(c) 2019 Advanced Micro Devices, Inc.All rights reserved
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
/// \author AMD Developer Tools Team
/// \description gfxrecon_graphics test main entry point
///////////////////////////////////////////////////////////////////////////////

#define CATCH_CONFIG_MAIN
#include <numeric>
#include <catch2/catch.hpp>

#include "graphics/vulkan_feature_util.h"
#include "graphics/vulkan_shader_group_handle.h"

TEST_CASE("vulkan_shader_group_handle - create empty handles", "[]")
{
    gfxrecon::graphics::shader_group_handle_t one, two;

    // check for all zeros
    uint8_t data[gfxrecon::graphics::shader_group_handle_t::MAX_HANDLE_SIZE] = {};
    REQUIRE(memcmp(one.data, data, gfxrecon::graphics::shader_group_handle_t::MAX_HANDLE_SIZE) == 0);

    REQUIRE(one == two);
    REQUIRE_FALSE(one != two);

    auto three = one;
    REQUIRE(one == three);
}

TEST_CASE("vulkan_shader_group_handle - create handles", "[]")
{
    std::vector<uint8_t> data(32);
    std::iota(data.begin(), data.end(), 0);
    gfxrecon::graphics::shader_group_handle_t one(data.data(), data.size());

    data[31] = 99;
    gfxrecon::graphics::shader_group_handle_t two(data.data(), data.size());
    REQUIRE(one != two);

    // check hashing via std::hash
    std::hash<gfxrecon::graphics::shader_group_handle_t> hasher;
    REQUIRE(hasher(one) != hasher(two));
}

TEST_CASE("FilterPNextFeatures - remove unsupported", "[feature_util]")
{
    VkPhysicalDeviceFragmentDensityMapFeaturesEXT fdm_features = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT,
        nullptr
    };

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR,
        nullptr
    };

    SECTION("Both extensions enabled - nothing removed")
    {
        fdm_features.pNext = &fsr_features;
        fsr_features.pNext = nullptr;
        VkDeviceCreateInfo create_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, &fdm_features };

        std::vector<const char*> enabled = {
            "VK_EXT_fragment_density_map",
            "VK_KHR_fragment_shading_rate"
        };

        gfxrecon::graphics::feature_util::FilterPNextFeatures(&create_info, enabled);

        REQUIRE(create_info.pNext == &fdm_features);
        REQUIRE(fdm_features.pNext == &fsr_features);
        REQUIRE(fsr_features.pNext == nullptr);
    }

    SECTION("One extension disabled - that struct removed")
    {
        fdm_features.pNext = &fsr_features;
        fsr_features.pNext = nullptr;
        VkDeviceCreateInfo create_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, &fdm_features };

        // VK_EXT_fragment_density_map is NOT enabled
        std::vector<const char*> enabled = {
            "VK_KHR_fragment_shading_rate"
        };

        gfxrecon::graphics::feature_util::FilterPNextFeatures(&create_info, enabled);

        // fdm_features should be removed, create_info.pNext should point to fsr_features
        REQUIRE(create_info.pNext == &fsr_features);
        REQUIRE(fsr_features.pNext == nullptr);
    }

    SECTION("Other extension disabled - that struct removed")
    {
        fdm_features.pNext = &fsr_features;
        fsr_features.pNext = nullptr;
        VkDeviceCreateInfo create_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, &fdm_features };

        // VK_KHR_fragment_shading_rate is NOT enabled
        std::vector<const char*> enabled = {
            "VK_EXT_fragment_density_map"
        };

        gfxrecon::graphics::feature_util::FilterPNextFeatures(&create_info, enabled);

        // fsr_features should be removed, fdm_features.pNext should be nullptr
        REQUIRE(create_info.pNext == &fdm_features);
        REQUIRE(fdm_features.pNext == nullptr);
    }

    SECTION("Both extensions disabled - both removed")
    {
        fdm_features.pNext = &fsr_features;
        fsr_features.pNext = nullptr;
        VkDeviceCreateInfo create_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, &fdm_features };

        std::vector<const char*> enabled = {};

        gfxrecon::graphics::feature_util::FilterPNextFeatures(&create_info, enabled);

        REQUIRE(create_info.pNext == nullptr);
    }
}
