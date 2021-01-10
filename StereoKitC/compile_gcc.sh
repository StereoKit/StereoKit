#!/bin/bash

gcc \
 color.cpp \
 hierarchy.cpp \
 intersect.cpp \
 log.cpp \
 math.cpp \
 spherical_harmonics.cpp \
 stereokit.cpp \
 stereokit_ui.cpp \
 asset_types/assets.cpp \
 asset_types/font.cpp \
 asset_types/material.cpp \
 asset_types/mesh.cpp \
 asset_types/model.cpp \
 asset_types/model_fbx.cpp \
 asset_types/model_gltf.cpp \
 asset_types/model_obj.cpp \
 asset_types/model_stl.cpp \
 asset_types/shader.cpp \
 asset_types/sound.cpp \
 asset_types/sprite.cpp \
 asset_types/texture.cpp \
 -Ilib/include_no_win \
 -std=c++11 \
 -D _DEBUG \
 -D _DLL 