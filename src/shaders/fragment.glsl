#version 450
/* File: src/shaders/fragment.glsl
 * Part of snrkos <github.com/rmkrupp/snrkos>
 * Original version from <github.com/rmkrupp/cards-client>

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

layout(constant_id = 0) const uint N_LIGHTS = 1;

struct light {
    vec4 position;
    vec4 color;
    float intensity;
    uint flags;
};

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldPosition;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in flat ivec3 texture_indices;
layout(location = 5) in flat uint fragFlags;

layout(binding = 1) uniform sampler2DArray texSampler;

layout(location = 0) out vec4 out_color;

layout(binding = 2, std140) uniform UniformBufferObjectG {
    float ambient_light;
    light lights[N_LIGHTS];
} ubo_g;

void main() {

    float t_solid = texture(texSampler, vec3(fragTexCoord, texture_indices.x)).x;
    float t_outline = texture(texSampler, vec3(fragTexCoord, texture_indices.y)).x;
    float t_glow = texture(texSampler, vec3(fragTexCoord, texture_indices.z)).x;

    bool glows = ((fragFlags & 2) == 2) && (texture_indices.z > 0);

    /*
    float dcenter = distance(fragTexCoord, vec2(0.5, 0.5));
    float dcorner00 = distance(fragTexCoord, vec2(0.0, 0.0));
    float dcorner01 = distance(fragTexCoord, vec2(0.0, 1.0));
    float dcorner10 = distance(fragTexCoord, vec2(1.0, 0.0));
    float dcorner11 = distance(fragTexCoord, vec2(1.0, 1.0));

    float threshold = 0.025;

    float mind = min(dcenter, min(dcorner00, min(dcorner01, min(dcorner10, dcorner11))));
    */

    float outline_cutoff = 0.0625;

    vec3 color = vec3(1.0, 1.0, 1.0) * ubo_g.ambient_light;

    for (uint i = 0; i < N_LIGHTS; i++) {
        if ((ubo_g.lights[i].flags & 1) == 1) {
            float d = distance(fragWorldPosition, ubo_g.lights[i].position.xyz);
            color += ubo_g.lights[i].color.xyz / (d * d) * ubo_g.lights[i].intensity;
        }
    }

    if (fragWorldPosition.y < -0.5) {
        discard;
    } else 
    if (t_solid > 0.5 * 16 / 128) {
        discard;
    } else if (t_outline <= outline_cutoff) {
        out_color = vec4(0.0, 0.0, 0.0, 1.0);
    } else if (glows && t_glow <= outline_cutoff) {
        out_color = vec4(1.0, 0.647, 0.0, 1.0);
    } else {
        out_color = vec4(color, 1.0);
    }
}
