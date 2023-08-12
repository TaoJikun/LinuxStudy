#include "tp_st.h"

  thread_local unsigned thread_pool::my_index=0;
  thread_local work_stealing_queue* thread_pool::local_work_queue = nullptr; //= new work_stealing_queue();