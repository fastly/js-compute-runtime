#include "core/event_loop.h"
#include "host_interface/host_api.h"
#include <chrono>
#include <list>
#include <vector>

namespace core {

namespace {

using TimerArgumentsVector = std::vector<JS::Heap<JS::Value>>;

class Timer {
public:
  uint32_t id;
  JS::Heap<JSObject *> callback;
  TimerArgumentsVector arguments;
  uint32_t delay;
  std::chrono::system_clock::time_point deadline;
  bool repeat;

  Timer(uint32_t id, JS::HandleObject callback, uint32_t delay, JS::HandleValueVector args,
        bool repeat)
      : id(id), callback(callback), delay(delay), repeat(repeat) {
    deadline = std::chrono::system_clock::now() + std::chrono::system_clock::duration(delay * 1000);
    arguments.reserve(args.length());
    for (auto &arg : args) {
      arguments.push_back(JS::Heap(arg));
    }
  }

  void trace(JSTracer *trc) {
    JS::TraceEdge(trc, &callback, "Timer callback");
    for (auto &arg : arguments) {
      JS::TraceEdge(trc, &arg, "Timer callback arguments");
    }
  }
};

class ScheduledTimers {
public:
  Timer *first() {
    if (std::empty(timers)) {
      return nullptr;
    } else {
      return timers.front().get();
    }
  }

private:
  std::list<std::unique_ptr<Timer>> timers;

  // `repeat_first` must only be called if the `timers` list is not empty
  // The caller of repeat_first needs to check the `timers` list is not empty
  void repeat_first() {
    MOZ_ASSERT(!this->timers.empty());
    auto timer = std::move(this->timers.front());
    timers.pop_front();
    timer->deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(timer->delay);
    add_timer(std::move(timer));
  }

public:
  void add_timer(std::unique_ptr<Timer> timer) {
    auto iter = timers.begin();

    for (; iter != timers.end(); iter++) {
      if ((*iter)->deadline > timer->deadline) {
        break;
      }
    }

    timers.insert(iter, std::move(timer));
  }

  bool empty() { return timers.empty(); }

  void remove_timer(uint32_t id) {
    auto it =
        std::find_if(timers.begin(), timers.end(), [id](auto &timer) { return timer->id == id; });
    if (it != timers.end()) {
      timers.erase(it);
    }
  }

  bool run_first_timer(JSContext *cx) {
    JS::RootedValue fun_val(cx);
    JS::RootedVector<JS::Value> argv(cx);
    uint32_t id;
    {
      auto *timer = first();
      MOZ_ASSERT(timer);
      MOZ_ASSERT(std::chrono::system_clock::now() > timer->deadline);
      id = timer->id;
      JS::RootedObject fun(cx, timer->callback);
      fun_val.setObject(*fun.get());
      if (!argv.initCapacity(timer->arguments.size())) {
        JS_ReportOutOfMemory(cx);
        return false;
      }

      for (auto &arg : timer->arguments) {
        argv.infallibleAppend(arg);
      }
    }

    JS::RootedObject fun(cx, &fun_val.toObject());

    JS::RootedValue rval(cx);
    if (!JS::Call(cx, JS::NullHandleValue, fun, argv, &rval)) {
      return false;
    }

    // Repeat / remove the first timer if it's still the one we just ran.
    auto *timer = first();
    if (timer && timer->id == id) {
      if (timer->repeat) {
        repeat_first();
      } else {
        remove_timer(timer->id);
      }
    }

    return true;
  }

  void trace(JSTracer *trc) {
    for (auto &timer : timers) {
      timer->trace(trc);
    }
  }
};

JS::PersistentRooted<js::UniquePtr<ScheduledTimers>> timers;

JS::PersistentRootedObjectVector *pending_async_tasks;
} // namespace

JSObject *AsyncTask::create(JSContext *cx, uint32_t handle, JS::HandleObject context,
                            JS::HandleObject promise, ProcessAsyncTask *process) {
  auto instance = JS_NewObjectWithGivenProto(cx, &class_, proto_obj);
  if (!instance) {
    return nullptr;
  }
  JS::SetReservedSlot(instance, static_cast<int32_t>(Slots::Handle), JS::Int32Value(handle));
  JS::SetReservedSlot(instance, static_cast<int32_t>(Slots::Context), JS::ObjectValue(*context));
  JS::SetReservedSlot(instance, static_cast<int32_t>(Slots::Process),
                      JS::PrivateValue((void *)process));
  if (promise) {
    JS::SetReservedSlot(instance, static_cast<int32_t>(Slots::Promise), JS::ObjectValue(*promise));
  } else {
    JS::SetReservedSlot(instance, static_cast<int32_t>(Slots::Promise), JS::NullValue());
  }

  return instance;
}

uint32_t AsyncTask::get_handle(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  auto result = JS::GetReservedSlot(self, static_cast<int32_t>(Slots::Handle)).toInt32();
  return result;
}
JSObject *AsyncTask::get_context(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  auto result = JS::GetReservedSlot(self, static_cast<int32_t>(Slots::Context)).toObjectOrNull();
  return result;
}
JSObject *AsyncTask::get_promise(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  auto result = JS::GetReservedSlot(self, static_cast<int32_t>(Slots::Promise)).toObjectOrNull();
  return result;
}
ProcessAsyncTask *AsyncTask::get_process(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  auto result = JS::GetReservedSlot(self, static_cast<int32_t>(Slots::Process)).toPrivate();
  return (ProcessAsyncTask *)(result);
}

bool EventLoop::process_pending_async_tasks(JSContext *cx) {
  MOZ_ASSERT(has_pending_async_tasks());

  uint32_t timeout = 0;
  if (!timers->empty()) {
    Timer *timer = timers->first();
    double diff =
        ceil<std::chrono::milliseconds>(timer->deadline - std::chrono::system_clock::now()).count();

    // If a timeout is already overdue, run it immediately and return.
    if (diff <= 0) {
      return timers->run_first_timer(cx);
    }

    timeout = diff;
    MOZ_ASSERT(timeout > 0);
  }

  size_t count = pending_async_tasks->length();
  std::vector<host_api::AsyncHandle> handles;
  handles.reserve(count);

  for (size_t i = 0; i < count; i++) {
    auto pending_obj = (*pending_async_tasks)[i];
    fprintf(stderr, "oooh %i\n", AsyncTask::get_handle(pending_obj));
    handles.push_back(host_api::AsyncHandle(AsyncTask::get_handle(pending_obj)));
  }

  auto res = host_api::AsyncHandle::select(handles, timeout);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto ret = res.unwrap();
  if (!ret.has_value()) {
    // The only path that leads us to the timeout expiring is if there were timers to run, otherwise
    // the timeout would have been set to `0` and we would have waited until an async handle was
    // ready.
    MOZ_ASSERT(!timers->empty());
    return timers->run_first_timer(cx);
  }

  // At this point we know that handles is not empty, and that ready_index is valid, both because
  // the timeout wasn't reached. If handles was empty and timeout was zero, we would have errored
  // out after the call to `select`. If the timeout was non-zero and handles was empty, the timeout
  // would expire and we would exit through the path that runs the first timer.
  auto ready_index = ret.value();

#ifdef DEBUG
  auto ready_handle = handles[ready_index];
  auto is_ready = ready_handle.is_ready();
  MOZ_ASSERT(!is_ready.is_err());
  MOZ_ASSERT(is_ready.unwrap());
#endif

  bool ok;
  auto ready_obj = (*pending_async_tasks)[ready_index];
  JS::RootedObject context(cx, AsyncTask::get_context(ready_obj));
  JS::RootedObject promise(cx, AsyncTask::get_promise(ready_obj));
  ok = (*AsyncTask::get_process(ready_obj))(cx, AsyncTask::get_handle(ready_obj), context, promise);

  pending_async_tasks->erase(ready_obj.address());
  return ok;
}

bool EventLoop::queue_async_task(JSObject *task) { return pending_async_tasks->append(task); }

uint32_t EventLoop::add_timer(JS::HandleObject callback, uint32_t delay,
                              JS::HandleValueVector arguments, bool repeat) {
  static uint32_t next_timer_id = 1;

  auto id = next_timer_id++;
  timers->add_timer(std::make_unique<Timer>(id, callback, delay, arguments, repeat));
  return id;
}

bool EventLoop::has_pending_async_tasks() {
  return pending_async_tasks->length() > 0 || !timers->empty();
}

void EventLoop::remove_timer(uint32_t id) { timers->remove_timer(id); }

void EventLoop::init(JSContext *cx) {

  pending_async_tasks = new JS::PersistentRootedObjectVector(cx);
  timers.init(cx, js::MakeUnique<ScheduledTimers>());
}

} // namespace core
