#ifndef _MINOTAUR_LOCK_FREE_UTIL_H_
#define _MINOTAUR_LOCK_FREE_UTIL_H_
/**
  @file lf_util.h
  @author Wolfhead
*/
#define LF_CAS(a_ptr, a_oldVal, a_newVal) \
__sync_bool_compare_and_swap(a_ptr, a_oldVal, a_newVal)

#endif // _MINOTAUR_LOCK_FREE_UTIL_H_
