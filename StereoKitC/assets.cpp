#include "assets.h"

#include "mesh.h"

#include <vector>
using namespace std;

vector<asset_header_t *> assets;

void *assets_allocate(asset_type_ type) {
	size_t size = sizeof(asset_header_t);
	switch(type) {
	case asset_type_mesh: size = sizeof(_mesh_t); break;
	default: throw;
	}

	asset_header_t *header = (asset_header_t *)malloc(size);
	memset(header, 0, size);
	header->type = type;
	header->refs += 1;
	assets.push_back(header);
	return header;
}