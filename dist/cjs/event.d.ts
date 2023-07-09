import { EventMap, EventHandler, ArgumentType } from './type';
declare class EventStore {
    private groups;
    private getEventGroupKey;
    set<A extends ArgumentType>(moduleName: string, methodName: string, event: string, eventHandler: EventHandler<A>): void;
    clear(moduleName: string, methodName: string, event: string): void;
    clearAll(moduleName: string, methodName: string): void;
    getHandlers(moduleName: string, methodName: string): EventMap | undefined;
    definePyEvent(moduleName: string, methodName: string, event: string): <This, Args extends ArgumentType>(target: (this: This, args: Args) => void, context: ClassMethodDecoratorContext<This, (this: This, args: Args) => void>) => (this: This, args: Args) => void;
}
declare const event: EventStore;
export default event;
