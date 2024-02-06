// The backend used :)
#include <pthread.h>

#include <engine/core/logging.h>
#include <engine/core/thread.h>

thread_t thread_spawn(thread_fn* routine, void* arg) {
  pthread_t t;
  pthread_attr_t attr;

  if (pthread_attr_init(&attr)) {
    ERROR("Failed to create thread attribute");
    return -1;
  }

  if (pthread_create(&t, &attr, routine, arg)) {
    ERROR("Failed to spawn new thread");
  }

  // This compile-time check should only be run once, but i am lazy and have
  // spent too much time refactoring
  if (sizeof(thread_t) < sizeof(pthread_t)) {
    ERROR("THE SIZE AINT BIG ENOUGH!");
    exit(EXIT_FAILURE);
  }

  if(pthread_attr_destroy(&attr)) {
    ERROR("Failed to destroy thread attribute");
  }
  // this little trick might cost us 40 years
  return (thread_t)t;
}

void thread_join(thread_t thread) {
  if(pthread_join((pthread_t)thread, NULL)) {
    ERROR("Unable to join thread");
  }
}
