/* File: include/renderer/renderer.h
 * Part of snrkos <github.com/rmkrupp/snrkos>
 * Original part of <github.com/rmkrupp/cards-client>
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
#ifndef RENDERER_ATLAS_H
#define RENDERER_ATLAS_H

struct atlas {
    VkImage image;
    VkDeviceMemory image_memory;
    uint32_t element_size;
    uint32_t elements_tall;
    uint32_t elements_wide;
    uint32_t layers;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    void * staging_buffer_data;

    bool begin;
    bool done;

    struct atlas_cursor {
        uint32_t x,
                 y,
                 z;
    } cursor;
};

struct atlas * atlas_create(uint32_t element_size, uint32_t element_max);
void atlas_destroy(struct atlas * atlas);
enum renderer_result atlas_upload(
        struct atlas * atlas,
        void * data,
        float * x_out,
        float * y_out,
        float * z_out,
        float * width_out,
        float * height_out
    );

#endif /* RENDERER_ATLAS_H */
