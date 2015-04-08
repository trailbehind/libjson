#include "JSONNode.h"

#define IMPLEMENT_CTOR(type)\
    JSONWGNode::JSONWGNode(const json_string & name_t, type value_t) json_nothrow : internal(internalWGJSONWGNode::newInternal()){\
	   internal -> Set(value_t);\
	   internal -> setname(name_t);\
	   LIBJSON_CTOR;\
    }
IMPLEMENT_FOR_ALL_TYPES(IMPLEMENT_CTOR)

#ifndef JSON_LIBRARY
    JSONWGNode::JSONWGNode(const json_string & name_t, const json_char * value_t) json_nothrow : internal(internalWGJSONWGNode::newInternal()){
	   internal -> Set(json_string(value_t));
	   internal -> setname(name_t);
	   LIBJSON_CTOR;
    }
#endif

#if (defined(JSON_PREPARSE) && defined(JSON_READ_PRIORITY))
    #include "JSONWorker.h"
    JSONWGNode JSONWGNode::stringType(const json_string & str){
        JSONWGNode res;
        res.set_name(json_global(EMPTY_JSON_STRING));
        #ifdef JSON_LESS_MEMORY
            res = JSONWGWorker::FixString(str, res.internal, false);
        #else
            res = JSONWGWorker::FixString(str, res.internal -> _string_encoded);
        #endif
        return res;
    }

    void JSONWGNode::set_name_(const json_string & newname) json_nothrow {
        #ifdef JSON_LESS_MEMORY
            json_string _newname = JSONWGWorker::FixString(newname, internal, true);
        #else
            json_string _newname = JSONWGWorker::FixString(newname, internal -> _name_encoded);
        #endif
        set_name(_newname);
    }
#endif

#ifdef JSON_CASTABLE
    JSONWGNode JSONWGNode::as_node(void) const json_nothrow {
	   JSON_CHECK_INTERNAL();
	   if (type() == JSON_NODE){
		  return *this;
	   } else if (type() == JSON_ARRAY){
		  JSONWGNode res(duplicate());
		  res.internal -> _type = JSON_NODE;
		  return res;
	   }
	   #ifdef JSON_MUTEX_CALLBACKS
		  if (internal -> mylock != 0){
			 JSONWGNode res(JSON_NODE);
			 res.set_mutex(internal -> mylock);
			 return res;
		  }
	   #endif
	   return JSONWGNode(JSON_NODE);
    }

    JSONWGNode JSONWGNode::as_array(void) const json_nothrow {
	   JSON_CHECK_INTERNAL();
	   if (type() == JSON_ARRAY){
		  return *this;
	   } else if (type() == JSON_NODE){
		  JSONWGNode res(duplicate());
		  res.internal -> _type = JSON_ARRAY;
		  json_foreach(res.internal -> CHILDREN, runner){
			 (*runner) -> clear_name();
		  }
		  return res;
	   }
	   #ifdef JSON_MUTEX_CALLBACKS
		  if (internal -> mylock != 0){
			 JSONWGNode res(JSON_ARRAY);
			 res.set_mutex(internal -> mylock);
			 return res;
		  }
	   #endif
	   return JSONWGNode(JSON_ARRAY);
    }

    void JSONWGNode::cast(char newtype) json_nothrow {
	   JSON_CHECK_INTERNAL();
	   if (newtype == type()) return;

	   switch(newtype){
		  case JSON_NULL:
			 nullify();
			 return;
		  case JSON_STRING:
			 *this = as_string();
			 return;
		  case JSON_NUMBER:
			 *this = as_float();
			 return;
		  case JSON_BOOL:
			 *this = as_bool();
			 return;
		  case JSON_ARRAY:
			 *this = as_array();
			 return;
		  case JSON_NODE:
			 *this = as_node();
			 return;
	   }
	   JSON_FAIL(JSON_TEXT("cast to unknown type"));
    }
#endif

//different just to supress the warning
#ifdef JSON_REF_COUNT
void JSONWGNode::merge(JSONWGNode & other) json_nothrow {
#else
void JSONWGNode::merge(JSONWGNode &) json_nothrow {
#endif
    JSON_CHECK_INTERNAL();
    #ifdef JSON_REF_COUNT
	   if (internal == other.internal) return;
	   JSON_ASSERT(*this == other, JSON_TEXT("merging two nodes that aren't equal"));
	   if (internal -> refcount < other.internal -> refcount){
		  *this = other;
	   } else {
		  other = *this;
	   }
    #endif
}

#ifdef JSON_REF_COUNT
    void JSONWGNode::merge(JSONWGNode * other) json_nothrow {
	   JSON_CHECK_INTERNAL();
	   if (internal == other -> internal) return;
	   *other = *this;
    }

    //different just to supress the warning
    void JSONWGNode::merge(unsigned int num, ...) json_nothrow {
#else
    void JSONWGNode::merge(unsigned int, ...) json_nothrow {
#endif
    JSON_CHECK_INTERNAL();
    #ifdef JSON_REF_COUNT
	   va_list args;
	   va_start(args, num);
	   for(unsigned int i = 0; i < num; ++i){
		  merge(va_arg(args, JSONWGNode*));
	   }
	   va_end(args);
    #endif
}

JSONWGNode JSONWGNode::duplicate(void) const json_nothrow {
    JSON_CHECK_INTERNAL();
    JSONWGNode mycopy(*this);
    #ifdef JSON_REF_COUNT
	   JSON_ASSERT(internal == mycopy.internal, JSON_TEXT("copy ctor failed to ref count correctly"));
	   mycopy.makeUniqueInternal();
    #endif
    JSON_ASSERT(internal != mycopy.internal, JSON_TEXT("makeUniqueInternal failed"));
    return mycopy;
}

JSONWGNode & JSONWGNode::at(json_index_t pos) json_throws(std::out_of_range) {
    JSON_CHECK_INTERNAL();
    if (json_unlikely(pos >= internal -> size())){
	   JSON_FAIL(JSON_TEXT("at() out of bounds"));
	   json_throw(std::out_of_range(json_global(EMPTY_STD_STRING)));
    }
    return (*this)[pos];
}

const JSONWGNode & JSONWGNode::at(json_index_t pos) const json_throws(std::out_of_range) {
    JSON_CHECK_INTERNAL();
    if (json_unlikely(pos >= internal -> size())){
	   JSON_FAIL(JSON_TEXT("at() const out of bounds"));
	   json_throw(std::out_of_range(json_global(EMPTY_STD_STRING)));
    }
    return (*this)[pos];
}

JSONWGNode & JSONWGNode::operator[](json_index_t pos) json_nothrow {
    JSON_CHECK_INTERNAL();
    JSON_ASSERT(pos < internal -> size(), JSON_TEXT("[] out of bounds"));
    makeUniqueInternal();
    return *(internal -> at(pos));
}

const JSONWGNode & JSONWGNode::operator[](json_index_t pos) const json_nothrow {
    JSON_CHECK_INTERNAL();
    JSON_ASSERT(pos < internal -> size(), JSON_TEXT("[] const out of bounds"));
    return *(internal -> at(pos));
}

JSONWGNode & JSONWGNode::at(const json_string & name_t) json_throws(std::out_of_range) {
    JSON_CHECK_INTERNAL();
    JSON_ASSERT(type() == JSON_NODE, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("at"));
    makeUniqueInternal();
    if (JSONWGNode ** res = internal -> at(name_t)){
	   return *(*res);
    }
    JSON_FAIL(json_string(JSON_TEXT("at could not find child by name: ")) + name_t);
    json_throw(std::out_of_range(json_global(EMPTY_STD_STRING)));
}

const JSONWGNode & JSONWGNode::at(const json_string & name_t) const json_throws(std::out_of_range) {
    JSON_CHECK_INTERNAL();
    JSON_ASSERT(type() == JSON_NODE, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("at"));
    if (JSONWGNode ** res = internal -> at(name_t)){
	   return *(*res);
    }
    JSON_FAIL(json_string(JSON_TEXT("at const could not find child by name: ")) + name_t);
    json_throw(std::out_of_range(json_global(EMPTY_STD_STRING)));
}

#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
    JSONWGNode & JSONWGNode::at_nocase(const json_string & name_t) json_throws(std::out_of_range) {
	   JSON_CHECK_INTERNAL();
	   JSON_ASSERT(type() == JSON_NODE, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("at_nocase"));
	   makeUniqueInternal();
	   if (JSONWGNode ** res = internal -> at_nocase(name_t)){
		  return *(*res);
	   }
	   JSON_FAIL(json_string(JSON_TEXT("at_nocase could not find child by name: ")) + name_t);
	   json_throw(std::out_of_range(json_global(EMPTY_STD_STRING)));
    }

    const JSONWGNode & JSONWGNode::at_nocase(const json_string & name_t) const json_throws(std::out_of_range) {
	   JSON_CHECK_INTERNAL();
	   JSON_ASSERT(type() == JSON_NODE, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("at_nocase"));
	   if (JSONWGNode ** res = internal -> at_nocase(name_t)){
		  return *(*res);
	   }
	   JSON_FAIL(json_string(JSON_TEXT("at_nocase const could not find child by name: ")) + name_t);
	   json_throw(std::out_of_range(json_global(EMPTY_STD_STRING)));
    }
#endif

#ifndef JSON_LIBRARY
    struct auto_delete {
	   public:
		  auto_delete(JSONWGNode * node) json_nothrow : mynode(node){};
		  ~auto_delete(void) json_nothrow { JSONWGNode::deleteJSONWGNode(mynode); };
		  JSONWGNode * mynode;
	   private:
		  auto_delete(const auto_delete &);
		  auto_delete & operator = (const auto_delete &);
    };
#endif

JSONWGNode JSON_PTR_LIB JSONWGNode::pop_back(json_index_t pos) json_throws(std::out_of_range) {
    JSON_CHECK_INTERNAL();
    if (json_unlikely(pos >= internal -> size())){
	   JSON_FAIL(JSON_TEXT("pop_back out of bounds"));
	   json_throw(std::out_of_range(json_global(EMPTY_STD_STRING)));
    }
    makeUniqueInternal();
    #ifdef JSON_LIBRARY
	   return internal -> pop_back(pos);
    #else
	   auto_delete temp(internal -> pop_back(pos));
	   return *temp.mynode;
    #endif
}

JSONWGNode JSON_PTR_LIB JSONWGNode::pop_back(const json_string & name_t) json_throws(std::out_of_range) {
    JSON_CHECK_INTERNAL();
    JSON_ASSERT(type() == JSON_NODE, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("pop_back"));
    #ifdef JSON_LIBRARY
	   return internal -> pop_back(name_t);
    #else
	   if (JSONWGNode * res = internal -> pop_back(name_t)){
		  auto_delete temp(res);
		  return *(temp.mynode);
	   }
	   JSON_FAIL(json_string(JSON_TEXT("pop_back const could not find child by name: ")) + name_t);
	   json_throw(std::out_of_range(json_global(EMPTY_STD_STRING)));
    #endif
}

#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
    JSONWGNode JSON_PTR_LIB JSONWGNode::pop_back_nocase(const json_string & name_t) json_throws(std::out_of_range) {
	   JSON_CHECK_INTERNAL();
	   JSON_ASSERT(type() == JSON_NODE, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("pop_back_no_case"));
	   #ifdef JSON_LIBRARY
		  return internal -> pop_back_nocase(name_t);
	   #else
		  if (JSONWGNode * res = internal -> pop_back_nocase(name_t)){
			 auto_delete temp(res);
			 return *(temp.mynode);
		  }
		  JSON_FAIL(json_string(JSON_TEXT("pop_back_nocase could not find child by name: ")) + name_t);
		  json_throw(std::out_of_range(json_global(EMPTY_STD_STRING)));
	   #endif
    }
#endif
		
#ifdef JSON_MEMORY_POOL
	#include "JSONMemoryPool.h"
	memory_pool<NODEPOOL> json_node_mempool;
#endif
		
void JSONWGNode::deleteJSONWGNode(JSONWGNode * ptr) json_nothrow {
	#ifdef JSON_MEMORY_POOL
		ptr -> ~JSONWGNode();
		json_node_mempool.deallocate((void*)ptr);
	#elif defined(JSON_MEMORY_CALLBACKS)
		ptr -> ~JSONWGNode();
		libjson_free<JSONWGNode>(ptr);
	#else
		delete ptr;
	#endif
}
		
inline JSONWGNode * _newJSONWGNode(const JSONWGNode & orig) {
	#ifdef JSON_MEMORY_POOL
		return new((JSONWGNode*)json_node_mempool.allocate()) JSONWGNode(orig);
	#elif defined(JSON_MEMORY_CALLBACKS)
		return new(json_malloc<JSONWGNode>(1)) JSONWGNode(orig);
	#else
		return new JSONWGNode(orig);
	#endif
}
		
JSONWGNode * JSONWGNode::newJSONWGNode(const JSONWGNode & orig    JSON_MUTEX_COPY_DECL) {
	#ifdef JSON_MUTEX_CALLBACKS
		if (parentMutex != 0){
			JSONWGNode * temp = _newJSONWGNode(orig);
			temp -> set_mutex(parentMutex);
			return temp;
		}
	#endif
	return _newJSONWGNode(orig);
}
		
JSONWGNode * JSONWGNode::newJSONWGNode(internalWGJSONWGNode * internal_t) {
	#ifdef JSON_MEMORY_POOL
		return new((JSONWGNode*)json_node_mempool.allocate()) JSONWGNode(internal_t);
	#elif defined(JSON_MEMORY_CALLBACKS)
		return new(json_malloc<JSONWGNode>(1)) JSONWGNode(internal_t);
	#else
		return new JSONWGNode(internal_t);
	#endif
}
		
JSONWGNode * JSONWGNode::newJSONWGNode_Shallow(const JSONWGNode & orig) {
	#ifdef JSON_MEMORY_POOL
		return new((JSONWGNode*)json_node_mempool.allocate()) JSONWGNode(true, const_cast<JSONWGNode &>(orig));
	#elif defined(JSON_MEMORY_CALLBACKS)
		return new(json_malloc<JSONWGNode>(1)) JSONWGNode(true, const_cast<JSONWGNode &>(orig));
	#else
		return new JSONWGNode(true, const_cast<JSONWGNode &>(orig));
	#endif
}
		

