/*
 * Copyright 2021 Mattia Penati <mattia.penati@protonmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

@ctype mat4 hmm_mat4

@vs vs
layout(binding=0) uniform vs_params {
    mat4 proj;
};

layout(location=0) in vec3 position;
layout(location=1) in vec2 texCoord;

out vec2 uv;

void main() {
    gl_Position = proj * vec4(position, 1.0f);
    uv = texCoord;
}
@end

@fs fs
layout(location=0) uniform sampler2D tex;

in vec2 uv;

out vec4 frag_color;

void main() {
    frag_color = texture(tex, uv);
}
@end

@program rgba vs fs

