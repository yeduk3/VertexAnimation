//
//  va.vert
//  VertexAnimation
//
//  Created by 이용규 on 8/17/25.
//

#version 410 core

layout(location = 0) in vec3 Pos1;
layout(location = 1) in vec3 Nor1;
layout(location = 2) in vec3 Pos2;
layout(location = 3) in vec3 Nor2;

uniform float interTime;
uniform mat4 MVP;
uniform mat3 NormalMatrix;
uniform mat4 viewMat;

out vec3 Normal;
out vec4 VPos;

void main() {
    vec3 Pos = mix(Pos1, Pos2, interTime);
    gl_Position = MVP * vec4(Pos, 1);
    VPos = viewMat * vec4(Pos, 1);
    Normal = NormalMatrix * mix(Nor1, Nor2, interTime);
}
