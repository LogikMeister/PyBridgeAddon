import { EventMap, EventHandler, EventGroupType, ArgumentType } from './type';

class EventGroup {
    private handlers: EventMap = {};

    // Add an event handler
    public on<T extends ArgumentType>(event: string, handler: EventHandler<T>) {
      this.handlers[event] = handler;
    }

    // Delete an event handler
    public off(event: string) {
      delete this.handlers[event];
    }

    // Get all event handlers
    public getHandlers(): EventMap {
      return this.handlers;
    }

    // Delete all event handlers
    public offAll() {
        this.handlers = {};
    }
}

class EventStore {
    private groups: Map<string, EventGroup> = new Map();

    private getEventGroupKey({ moduleName, methodName }: EventGroupType): string {
        return `${moduleName}.${methodName}`;
    }

    public set<A extends ArgumentType>(moduleName: string, methodName: string, event: string, eventHandler: EventHandler<A>) {
        const key = this.getEventGroupKey({ moduleName, methodName });
        if (!this.groups.has(key)) {
          this.groups.set(key, new EventGroup());
        }
        const group = this.groups.get(key);
        group?.on(event, eventHandler);
    }

    public clear(moduleName: string, methodName: string, event: string) {
        const key = this.getEventGroupKey({ moduleName, methodName });
        if (!this.groups.has(key)) {
          return;
        }
        const group = this.groups.get(key);
        group?.off(event);
    }

    public clearAll(moduleName: string, methodName: string) {
        const key = this.getEventGroupKey({ moduleName, methodName });
        if (!this.groups.has(key)) {
          return;
        }
        const group = this.groups.get(key);
        group?.offAll();
    }

    public getHandlers(moduleName: string, methodName: string): EventMap | undefined {
        const key = this.getEventGroupKey({ moduleName, methodName });
        return this.groups.get(key)?.getHandlers();
    }

    public definePyEvent(moduleName: string, methodName: string, event: string) {
        // Method Decorator
        const instance = this;
        return function <This, Args extends ArgumentType>(
            target: (this: This, args: Args) => void,
            context: ClassMethodDecoratorContext<This, (this: This, args: Args) => void>
        ) {
            context.addInitializer(function() {
                instance.set<Args>(moduleName, methodName, event, target.bind(this));
            })
            return target;
        }
    }
}

const event = new EventStore();

export default event;
