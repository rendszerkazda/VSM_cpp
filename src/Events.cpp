#include "vsm/Events.h"

#include <algorithm>

namespace vsm {

void EventBus::subscribe(IObserver& observer) {
    if (std::find(observers_.begin(), observers_.end(), &observer) == observers_.end()) {
        observers_.push_back(&observer);
    }
}

void EventBus::unsubscribe(IObserver& observer) {
    observers_.erase(std::remove(observers_.begin(), observers_.end(), &observer),
                     observers_.end());
}

void EventBus::publish(const Event& event) const {
    for (IObserver* observer : observers_) {
        observer->onEvent(event);
    }
}

}  // namespace vsm
