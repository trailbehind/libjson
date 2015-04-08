#include "JSONNode.h"

#ifdef JSON_ITERATORS
    #ifdef JSON_REF_COUNT
	   #define JSON_ASSERT_UNIQUE(x) JSON_ASSERT(internal -> refcount == 1, json_string(JSON_TEXT(x)) + JSON_TEXT(" in non single reference"))
    #else
	   #define JSON_ASSERT_UNIQUE(x) (void)0
    #endif

    #ifdef JSON_MUTEX_CALLBACKS
	   #define JSON_MUTEX_COPY2 ,internal -> mylock
    #else
	   #define JSON_MUTEX_COPY2
    #endif

JSONWGNode::json_iterator JSONWGNode::find(const json_string & name_t) json_nothrow {
    JSON_CHECK_INTERNAL();
    JSON_ASSERT(type() == JSON_NODE, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("find"));
    makeUniqueInternal();
    if (JSONWGNode ** res = internal -> at(name_t)){
	   return ptr_to_json_iterator(res);
    }
    return end();
}

#ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
    JSONWGNode::json_iterator JSONWGNode::find_nocase(const json_string & name_t) json_nothrow {
	   JSON_CHECK_INTERNAL();
	   JSON_ASSERT(type() == JSON_NODE, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("find_nocase"));
	   makeUniqueInternal();
	   if (JSONWGNode ** res = internal -> at_nocase(name_t)){
		  return ptr_to_json_iterator(res);
	   }
	   return end();
    }
#endif

JSONWGNode::json_iterator JSONWGNode::erase(json_iterator pos) json_nothrow {
    JSON_CHECK_INTERNAL();
    JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("erase"));
    JSON_ASSERT_UNIQUE("erase 1");
    JSON_ASSERT_SAFE(pos < end(), JSON_TEXT("erase out of range"), return end(););
    JSON_ASSERT_SAFE(pos >= begin(), JSON_TEXT("erase out of range"), return begin(););
    deleteJSONWGNode(*(json_iterator_ptr(pos)));
    internal -> CHILDREN -> erase(json_iterator_ptr(pos));
    return (empty()) ? end() : pos;
}

JSONWGNode::json_iterator JSONWGNode::erase(json_iterator _start, const json_iterator & _end) json_nothrow {
    if (_start == _end) return _start;
    JSON_CHECK_INTERNAL();
    JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("erase"));
    JSON_ASSERT_UNIQUE("erase 3");
    JSON_ASSERT_SAFE(_start <= end(), JSON_TEXT("erase out of lo range"), return end(););
    JSON_ASSERT_SAFE(_end <= end(), JSON_TEXT("erase out of hi range"), return end(););
    JSON_ASSERT_SAFE(_start >= begin(), JSON_TEXT("erase out of lo range"), return begin(););
    JSON_ASSERT_SAFE(_end >= begin(), JSON_TEXT("erase out of hi range"), return begin(););
    for (JSONWGNode ** pos = json_iterator_ptr(_start); pos < json_iterator_ptr(_end); ++pos){
	   deleteJSONWGNode(*pos);
    }

    internal -> CHILDREN -> erase(json_iterator_ptr(_start), (json_index_t)(json_iterator_ptr(_end) - json_iterator_ptr(_start)));
    return (empty()) ? end() : _start;
}

#ifdef JSON_LIBRARY
JSONWGNode::json_iterator JSONWGNode::insert(json_iterator pos, JSONWGNode * x) json_nothrow {
#else
JSONWGNode::json_iterator JSONWGNode::insert(json_iterator pos, const JSONWGNode & x) json_nothrow {
#endif
    JSON_CHECK_INTERNAL();
    JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("insert"));
    JSON_ASSERT_UNIQUE("insert 1");
    if (json_iterator_ptr(pos) >= internal -> CHILDREN -> end()){
	   internal -> push_back(x);
	   return end() - 1;
    }
    JSON_ASSERT_SAFE(pos >= begin(), JSON_TEXT("insert out of lo range"), return begin(););
    #ifdef JSON_LIBRARY
	   internal -> CHILDREN -> insert(json_iterator_ptr(pos), x);
    #else
	   internal -> CHILDREN -> insert(json_iterator_ptr(pos), newJSONWGNode(x));
    #endif
    return pos;
}

JSONWGNode::json_iterator JSONWGNode::insertFFF(json_iterator pos, JSONWGNode ** const _start, JSONWGNode ** const _end) json_nothrow {
    JSON_CHECK_INTERNAL();
    JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("insertFFF"));
    JSON_ASSERT_UNIQUE("insertFFF");
    JSON_ASSERT_SAFE(pos <= end(), JSON_TEXT("insert out of high range"), return end(););
    JSON_ASSERT_SAFE(pos >= begin(), JSON_TEXT("insert out of low range"), return begin(););
    const json_index_t num = (json_index_t)(_end - _start);
    json_auto<JSONWGNode *> mem(num);
    JSONWGNode ** runner = mem.ptr;
    for (JSONWGNode ** po = _start; po < _end; ++po){
	   *runner++ = newJSONWGNode(*(*po)  JSON_MUTEX_COPY2);
    }
    internal -> CHILDREN -> insert(json_iterator_ptr(pos), mem.ptr, num);
    return pos;
}

#ifndef JSON_LIBRARY
    JSONWGNode::const_iterator JSONWGNode::find(const json_string & name_t) const json_nothrow {
	   JSON_CHECK_INTERNAL();
	   JSON_ASSERT(type() == JSON_NODE, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("find"));
	   if (JSONWGNode ** res = internal -> at(name_t)){
		  return JSONWGNode::const_iterator(res);
	   }
	   return JSONWGNode::const_iterator(internal -> end());
    }

    #ifdef JSON_CASE_INSENSITIVE_FUNCTIONS
	   JSONWGNode::const_iterator JSONWGNode::find_nocase(const json_string & name_t) const json_nothrow {
		  JSON_CHECK_INTERNAL();
		  JSON_ASSERT(type() == JSON_NODE, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("find_nocase"));
		  if (JSONWGNode ** res = internal -> at_nocase(name_t)){
			 return JSONWGNode::const_iterator(res);
		  }
		  return JSONWGNode::const_iterator(internal -> end());
	   }
    #endif

    JSONWGNode::reverse_iterator JSONWGNode::erase(reverse_iterator pos) json_nothrow {
	   JSON_CHECK_INTERNAL();
	   JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("erase"));
	   JSON_ASSERT_UNIQUE("erase 2");
	   JSON_ASSERT_SAFE(pos < rend(), JSON_TEXT("erase out of range"), return rend(););
	   JSON_ASSERT_SAFE(pos >= rbegin(), JSON_TEXT("erase out of range"), return rbegin(););
	   deleteJSONWGNode(*(pos.it));
	   internal -> CHILDREN -> erase(pos.it);
	   return (empty()) ? rend() : pos + 1;
    }

    JSONWGNode::reverse_iterator JSONWGNode::erase(reverse_iterator _start, const reverse_iterator & _end) json_nothrow {
	   if (_start == _end) return _start;
	   JSON_CHECK_INTERNAL();
	   JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("erase"));
	   JSON_ASSERT_UNIQUE("erase 4");
	   JSON_ASSERT_SAFE(_start <= rend(), JSON_TEXT("erase out of lo range"), return rend(););
	   JSON_ASSERT_SAFE(_end <= rend(), JSON_TEXT("erase out of hi range"), return rend(););
	   JSON_ASSERT_SAFE(_start >= rbegin(), JSON_TEXT("erase out of lo range"), return rbegin(););
	   JSON_ASSERT_SAFE(_end >= rbegin(), JSON_TEXT("erase out of hi range"), return rbegin(););
	   for (JSONWGNode ** pos = _start.it; pos > _end.it; --pos){
		  deleteJSONWGNode(*pos);
	   }
	   const json_index_t num = (json_index_t)(_start.it - _end.it);
	   internal -> CHILDREN -> erase(_end.it + 1, num, _start.it);
	   return (empty()) ? rend() : _start + num;
    }

    JSONWGNode::reverse_iterator JSONWGNode::insert(reverse_iterator pos, const JSONWGNode & x) json_nothrow {
	   JSON_CHECK_INTERNAL();
	   JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("insert"));
	   JSON_ASSERT_UNIQUE("insert 1");
	   if (pos.it < internal -> CHILDREN -> begin()){
		  internal -> push_front(x);
		  return rend() - 1;
	   }
	   JSON_ASSERT_SAFE(pos >= rbegin(), JSON_TEXT("insert out of range"), return rbegin(););
	   internal -> CHILDREN -> insert(++pos.it, newJSONWGNode(x), true);
	   return pos;
    }

    JSONWGNode::reverse_iterator JSONWGNode::insertRFF(reverse_iterator pos, JSONWGNode ** const _start, JSONWGNode ** const _end) json_nothrow {
	   JSON_CHECK_INTERNAL();
	   JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("insertRFF"));
	   JSON_ASSERT_UNIQUE("insert RFF");
	   JSON_ASSERT_SAFE(pos <= rend(), JSON_TEXT("insert out of range"), return rend(););
	   JSON_ASSERT_SAFE(pos >= rbegin(), JSON_TEXT("insert out of range"), return rbegin(););
	   const json_index_t num = (json_index_t)(_end - _start);
	   json_auto<JSONWGNode *> mem(num);
	   JSONWGNode ** runner = mem.ptr + num;
	   for (JSONWGNode ** po = _start; po < _end; ++po){  //fill it backwards
		  *(--runner) = newJSONWGNode(*(*po)    JSON_MUTEX_COPY2);
	   }
	   internal -> CHILDREN -> insert(++pos.it, mem.ptr, num);
	   return pos - num + 1;
    }

    JSONWGNode::iterator JSONWGNode::insertFRR(json_iterator pos, JSONWGNode ** const _start, JSONWGNode ** const _end) json_nothrow {
	   JSON_CHECK_INTERNAL();
	   JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("insertFRR"));
	   JSON_ASSERT_UNIQUE("insert FRR");
	   JSON_ASSERT_SAFE(pos <= end(), JSON_TEXT("insert out of range"), return end(););
	   JSON_ASSERT_SAFE(pos >= begin(), JSON_TEXT("insert out of range"), return begin(););
	   const json_index_t num = (json_index_t)(_start - _end);
	   json_auto<JSONWGNode *> mem(num);
	   JSONWGNode ** runner = mem.ptr;
	   for (JSONWGNode ** po = _start; po > _end; --po){
		  *runner++ = newJSONWGNode(*(*po)    JSON_MUTEX_COPY2);
	   }
	   internal -> CHILDREN -> insert(pos.it, mem.ptr, num);
	   return pos;
    }

    JSONWGNode::reverse_iterator JSONWGNode::insertRRR(reverse_iterator pos, JSONWGNode ** const _start, JSONWGNode ** const _end) json_nothrow {
	   JSON_CHECK_INTERNAL();
	   JSON_ASSERT(type() == JSON_NODE || type() == JSON_ARRAY, json_global(ERROR_NON_ITERATABLE) + JSON_TEXT("insertRRR"));
	   JSON_ASSERT_UNIQUE("insert RRR");
	   JSON_ASSERT_SAFE(pos <= rend(), JSON_TEXT("insert out of range"), return rend(););
	   JSON_ASSERT_SAFE(pos >= rbegin(), JSON_TEXT("insert out of range"), return rbegin(););
	   const json_index_t num = (json_index_t)(_start - _end);
	   json_auto<JSONWGNode *> mem(num);
	   JSONWGNode ** runner = mem.ptr;
	   for (JSONWGNode ** po = _start; po > _end; --po){
		  *runner++ = newJSONWGNode(*(*po)    JSON_MUTEX_COPY2);
	   }
	   internal -> CHILDREN -> insert(++pos.it, mem.ptr, num);
	   return pos - num + 1;
    }
#endif

#endif
