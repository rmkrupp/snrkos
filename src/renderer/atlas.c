/* File: src/renderer/atlas.c
 * Part of snrkos <github.com/rmkrupp/snrkos>
 * Original version from <github.com/rmkrupp/cards-client>
 *
 * Copyright (C) 2025 Noah Santer <n.ed.santer@gmail.com>
 * Copyright (C) 2025 Rebecca Krupp <beka.krupp@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "renderer/renderer.h"
#include "renderer/atlas.h"

/* create an atlas capable of holding element_max textures, each a 2D image of
 * (element_size, element_size) dimensions
 *
 * it will pack this into one 2D texture (bounds limited by device and
 * configuration) with as many layers as needed
 *
 * in the case that there is not enough space in one texture, it fails
 *
 * note that the lowest supported dimensions are 256 layers and 4k x/y,
 * found (outside of mobile) only on ARM Mali GPUs. the vast majority of
 * devices support 2048 layers of 16k x/y
 *
 * even 256 * 4k * 4k = 4GB of memory and could pack 16k 512x512 textures,
 * or 1M 64x64 textures; 2048 * 16k * 16k is 512GB of memory
 *
 * the following config value are used to influence this proess:
 *      min(config.atlas.max_texture_width, limits.maxImageDimension2D)
 *      min(config.atlas.max_texture_layers, limits.maxImageArrayLayers)
 */
struct atlas * atlas_create(uint32_t element_size, uint32_t elements)
{

    fprintf(
            stderr,
            "[renderer] (INFO) creating atlas for %u %ux%u elements\n",
            elements,
            element_size,
            element_size
        );

    if (element_size == 0) {
        fprintf(
                stderr,
                "[renderer] atlas: element width must be >= 0\n"
            );
        return NULL;
    }

    struct atlas * atlas = calloc(1, sizeof(*atlas));

    if (elements == 0) {
        fprintf(
                stderr,
                "[render] (INFO) atlas: zero-element atlas created\n"
            );
        return atlas;
    }

    size_t max_texture_width = renderer.limits.maxImageDimension2D;
    size_t max_texture_layers = renderer.limits.maxImageArrayLayers;
    if (renderer.config.atlas.max_texture_width < max_texture_width) {
        max_texture_width = renderer.config.atlas.max_texture_width;
        fprintf(
                stderr,
                "[renderer] (INFO) atlas: width is limited by config to %zu\n",
                max_texture_width
            );
    } else {
        fprintf(
                stderr,
                "[renderer] (INFO) atlas: width is limited by device to %zu\n",
                max_texture_width
            );
    }
    if (renderer.config.atlas.max_texture_layers < max_texture_layers) {
        max_texture_layers = renderer.config.atlas.max_texture_layers;
        fprintf(
                stderr,
                "[renderer] (INFO) atlas: layer count is limited by config to %zu\n",
                max_texture_layers
            );
    } else {
        fprintf(
                stderr,
                "[renderer] (INFO) atlas: layer count is limited by device to %zu\n",
                max_texture_layers
            );
    }

    if (element_size > max_texture_width) {
        fprintf(
                stderr,
                "[renderer] atlas: cannot be created (element size %u larger than largest texture size %zu)\n",
                element_size,
                max_texture_width
            );
        free(atlas);
        return NULL;
    }

    /* how many element_size blocks can we fit in a layer of this size? */
    size_t elements_wide_max = max_texture_width / element_size;
    size_t elements_per_layer = elements_wide_max * elements_wide_max;
    size_t needed_layers;
    if (elements % elements_per_layer == 0) {
        needed_layers = elements / elements_per_layer;
    } else {
        needed_layers = elements / elements_per_layer + 1;
    }

    if (needed_layers > max_texture_layers) {
        fprintf(
                stderr,
                "[renderer] atlas: cannot be created, need %zu elements but only have %zu\n",
                needed_layers * elements_per_layer,
                max_texture_layers * elements_per_layer
            );
        free(atlas);
        return NULL;
    }

    assert(needed_layers > 0);

    size_t elements_wide,
           elements_tall;
    if (needed_layers == 1) {
        size_t sqrt_max = sqrt(elements);
        if (sqrt_max * sqrt_max == elements) {
            elements_wide = sqrt_max;
            elements_tall = sqrt_max;
        } else {
            elements_wide = sqrt_max;
            elements_tall = elements / sqrt_max + elements % sqrt_max;
        }
    } else {
        elements_wide = elements_wide_max;
        elements_tall = elements_wide_max;
    }

    fprintf(
            stderr,
            "[renderer] (INFO) atlas: using %zu total layers of %zu x %zu texels\n",
            needed_layers,
            elements_wide * element_size,
            elements_tall * element_size
        );

    if (create_image(
                &atlas->image,
                &atlas->image_memory,
                elements_wide * element_size,
                elements_tall * element_size,
                needed_layers,
                VK_FORMAT_R8_SNORM,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            )) {
        free(atlas);
        return NULL;
    }

    if (transition_image_layout(
                renderer.texture_image,
                VK_FORMAT_R8_SNORM,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                needed_layers
            )) {
        vkDestroyImage(renderer.device, atlas->image, NULL);
        vkFreeMemory(renderer.device, atlas->image_memory, NULL);
        free(atlas);
        return NULL;
    }

    if (create_buffer(
                &atlas->staging_buffer,
                &atlas->staging_buffer_memory,
                elements_wide * elements_tall *
                element_size * element_size *
                needed_layers,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            )) {
        return RENDERER_ERROR;
    }

    vkMapMemory(
            renderer.device,
            atlas->staging_buffer_memory,
            0,
            size,
            0,
            &atlas->staging_buffer_data
        );

    atlas->element_size = element_size;
    atlas->elements_wide = elements_wide;
    atlas->elements_tall = elements_tall;
    atlas->layers = needed_layers;

    atlas->cursor = (struct atlas_cursor) {
        .x = 0,
        .y = 0,
        .z = 0
    };

    return atlas;
}

void atlas_destroy(struct atlas * atlas)
{
    if (atlas->staging_buffer_data) {
        vkUnmapMemory(renderer.device, atlas->staging_buffer_memory);
    }
    if (atlas->staging_buffer) {
        vkDestroyBuffer(renderer.device, atlas->staging_buffer, NULL);
    }
    if (atlas->staging_buffer_memory) {
        vkFreeMemory(renderer.device, atlas->staging_buffer_memory, NULL);
    }
    vkDestroyImage(renderer.device, atlas->image, NULL);
    vkFreeMemory(renderer.device, atlas->image_memory, NULL);
    free(atlas);
}

enum renderer_result atlas_upload(
        struct atlas * atlas,
        void * data,
        float * x_out,
        float * y_out,
        float * z_out,
        float * width_out,
        float * height_out
    )
{
    if (atlas->done) {
        return RENDERER_ERROR;
    }

    VkDeviceSize row_size = atlas->element_size * atlas->elements_wide;
    VkDeviceSize layer_size = atlas->element_size * atlas->element_size *
                              atlas->elements_wide * atlas->elements_tall;
    VkDeviceSize offset =
        atlas->cursor.z * layer_size +
        atlas->cursor.y * row_size +
        atlas->cursor.x * element_size;

    memcpy(
            atlas->staging_buffer_data,
            data,
            atlas->element_size * atlas->element_size
        );

    VkCommandBuffer command_buffer;
    if (command_buffer_oneoff_begin(&command_buffer)) {
        return RENDERER_ERROR;
    }

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = atlas->cursor.z,
            .layerCount = 1
        },
        .imageOffset = { 0, 0, 0 },
        .imageExtent = {
            atlas->element_size,
            atlas->element_size,
            1
        }
    };

    vkCmdCopyBufferToImage(
            command_buffer,
            atlas->staging_buffer,
            atlas->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            (VkBufferImageCopy[]) {
                region
            }
        );

    if (command_buffer_oneoff_end(&command_buffer)) {
        return RENDERER_ERROR;
    }

    *x_out = (float)atlas->cursor.x / (float)atlas->elements_wide;
    *y_out = (float)atlas->cursor.y / (float)atlas->elements_tall;
    *z_out = (float)atlas->cursor.z;
    *width_out = 1 / (float)atlas->elements_wide;
    *height_out = 1 / (float)atlas->elements_tall;

    atlas->cursor.x++;
    if (atlas->cursor.x == atlas->elements_wide) {
        atlas->cursor.x = 0;
        atlas->cursor.y++;
        if (atlas->cursor.y == atlas->elements_tall) {
            atlas->cursor.y = 0;
            atlas->cursor.z++;
            if (atlas->cursor.z == atlas->layers) {
                atlas->cursor.z = 0;
                if (transition_image_layout(
                            atlas->image,
                            VK_FORMAT_R8_SNORM,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            1
                        )) {
                    return RENDERER_ERROR;
                }
                atlas_end_upload(atlas);
                atlas->done = true;
            }
        }
    }

    return RENDERER_OKAY;
}

static void atlas_end_upload(struct atlas * atlas)
{
    /* TODO: destroy staging buffer */
    vkUnmapMemory(renderer.device, atlas->staging_buffer_memory);
    atlas->staging_buffer_data = NULL;
}
