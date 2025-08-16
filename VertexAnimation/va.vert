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

out vec3 Normal;

void main() {
    gl_Position = MVP * vec4(mix(Pos1, Pos2, interTime), 1);
    Normal = NormalMatrix * mix(Nor1, Nor2, interTime);
}
