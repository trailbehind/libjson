/*
    This is the implementation of the C interface to libjson
    This file may be included in any C++ application, but it will
    be completely ignored if JSON_LIBRARY isn't defined.  The
    only reason JSON_LIBRARY should be defined is when compiling libjson
    as a library
*/
#include "../../libjson.h"
#ifdef JSON_LIBRARY

    #include "JSONWGNode.h"
    #include "JSONWGWorker.h"
    #include "JSONValidator.h"
    #include "JSONStream.h"
	#include "JSONGlobals.h"
    #include <stdexcept>  //some methods throw exceptions
    #ifdef JSON_MEMORY_MANAGE
	   #define MANAGER_INSERT(x) json_global(NODE_HANDLER).insert(x)
	   #define MANAGER_STREAM_INSERT(x) json_global(STREAM_HANDLER).insert(x)
    #else
	   #define MANAGER_INSERT(x) x
	   #define MANAGER_STREAM_INSERT(x) x
    #endif

    static const json_char * EMPTY_CSTRING(JSON_TEXT(""));

	#ifdef JSON_MEMORY_POOL
		#include "JSONMemoryPool.h"
		extern memory_pool<NODEPOOL> json_node_mempool;
	#endif

    inline json_char * toCString(const json_string & str) json_nothrow {
	   const size_t len = (str.length() + 1) * sizeof(json_char);
	   #ifdef JSON_MEMORY_MANAGE
		  return (json_char *)json_global(STRING_HANDLER).insert(std::memcpy(json_malloc<json_char>(len), str.c_str(), len));
	   #else
		  return (json_char *)std::memcpy(json_malloc<json_char>(len), str.c_str(), len);
	   #endif
    }
	
	inline json_char * alreadyCString(json_char * str) json_nothrow {
		#ifdef JSON_MEMORY_MANAGE
		   return (json_char *)json_global(STRING_HANDLER).insert(str);
	    #else
			return str;
		#endif
	}

    /*
	   stuff that's in namespace libjson
    */
    void json_free(void * str){
	   JSON_ASSERT_SAFE(str, JSON_TEXT("freeing null ptr"), return;);
	   #ifdef JSON_MEMORY_MANAGE
		  json_global(STRING_HANDLER).remove(str);
	   #endif
	   libjson_free<void>(str);
    }

    void json_delete(JSONWGNode * node){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("deleting null ptr"), return;);
	   #ifdef JSON_MEMORY_MANAGE
		  json_global(NODE_HANDLER).remove(node);
	   #endif
	   JSONWGNode::deleteJSONWGNode((JSONWGNode *)node);
    }

    #ifdef JSON_MEMORY_MANAGE
	   void json_free_all(void){
		  json_global(STRING_HANDLER).clear();
	   }

	   void json_delete_all(void){
		  json_global(NODE_HANDLER).clear();
	   }
    #endif

    #ifdef JSON_READ_PRIORITY
	   JSONWGNode * json_parse(json_const json_char * json){
		  JSON_ASSERT_SAFE(json, JSON_TEXT("null ptr to json_parse"), return 0;);
		  json_try {
			 //use this constructor to simply copy reference instead of copying the temp
			 return MANAGER_INSERT(JSONWGNode::newJSONWGNode_Shallow(JSONWGWorker::parse(TOCONST_CSTR(json))));
		  } json_catch (std::invalid_argument, (void)0; )
		  #ifndef JSON_NO_EXCEPTIONS
			 return 0;
		  #endif
	   }

	   JSONWGNode * json_parse_unformatted(json_const json_char * json){
		  JSON_ASSERT_SAFE(json, JSON_TEXT("null ptr to json_parse"), return 0;);
		  json_try {
			 //use this constructor to simply copy reference instead of copying the temp
			 return MANAGER_INSERT(JSONWGNode::newJSONWGNode_Shallow(JSONWGWorker::parse_unformatted(TOCONST_CSTR(json))));
		  } json_catch(std::invalid_argument, (void)0; )
		  #ifndef JSON_NO_EXCEPTIONS
			 return 0;
		  #endif
	   }
    #endif

    json_char * json_strip_white_space(json_const json_char * json){
	   JSON_ASSERT_SAFE(json, JSON_TEXT("null ptr to json_strip_white_space"), return 0;);
	   return alreadyCString(JSONWGWorker::RemoveWhiteSpaceAndCommentsC(TOCONST_CSTR(json), false));
    }

    #ifdef JSON_VALIDATE
	   #ifdef JSON_DEPRECATED_FUNCTIONS
		  JSONWGNode * json_validate(json_const json_char * json){
			 JSON_ASSERT_SAFE(json, JSON_TEXT("null ptr to json_validate"), return 0;);
			 if (json_is_valid(json)){
				return json_parse(json);
			 }
			 return 0;
		  }
	   #endif
	   json_bool_t json_is_valid(json_const json_char * json){
		  JSON_ASSERT_SAFE(json, JSON_TEXT("null ptr to json_is_valid"), return (json_bool_t)false;);
		  #ifdef JSON_SECURITY_MAX_STRING_LENGTH
			 if (json_unlikely(json_strlen(json) > JSON_SECURITY_MAX_STRING_LENGTH)){
				JSON_FAIL(JSON_TEXT("Exceeding JSON_SECURITY_MAX_STRING_LENGTH"));
				return false;
			 }
		  #endif
		  json_auto<json_char> s;
		  s.set(JSONWGWorker::RemoveWhiteSpaceAndCommentsC(json, false));
		  return (json_bool_t)JSONValidator::isValidRoot(s.ptr);
	   }

	   json_bool_t json_is_valid_unformatted(json_const json_char * json){
		  JSON_ASSERT_SAFE(json, JSON_TEXT("null ptr to json_is_valid_unformatted"), return (json_bool_t)true;);
		  #ifdef JSON_SECURITY_MAX_STRING_LENGTH
			 if (json_unlikely(json_strlen(json) > JSON_SECURITY_MAX_STRING_LENGTH)){
				JSON_FAIL(JSON_TEXT("Exceeding JSON_SECURITY_MAX_STRING_LENGTH"));
				return false;
			 }
		  #endif
		  return (json_bool_t)JSONValidator::isValidRoot(json);
	   }
    #endif

    #if defined JSON_DEBUG && !defined JSON_STDERROR
	   //When libjson errors, a callback allows the user to know what went wrong
	   void json_register_debug_callback(json_error_callback_t callback){
		  JSONDebug::register_callback(callback);
	   }
    #endif

    #ifdef JSON_MUTEX_CALLBACKS
	   #ifdef JSON_MUTEX_MANAGE
		  void json_register_mutex_callbacks(json_mutex_callback_t lock, json_mutex_callback_t unlock, json_mutex_callback_t destroy, void * manager_lock){
			 JSONWGNode::register_mutex_callbacks(lock, unlock, manager_lock);
			 JSONWGNode::register_mutex_destructor(destroy);
		  }

	   #else
		  void json_register_mutex_callbacks(json_mutex_callback_t lock, json_mutex_callback_t unlock, void * manager_lock){
			 JSONWGNode::register_mutex_callbacks(lock, unlock, manager_lock);
		  }
	   #endif

	   void json_set_global_mutex(void * mutex){
		  JSONWGNode::set_global_mutex(mutex);
	   }

	   void json_set_mutex(JSONWGNode * node, void * mutex){
		  JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_mutex"), return;);
		  ((JSONWGNode*)node) -> set_mutex(mutex);
	   }

	   void json_lock(JSONWGNode * node, int threadid){
		  JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_lock"), return;);
		  ((JSONWGNode*)node) -> lock(threadid);
	   }

	   void json_unlock(JSONWGNode * node, int threadid){
		  JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_unlock"), return;);
		  ((JSONWGNode*)node) -> unlock(threadid);
	   }
    #endif

    #ifdef JSON_MEMORY_CALLBACKS
	   void json_register_memory_callbacks(json_malloc_t mal, json_realloc_t real, json_free_t fre){
		  JSONMemory::registerMemoryCallbacks(mal, real, fre);
	   }
    #endif

    #ifdef JSON_STREAM
	   void json_stream_push(JSONSTREAM * stream, json_const json_char * addendum){
		  (*((JSONStream*)stream)) << addendum;
	   }

	   void json_delete_stream(JSONSTREAM * stream){
		  JSON_ASSERT_SAFE(stream, JSON_TEXT("deleting null ptr"), return;);
		  #ifdef JSON_MEMORY_MANAGE
			 json_global(STREAM_HANDLER).remove(stream);
		  #endif
		  JSONStream::deleteJSONStream((JSONStream *)stream);
	   }

	   JSONSTREAM * json_new_stream(json_stream_callback_t callback, json_stream_e_callback_t e_callback, void * identifier){
		  return MANAGER_STREAM_INSERT(JSONStream::newJSONStream(callback, e_callback, identifier));
	   }

		void json_stream_reset(JSONSTREAM * stream){
			JSON_ASSERT_SAFE(stream, JSON_TEXT("resetting null ptr"), return;);
			((JSONStream*)stream) -> reset();
		}
    #endif


    /*
	stuff that's in class JSONWGNode
	*/
    //ctors
    JSONWGNode * json_new_a(json_const json_char * name, json_const json_char * value){
	   if (!name) name = EMPTY_CSTRING;
	   JSON_ASSERT_SAFE(value, JSON_TEXT("null value to json_new_a"), value = EMPTY_CSTRING;);
		#ifdef JSON_MEMORY_POOL
			return MANAGER_INSERT(new((JSONWGNode*)json_node_mempool.allocate()) JSONWGNode(TOCONST_CSTR(name), json_string(TOCONST_CSTR(value))));
		#elif defined(JSON_MEMORY_CALLBACKS)
		  return MANAGER_INSERT(new(json_malloc<JSONWGNode>(1)) JSONWGNode(TOCONST_CSTR(name), json_string(TOCONST_CSTR(value))));
	   #else
		  return MANAGER_INSERT(new JSONWGNode(TOCONST_CSTR(name), json_string(TOCONST_CSTR(value))));
	   #endif
    }

    JSONWGNode * json_new_i(json_const json_char * name, json_int_t value){
	   if (!name) name = EMPTY_CSTRING;
		#ifdef JSON_MEMORY_POOL
			return MANAGER_INSERT(new((JSONWGNode*)json_node_mempool.allocate()) JSONWGNode(TOCONST_CSTR(name), value));
		#elif defined(JSON_MEMORY_CALLBACKS)
		  return MANAGER_INSERT(new(json_malloc<JSONWGNode>(1)) JSONWGNode(TOCONST_CSTR(name), value));
	   #else
		  return MANAGER_INSERT(new JSONWGNode(TOCONST_CSTR(name), value));
	   #endif
    }

    JSONWGNode * json_new_f(json_const json_char * name, json_number value){
	   if (!name) name = EMPTY_CSTRING;
		#ifdef JSON_MEMORY_POOL
			return MANAGER_INSERT(new((JSONWGNode*)json_node_mempool.allocate()) JSONWGNode(TOCONST_CSTR(name), value));
		#elif defined(JSON_MEMORY_CALLBACKS)
		  return MANAGER_INSERT(new(json_malloc<JSONWGNode>(1)) JSONWGNode(TOCONST_CSTR(name), value));
	   #else
		  return MANAGER_INSERT(new JSONWGNode(TOCONST_CSTR(name), value));
	   #endif
    }

    JSONWGNode * json_new_b(json_const json_char * name, json_bool_t value){
	   if (!name) name = EMPTY_CSTRING;
		#ifdef JSON_MEMORY_POOL
			return MANAGER_INSERT(new((JSONWGNode*)json_node_mempool.allocate()) JSONWGNode(TOCONST_CSTR(name), static_cast<bool>(value)));
		#elif defined(JSON_MEMORY_CALLBACKS)
		  return MANAGER_INSERT(new(json_malloc<JSONWGNode>(1)) JSONWGNode(TOCONST_CSTR(name), static_cast<bool>(value)));
	   #else
		  return MANAGER_INSERT(new JSONWGNode(TOCONST_CSTR(name), static_cast<bool>(value)));
	   #endif
    }

    JSONWGNode * json_new(char type){
		#ifdef JSON_MEMORY_POOL
			return MANAGER_INSERT(new((JSONWGNode*)json_node_mempool.allocate()) JSONWGNode(type));
	    #elif defined(JSON_MEMORY_CALLBACKS)
		  return MANAGER_INSERT(new(json_malloc<JSONWGNode>(1)) JSONWGNode(type));
	   #else
		  return MANAGER_INSERT(new JSONWGNode(type));
	   #endif
    }

	JSONWGNode * json_copy(json_const JSONWGNode * orig){
		JSON_ASSERT_SAFE(orig, JSON_TEXT("null orig to json_copy"), return 0;);
		#ifdef JSON_MEMORY_POOL
			return MANAGER_INSERT(new((JSONWGNode*)json_node_mempool.allocate()) JSONWGNode(*((JSONWGNode*)orig)));
		#elif defined(JSON_MEMORY_CALLBACKS)
			return MANAGER_INSERT(new(json_malloc<JSONWGNode>(1)) JSONWGNode(*((JSONWGNode*)orig)));
		#else
			return MANAGER_INSERT(new JSONWGNode(*((JSONWGNode*)orig)));
		#endif
	}

    JSONWGNode * json_duplicate(json_const JSONWGNode * orig){
	   JSON_ASSERT_SAFE(orig, JSON_TEXT("null orig to json_duplicate"), return 0;);
	   return MANAGER_INSERT(JSONWGNode::newJSONWGNode_Shallow(((JSONWGNode*)orig) -> duplicate()));
    }

    //assignment
    void json_set_a(JSONWGNode * node, json_const json_char * value){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_a"), return;);
	   JSON_ASSERT_SAFE(value, JSON_TEXT("null value to json_set_a"), value = EMPTY_CSTRING;);
	   *((JSONWGNode*)node) = json_string(TOCONST_CSTR(value));
    }

    void json_set_i(JSONWGNode * node, json_int_t value){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_i"), return;);
	   *((JSONWGNode*)node) = value;
    }

    void json_set_f(JSONWGNode * node, json_number value){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_f"), return;);
	   *((JSONWGNode*)node) = value;
    }

    void json_set_b(JSONWGNode * node, json_bool_t value){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_b"), return;);
	   *((JSONWGNode*)node) = static_cast<bool>(value);
    }

    void json_set_n(JSONWGNode * node, json_const JSONWGNode * orig){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_n"), return;);
	   JSON_ASSERT_SAFE(orig, JSON_TEXT("null node to json_set_n"), return;);
	   *((JSONWGNode*)node) = *((JSONWGNode*)orig);
    }


    //inspectors
    char json_type(json_const JSONWGNode * node){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_type"), return JSON_NULL;);
	   return ((JSONWGNode*)node) -> type();
    }

    json_index_t json_size(json_const JSONWGNode * node){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_size"), return 0;);
	   return ((JSONWGNode*)node) -> size();
    }

    json_bool_t json_empty(json_const JSONWGNode * node){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_empty"), return true;);
	   return (json_bool_t)(((JSONWGNode*)node) -> empty());
    }

    json_char * json_name(json_const JSONWGNode * node){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_name"), return toCString(EMPTY_CSTRING););
	   return toCString(((JSONWGNode*)node) -> name());
    }

    #ifdef JSON_COMMENTS
	   json_char * json_get_comment(json_const JSONWGNode * node){
		  JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_get_comment"), return toCString(EMPTY_CSTRING););
		  return toCString(((JSONWGNode*)node) -> get_comment());
	   }
    #endif

    json_char * json_as_string(json_const JSONWGNode * node){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_string"), return toCString(EMPTY_CSTRING););
	   return toCString(((JSONWGNode*)node) -> as_string());
	   //return toCString(static_cast<json_string>(*((JSONWGNode*)node)));
    }

    json_int_t json_as_int(json_const JSONWGNode * node){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_int"), return 0;);
	   return ((JSONWGNode*)node) -> as_int();
	   //return static_cast<json_int_t>(*((JSONWGNode*)node));
    }

    json_number json_as_float(json_const JSONWGNode * node){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_float"), return 0.0f;);
	   return ((JSONWGNode*)node) -> as_float();
	   //return static_cast<json_number>(*((JSONWGNode*)node));
    }

    json_bool_t json_as_bool(json_const JSONWGNode * node){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_bool"), return false;);
	   return ((JSONWGNode*)node) -> as_bool();
	   //return (json_bool_t)static_cast<bool>(*((JSONWGNode*)node));
    }

	#ifdef JSON_CASTABLE
		JSONWGNode * json_as_node(json_const JSONWGNode * node){
		   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_node"), return 0;);
		   return MANAGER_INSERT(JSONWGNode::newJSONWGNode_Shallow(((JSONWGNode*)node) -> as_node()));
		}

		JSONWGNode * json_as_array(json_const JSONWGNode * node){
		   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_array"), return 0;);
		   return MANAGER_INSERT(JSONWGNode::newJSONWGNode_Shallow(((JSONWGNode*)node) -> as_array()));
		}
	#endif

    #if defined(JSON_BINARY) || defined(JSON_EXPOSE_BASE64)
	   static void * returnDecode64(const std::string & result, unsigned long * size) json_nothrow json_cold;
	   static void * returnDecode64(const std::string & result, unsigned long * size) json_nothrow {
		  const size_t len = result.length();
		  if (json_likely(size)) *size = (json_index_t)len;
		  #ifdef JSON_SAFE
			 if (json_unlikely(result.empty())) return 0;
		  #endif
		  #ifdef JSON_MEMORY_MANAGE
			 return json_global(STRING_HANDLER).insert(std::memcpy(json_malloc<char>(len), result.data(), len));
		  #else
			 return std::memcpy(json_malloc<char>(len), result.data(), len);
		  #endif
	   }
    #endif

    #ifdef JSON_BINARY
	   void * json_as_binary(json_const JSONWGNode * node, unsigned long * size){
		  JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_as_binary"), if (size){*size = 0;} return 0;);
		  return returnDecode64(((JSONWGNode*)node) -> as_binary(), size);

	   }
    #endif

    #ifdef JSON_EXPOSE_BASE64
	   #include "JSON_Base64.h"
	   json_char * json_encode64(json_const void * binary, json_index_t bytes){
		  const json_string result(JSONBase64::json_encode64((const unsigned char *)binary, (size_t)bytes));
		  #ifdef JSON_MEMORY_MANAGE
			 return (json_char*)json_global(STRING_HANDLER).insert((json_char*)std::memcpy(json_malloc<json_char>(result.length() + 1), result.c_str(), (result.length() + 1) * sizeof(json_char)));
		  #else
			 return (json_char*)std::memcpy(json_malloc<json_char>(result.length() + 1), result.c_str(), (result.length() + 1) * sizeof(json_char));
		  #endif
	   }

	   void * json_decode64(const json_char * text, unsigned long * size){
		  return returnDecode64(JSONBase64::json_decode64(text), size);
	   }
    #endif

    #ifdef JSON_WRITE_PRIORITY
	   json_char * json_write(json_const JSONWGNode * node){
		  JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_write"), return toCString(EMPTY_CSTRING););
		  return toCString(((JSONWGNode*)node) -> write());
	   }

	   json_char * json_write_formatted(json_const JSONWGNode * node){
		  JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_write_formatted"), return toCString(EMPTY_CSTRING););
		  return toCString(((JSONWGNode*)node) -> write_formatted());
	   }
    #endif

    //modifiers
    void json_set_name(JSONWGNode * node, json_const json_char * name){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_name"), return;);
	   JSON_ASSERT_SAFE(name, JSON_TEXT("null name to json_set_name"), name = EMPTY_CSTRING;);
	   ((JSONWGNode*)node) -> set_name(TOCONST_CSTR(name));
    }

    #ifdef JSON_COMMENTS
	   void json_set_comment(JSONWGNode * node, json_const json_char * comment){
		  JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_set_comment"), return;);
		  JSON_ASSERT_SAFE(comment, JSON_TEXT("null name to json_set_comment"), comment = EMPTY_CSTRING;);
		  ((JSONWGNode*)node) -> set_comment(TOCONST_CSTR(comment));
	   }
    #endif

    void json_clear(JSONWGNode * node){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_clear"), return;);
	   ((JSONWGNode*)node) -> clear();
    }

    void json_nullify(JSONWGNode * node){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_nullify"), return;);
	   ((JSONWGNode*)node) -> nullify();
    }

    void json_swap(JSONWGNode * node, JSONWGNode * node2){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_swap"), return;);
	   JSON_ASSERT_SAFE(node2, JSON_TEXT("null node to json_swap"), return;);
	   ((JSONWGNode*)node) -> swap(*(JSONWGNode*)node2);
    }

    void json_merge(JSONWGNode * node, JSONWGNode * node2){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_merge"), return;);
	   JSON_ASSERT_SAFE(node2, JSON_TEXT("null node to json_merge"), return;);
	   ((JSONWGNode*)node) -> merge(*(JSONWGNode*)node2);
    }

    #if !defined(JSON_PREPARSE) && defined(JSON_READ_PRIORITY)
	   void json_preparse(JSONWGNode * node){
		  JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_preparse"), return;);
		  ((JSONWGNode*)node) -> preparse();
	   }
    #endif

    #ifdef JSON_BINARY
	   void json_set_binary(JSONWGNode * node, json_const void * data, unsigned long length){
		  JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_swap"), return;);
		  JSON_ASSERT_SAFE(data, JSON_TEXT("null data to json_set_binary"), *((JSONWGNode*)node) = EMPTY_CSTRING; return;);
		  ((JSONWGNode*)node) -> set_binary((unsigned char *)data, (size_t)length);
	   }
    #endif

    #ifdef JSON_CASTABLE
	   void json_cast(JSONWGNode * node, char type){
		  JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_cast"), return;);
		  ((JSONWGNode*)node) -> cast(type);
	   }
    #endif

    //children access
    void json_reserve(JSONWGNode * node, json_index_t siz){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_reserve"), return;);
	   ((JSONWGNode*)node) -> reserve(siz);
    }

    JSONWGNode * json_at(JSONWGNode * node, unsigned int pos){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_at"), return 0;);
	   json_try {
		  return &((JSONWGNode*)node) -> at(pos);
	   } json_catch (std::out_of_range, (void)0; )
	   #ifndef JSON_NO_EXCEPTIONS
		  return 0;
	   #endif
    }

    JSONWGNode * json_get(JSONWGNode * node, json_const json_char * name){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_get"), return 0;);
	   JSON_ASSERT_SAFE(name, JSON_TEXT("null node to json_get.  Did you mean to use json_at?"), return 0;);
	   json_try {
		  return &((JSONWGNode*)node) -> at(TOCONST_CSTR(name));
	   } json_catch (std::out_of_range, (void)0; )
	   #ifndef JSON_NO_EXCEPTIONS
		  return 0;
	   #endif
    }


	#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	   JSONWGNode * json_get_nocase(JSONWGNode * node, json_const json_char * name){
		  JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_at_nocase"), return 0;);
		  JSON_ASSERT_SAFE(name, JSON_TEXT("null name to json_at_nocase"), return 0;);
		  json_try {
			 return &((JSONWGNode*)node) -> at_nocase(TOCONST_CSTR(name));
		  } json_catch (std::out_of_range, (void)0; )
		  #ifndef JSON_NO_EXCEPTIONS
			 return 0;
		  #endif
	   }

	   JSONWGNode * json_pop_back_nocase(JSONWGNode * node, json_const json_char * name){
		  JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_pop_back_nocase"), return 0;);
		  JSON_ASSERT_SAFE(name, JSON_TEXT("null name to json_pop_back_nocase"), return 0;);
		  return MANAGER_INSERT(((JSONWGNode*)node) -> pop_back_nocase(TOCONST_CSTR(name)));
	   }
    #endif

    void json_push_back(JSONWGNode * node, JSONWGNode * node2){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_push_back"), return;);
	   JSON_ASSERT_SAFE(node2, JSON_TEXT("null node2 to json_push_back"), return;);
	   #ifdef JSON_MEMORY_MANAGE
		  json_global(NODE_HANDLER).remove(node2);
	   #endif
	   ((JSONWGNode*)node) -> push_back((JSONWGNode*)node2);
    }

    JSONWGNode * json_pop_back_at(JSONWGNode * node, unsigned int pos){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_pop_back_i"), return 0;);
	   return MANAGER_INSERT(((JSONWGNode*)node) -> pop_back(pos));
    }

    JSONWGNode * json_pop_back(JSONWGNode * node, json_const json_char * name){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_pop_back"), return 0;);
	   JSON_ASSERT_SAFE(name, JSON_TEXT("null name to json_pop_back.  Did you mean to use json_pop_back_at?"), return 0;);
	   return MANAGER_INSERT(((JSONWGNode*)node) -> pop_back(TOCONST_CSTR(name)));
    }

    #ifdef JSON_ITERATORS
	   JSONWGNode_ITERATOR json_find(JSONWGNode * node, json_const json_char * name){
		  return (JSONWGNode_ITERATOR)(((JSONWGNode*)node) -> find(TOCONST_CSTR(name)));
	   }

	   #ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
		  JSONWGNode_ITERATOR json_find_nocase(JSONWGNode * node, json_const json_char * name){
			 return (JSONWGNode_ITERATOR)(((JSONWGNode*)node) -> find_nocase(TOCONST_CSTR(name)));
		  }
	   #endif

	   JSONWGNode_ITERATOR json_erase(JSONWGNode * node, JSONWGNode_ITERATOR it){
		  return (JSONWGNode_ITERATOR)(((JSONWGNode*)node) -> erase((JSONWGNode**)it));
	   }

	   JSONWGNode_ITERATOR json_erase_multi(JSONWGNode * node, JSONWGNode_ITERATOR start, JSONWGNode_ITERATOR end){
		  return (JSONWGNode_ITERATOR)(((JSONWGNode*)node) -> erase((JSONWGNode**)start, (JSONWGNode**)end));
	   }

	   JSONWGNode_ITERATOR json_insert(JSONWGNode * node, JSONWGNode_ITERATOR it, JSONWGNode * node2){
		  #ifdef JSON_MEMORY_MANAGE
			 json_global(NODE_HANDLER).remove(node2);
		  #endif
		  return (JSONWGNode_ITERATOR)(((JSONWGNode*)node) -> insert((JSONWGNode**)it, (JSONWGNode*)node2));
	   }

	   JSONWGNode_ITERATOR json_insert_multi(JSONWGNode * node, JSONWGNode_ITERATOR it, JSONWGNode_ITERATOR start, JSONWGNode_ITERATOR end){
		  return (JSONWGNode_ITERATOR)(((JSONWGNode*)node) -> insert((JSONWGNode**)it, (JSONWGNode**)start, (JSONWGNode**)end));
	   }

	   //iterator functions
	   JSONWGNode_ITERATOR json_begin(JSONWGNode * node){
		  return (JSONWGNode_ITERATOR)(((JSONWGNode*)node) -> begin());
	   }

	   JSONWGNode_ITERATOR json_end(JSONWGNode * node){
		  return (JSONWGNode_ITERATOR)(((JSONWGNode*)node) -> end());
	   }
    #endif

    //comparison
    json_bool_t json_equal(JSONWGNode * node, JSONWGNode * node2){
	   JSON_ASSERT_SAFE(node, JSON_TEXT("null node to json_equal"), return false;);
	   JSON_ASSERT_SAFE(node2, JSON_TEXT("null node2 to json_equal"), return false;);
	   return (json_bool_t)(*((JSONWGNode*)node) == *((JSONWGNode*)node2));
    }

#endif //JSON_LIBRARY
