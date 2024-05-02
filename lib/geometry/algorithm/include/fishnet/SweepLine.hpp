#pragma once

#include <set>
#include <queue>

#include <fishnet/CollectionConcepts.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/GeometryObject.hpp>
#include <fishnet/Polygon.hpp>
namespace fishnet::geometry {

enum class EventType{
   INSERT,REMOVE
};




template<typename Input, typename Output,util::BiPredicate<Input> SLSLess, util::BiPredicate<fishnet::math::DEFAULT_NUMERIC> EventQueueGreater = std::greater<fishnet::math::DEFAULT_NUMERIC>>
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

        virtual void process(SweepLine<Input,Output,SLSLess,EventQueueGreater> & sweepLine, std::vector<Output> & output)const=0;

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
        virtual void process(SweepLine<Input,Output,SLSLess,EventQueueGreater> & sweepLine, std::vector<Output> & output) const {
            sweepLine.addSLS(this->obj);
        }
    };

    struct DefaultRemoveEvent: public RemoveEvent{
        DefaultRemoveEvent(const Input & obj):RemoveEvent(obj){}
        virtual void process(SweepLine<Input,Output,SLSLess,EventQueueGreater> & sweepLine, std::vector<Output> & output) const {
            sweepLine.removeSLS(this->obj);
        }
    };


    using eventPointer = std::unique_ptr<IEvent>;

private:
    const static inline EventQueueGreater eventQueueGreater = EventQueueGreater {};
    const static inline SLSLess slsLess = SLSLess {};

    struct EventQueueGreaterPtr {
        bool operator()(const eventPointer & lhs, const eventPointer & rhs) const noexcept {
            if(lhs->eventPoint() == rhs->eventPoint())
                return lhs->type() == EventType::INSERT ? false:true;
            return eventQueueGreater(rhs->eventPoint(), lhs->eventPoint());
        }
    };

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

    std::vector<Output> sweep(std::vector<Output> & output) noexcept {
        while(not queue.empty()){
            queue.top()->process(*this,output);
            queue.pop();
        }
        this->sls.clear();
        return output;
}

};






}

