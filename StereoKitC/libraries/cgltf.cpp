#include "../sk_memory.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning( disable : 26451 )
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#endif
#define CGLTF_IMPLEMENTATION
#define CGLTF_MALLOC sk::sk_malloc
#define CGLTF_FREE sk::_sk_free
#include "../libraries/cgltf.h"
#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma clang diagnostic pop
#endif

#include "stref.h"
#include "../stereokit.h"

namespace sk {

void gltf_parse_extras(model_t model, model_node_id node, const char *extras_json, size_t extras_size) {
	jsmn_parser parser;
	jsmn_init(&parser);

	int32_t token_ct = jsmn_parse(&parser, extras_json, extras_size, nullptr, 0);
	if (token_ct <= 0) return;

	jsmntok_t* tokens = sk_malloc_t(jsmntok_t, token_ct);
	jsmn_init(&parser);
	token_ct = jsmn_parse(&parser, extras_json, extras_size, tokens, token_ct);

	if (token_ct < 1 || tokens[0].type != JSMN_OBJECT) {
		return;
	}

	for (int32_t i = 1; i < token_ct; i+=1) {
		if (tokens[i].type == JSMN_STRING) {
			if (tokens[i].size == 1) {
				stref_t key     = { extras_json + tokens[i  ].start, (uint32_t)(tokens[i  ].end-tokens[i  ].start) };
				stref_t val     = { extras_json + tokens[i+1].start, (uint32_t)(tokens[i+1].end-tokens[i+1].start) };
				char*   key_str = stref_copy(key);
				char*   val_str = stref_copy(val);
				model_node_info_set(model, node, key_str, val_str);
				sk_free(key_str);
				sk_free(val_str);
			} else {
				stref_t key     = { extras_json + tokens[i].start, (uint32_t)(tokens[i].end - tokens[i].start) };
				char*   key_str = stref_copy(key);
				model_node_info_set(model, node, key_str, "true");
				sk_free(key_str);
			}
		} else if (tokens[i].type == JSMN_OBJECT) {
			log_warn("SK node extras does not support object types");
		} else if (tokens[i].type == JSMN_ARRAY) {
			log_warn("SK node extras does not support array types");
		}
		i += tokens[i].size;
	}
	sk_free(tokens);
}

}