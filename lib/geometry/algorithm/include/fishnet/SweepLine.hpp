#pragma once

#include <set>
#include <queue>
#include <memory>

#include <fishnet/CollectionConcepts.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/GeometryObject.hpp>

namespace fishnet::geometry {

enum class EventType{
   INSERT,REMOVE
};
/**
 * @brief Generic Sweep Line template
 * Stores a Sweep Line Status (SLS), keeping track of the current state of the sweep
 * Stores an Event Queue for processing the events in the appropiate order
 * 
 * @tparam Input input type, stored in the SLS
 * @tparam Output output type, return after the sweep in a vector
 * @tparam SLSLess BiPredicate for sorting the SLS
 * @tparam EventQueueGreater BiPredicate for sorting the event points
 */
template<typename Input, typename Output,util::BiPredicate<Input> SLSLess = std::less<Input>,bool InsertEventsFirst = false, util::BiPredicate<fishnet::math::DEFAULT_NUMERIC> EventQueueGreater = std::greater<fishnet::math::DEFAULT_NUMERIC>>
class SweepLine{
public: 
    
    /**
     * @brief Interface for EventQueue events.
     * process(...) is executed whenever an element is polled from the EventQueue
     * getEventType() returns the type of Event (INSERT|REMOVE)
     * eventPoint() returns a numeric value for the Event to determine the order in the EventQueue
     */
    class IEvent{
    protected:
        const Input * obj; //non-owning ptr
    public:

        IEvent(const Input & obj):obj(&obj){}

        virtual void process(SweepLine<Input,Output,SLSLess,InsertEventsFirst,EventQueueGreater> & sweepLine, Output & output)const=0;

        virtual EventType type() const noexcept = 0;

        virtual fishnet::math::DEFAULT_NUMERIC eventPoint() const noexcept = 0;

        const Input & getObject() const noexcept {
            return *obj;
        }

        virtual ~IEvent()=default;
    };

    struct InsertEvent: public IEvent {
        InsertEvent(const Input & obj):IEvent(obj){}
        virtual EventType type() const noexcept {
            return EventType::INSERT;
        }
    };

    struct RemoveEvent: public IEvent {
        RemoveEvent(const Input & obj):IEvent(obj){}
        virtual EventType type() const noexcept {
            return EventType::REMOVE;
        }
    };

    struct DefaultInsertEvent: public InsertEvent {
        DefaultInsertEvent(const Input & obj):InsertEvent(obj){}
        virtual void process(SweepLine<Input,Output,SLSLess,InsertEventsFirst,EventQueueGreater> & sweepLine, Output & output) const {
            sweepLine.addSLS(this->obj);
        }
    };

    struct DefaultRemoveEvent: public RemoveEvent{
        DefaultRemoveEvent(const Input & obj):RemoveEvent(obj){}
        virtual void process(SweepLine<Input,Output,SLSLess,InsertEventsFirst,EventQueueGreater> & sweepLine, Output & output) const {
            sweepLine.removeSLS(this->obj);
        }
    };

    using eventPointer = std::unique_ptr<IEvent>;

private:
    const static inline EventQueueGreater eventQueueGreater = EventQueueGreater {};
    const static inline SLSLess slsLess = SLSLess {};

    /**
     * @brief Comparator for eventPointer
     * Dereferences events to compare the event points
     */
    struct EventQueueGreaterPtr {
        bool operator()(const eventPointer & lhs, const eventPointer & rhs) const noexcept {
            if(fishnet::math::areEqual(lhs->eventPoint(),rhs->eventPoint())){
                if constexpr(InsertEventsFirst)
                    return lhs->type() == EventType::INSERT; // insert events are processed after remove events
                else {
                    return lhs->type() != EventType::INSERT; // insert events are processed before remove events
                }
            }
            return eventQueueGreater(rhs->eventPoint(), lhs->eventPoint());
        }
    };

    /**
     * @brief Dereferencing Comparator for Input Pointers, stored in the SLS
     * 
     */
    struct SLSOrdering {
        bool operator()(const Input * lhs, const Input * rhs) const noexcept {
            return slsLess(*lhs,*rhs);
        }
    };

public:
    using SLS = std::set<const Input *,SLSOrdering>;
    using EventQ = std::priority_queue<eventPointer,std::vector<eventPointer>,EventQueueGreaterPtr>;
private:
    SLS sls;
    EventQ queue;
public:

    void addEvent(eventPointer event) noexcept {
        queue.push(std::move(event));
    }

    void removeEvent(eventPointer const & event) noexcept {
        queue.erase(event);
    }

    eventPointer nextEvent() noexcept {
        return queue.top();
    }

    void addSLS(const Input * input) noexcept {
        sls.insert(input);
    }

    void removeSLS(const Input * input) noexcept {
        sls.erase(input);
    }

    bool containsSLS(const Input * input) noexcept {
        return sls.contains(input);
    }

    const SLS & getSLS() const noexcept {
        return this->sls;
    }

    auto lowerBound(const Input & input) const noexcept {
        return this->sls.lower_bound(&input);
    }

    auto upperBound(const Input & input) const noexcept {
        return this->sls.upper_bound(&input);
    }

    const EventQ & getEventQueue() const noexcept {
        return this->queue;
    }

    void addEvents(util::forward_range_of<eventPointer> auto && events) noexcept {
        for(auto && event : events) {
            addEvent(std::move(event));
        }
    }

    void addEvents(util::forward_range_of<Input> auto const & objects, util::UnaryFunction<Input,std::vector<eventPointer>> auto eventMapper) noexcept {
        for(const auto & obj : objects) {
            for(auto && event : eventMapper(obj)){
                addEvent(std::move(event));
            }
        }
    }

    Output sweep(Output & output) noexcept {
        while(not queue.empty()){
            queue.top()->process(*this,output);
            queue.pop();
        }
        this->sls.clear();
        return output;
    }
};
}

