#pragma once

#include "shader.h"
#include "texture.h"

struct material_t {
	shader_t *shader;
	tex2d_t *texture;
};