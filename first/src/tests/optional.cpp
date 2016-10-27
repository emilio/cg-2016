#include "tools/Optional.h"
#include "tests/Utils.h"

#include <memory>

static size_t gDestructorCalled = 0;

class WithDestructor {
public:
  explicit WithDestructor(bool) : m_valid(true) {}

  bool m_valid;

  WithDestructor& operator=(WithDestructor&& aOther) {
    m_valid = aOther.m_valid;
    aOther.m_valid = false;
    return *this;
  }

  WithDestructor(const WithDestructor& aOther) = delete;

  ~WithDestructor() {
    if (m_valid)
      gDestructorCalled++;
  }
};

int main() {
  Optional<int> maybeTexture;
  ASSERT(!maybeTexture.isSome());
  ASSERT(!maybeTexture);

  maybeTexture.set(0);

  ASSERT(maybeTexture.isSome());
  ASSERT(maybeTexture);
  ASSERT(*maybeTexture == maybeTexture.value());
  ASSERT(*maybeTexture == 0);

  {
    Optional<std::unique_ptr<WithDestructor>> maybeWithDestructor;

    maybeWithDestructor.set(std::make_unique<WithDestructor>(true));

    maybeWithDestructor.clear();
    maybeWithDestructor.set(std::make_unique<WithDestructor>(false));
  }

  ASSERT(gDestructorCalled == 2);

  {
    Optional<WithDestructor> d;
    d.set(true);

    Optional<WithDestructor> e;
    e.set(false);

    d = std::move(e);

    ASSERT(gDestructorCalled == 3);
  }

  ASSERT(gDestructorCalled == 4);
  return 0;
}
