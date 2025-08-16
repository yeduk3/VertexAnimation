//
//  va.frag
//  VertexAnimation
//
//  Created by 이용규 on 8/17/25.
//

#version 410 core

in vec3 Normal;
out vec4 FragColor;

void main() {
    FragColor = vec4(Normal, 1);
}
